----------------------------------------------------------------------------------
-- Company: Stanford University
-- Engineer: Jason Hogan
-- 
-- Create Date: 01/18/2020 06:15:13 PM
-- Design Name: STF bus timing core
-- Module Name: timing_core - Behavioral
-- Project Name: STI
-- Target Devices: XEM7310-A200 (xc7a200tfbg484-1)
-- Tool Versions: 
-- Description: Timing core state machine
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity timing_core is
    Port ( 
           clk      : in STD_LOGIC;
           reset    : in STD_LOGIC;
           
           -- Core control
           start    : in STD_LOGIC;
           stop     : in STD_LOGIC;
           ini_addr : in STD_LOGIC_VECTOR (31 downto 0);    -- initial address

           -- Event register
           evt_addr : out STD_LOGIC_VECTOR (31 downto 0);
           evt_data : in STD_LOGIC_VECTOR (63 downto 0);    -- 32 bit for delay, 32 bits for data
           write    : out STD_LOGIC;                        -- High when the core is writing
           evt_data_out : out STD_LOGIC_VECTOR (31 downto 0);

           -- STF module
           stf_bus   : out STD_LOGIC_VECTOR (27 downto 0);
           stf_data  : in STD_LOGIC_VECTOR (31 downto 0);
           stf_play  : out STD_LOGIC;
           stf_write : in STD_LOGIC;                        -- The stf module asserts this when data is ready
           stf_error : in STD_LOGIC
         );
end timing_core;

architecture timing_core_arch of timing_core is
   
    -- Load state machine
    type load_state_labels is ( Idle, Load, Hold, Finish, Abort, Jump, Save );
    signal load_state : load_state_labels;
    
    -- Play state machine
    type play_state_labels is ( Idle, Play, Count, Finish, Abort, Jump );
    signal play_state : play_state_labels;
    
    -- BRAM addresses
    signal addr             : unsigned(31 downto 0);
    signal jump_target_addr : unsigned(31 downto 0);

    signal addr_p1          : unsigned(31 downto 0);
    signal addr_p2          : unsigned(31 downto 0);
    signal play_addr        : unsigned(31 downto 0);
    
     
    -- Next data; alias for evt_data bits (just loaded)
    signal next_evt_time   : STD_LOGIC_VECTOR (31 downto 0);
    signal next_evt_value  : STD_LOGIC_VECTOR (27 downto 0);
    signal next_opcode     : unsigned (3 downto 0);
    
    -- Latched data
--    signal evt_time   : STD_LOGIC_VECTOR (31 downto 0);
    signal delay      : unsigned(31 downto 0);
    signal evt_value  : STD_LOGIC_VECTOR (27 downto 0);
    signal evt_opcode : unsigned (3 downto 0);

    signal play_evt_value  : STD_LOGIC_VECTOR (27 downto 0);    --pipeline delay of play

    -- output register
    signal stf_bus_reg  : STD_LOGIC_VECTOR (27 downto 0);

    signal write_reg       : STD_LOGIC;

    -- combo logic intermediates
    signal load_next       : STD_LOGIC;
    signal next_waiting    : STD_LOGIC;
    signal waiting         : STD_LOGIC;
    signal next_data_valid : STD_LOGIC;  --is data just loaded from BRAM valid (i.e., associated with addr)
    signal value_valid     : STD_LOGIC;
    signal jump_evt        : STD_LOGIC;
    signal jump_evt_p2     : STD_LOGIC;
    signal perform_jump    : STD_LOGIC;
        
    signal kill      : STD_LOGIC;
    
    type opcode_type is ( Action, WaitOp, EndOp, Jump, JumpSetReturn, SetAffine, Option, Noop);
    
    function opcode_lookup ( opcode_in : unsigned ) return opcode_type is
        variable op_out : opcode_type;
    begin
        case (to_integer(opcode_in)) is
          when 0 => op_out := Action;
          when 1 => op_out := WaitOp;
          when 2 => op_out := EndOp;
          when 3 => op_out := Jump;
          when 4 => op_out := JumpSetReturn;
          when 5 => op_out := SetAffine;
          when 6 => op_out := Option;
          when 7 => op_out := Noop;
          when others => op_out := Noop;
        end case;
        return op_out;
    end opcode_lookup;
    
    signal test_next_op : opcode_type;
    signal test_op : opcode_type;
    
    
    function opcode_is ( opcode_in : unsigned; opcode_name : opcode_type ) return boolean is
        variable match : boolean;
    begin
        if (opcode_lookup(opcode_in) = opcode_name) then
            match := True;
        else
            match := False;
        end if;
        return match;
    end opcode_is;
    
begin

    test_next_op <= opcode_lookup(next_opcode);
    test_op <= opcode_lookup(evt_opcode);

    ------- Load state machine
    Load_State_Proc: process (clk, reset)
    begin
      if (reset = '1') then
         load_state <= Idle;
      elsif rising_edge(clk) then
      
        case (load_state) is
          when Idle =>
            if (start = '1') then           load_state <= Load;
            end if;
          when Load =>
            if (stop = '1') then                                load_state <= Abort;
            elsif (addr = X"00000024") then                     load_state <= Finish;   --temp
            elsif (load_next = '0') then                        load_state <= Hold;
            elsif (opcode_lookup(next_opcode) = Jump ) then     load_state <= Jump;
            end if;
          when Hold =>
            if (stop = '1') then                                load_state <= Abort;
            elsif (stf_write = '1') then                        load_state <= Save;
            elsif (load_next = '1') then
                if ( opcode_lookup(evt_opcode) = Jump ) then    load_state <= Jump;
                else                                            load_state <= Load;
                end if;
            end if;
          when Save =>                      load_state <= Hold;     -- if write = '1' then
          when Jump =>                      load_state <= Load;
          when Finish =>                    load_state <= Idle;
          when Abort =>
            if (stop /= '1') then           load_state <= Idle;
            end if;
          when others =>                    load_state <= Idle;
        end case;
      
      end if;
    end process;

    ------- Play state machine
    Play_State_Proc: process (clk, reset)
    begin
      if (reset = '1') then
         play_state <= Idle;
      elsif rising_edge(clk) then
      
        case (play_state) is
          when Idle =>
            if (value_valid = '1') then
                if (delay > 0) then         play_state <= Count;
                else                        play_state <= Play;
                end if;
            end if;
          when Play =>
            if (stop = '1') then                                play_state <= Abort;
            elsif (delay > 0) then                              play_state <= Count;
            elsif ( opcode_lookup(evt_opcode) = Action ) then   play_state <= Play;
            elsif ( opcode_lookup(evt_opcode) = Jump ) then     play_state <= Jump;
            else                                                play_state <= Idle;
            end if;
          when Count =>
            if (stop = '1') then                                    play_state <= Abort;
            elsif (delay = 0) then
                if ( opcode_lookup(evt_opcode) = Jump ) then        play_state <= Jump;
                elsif ( opcode_lookup(evt_opcode) = Action ) then   play_state <= Play;
                end if;
            end if;
          when Jump =>                      play_state <= Idle;
          when Finish =>                    play_state <= Idle;
          when Abort =>
            if (stop /= '1') then           play_state <= Idle;
            end if;
          when others =>                    play_state <= Idle;
        end case;
        
      end if;
    end process;


    Core_Reg_Proc: process (clk, reset)
    begin
      if (reset = '1') then
          addr <= X"00000000";
          delay <= x"00000000";

          evt_value <= x"0000000";
          evt_opcode <= x"0";
          
          stf_bus_reg <= x"0000000";
          
          next_data_valid <= '0';
          value_valid <= '0';
          stf_play <= '0';
          
          jump_evt_p2 <= '0';
          
          write_reg <= '0';

      elsif rising_edge(clk) then
     
        -------- Delay counter
        if (next_data_valid = '1') then
            if (delay > 0) then
                delay <= delay - 1;
            end if;
        end if;
        
        -- addr
        if (load_state = Idle) then
            addr <= unsigned(ini_addr);
        else
            -- Advance address
            if (load_next = '1') then
                if (perform_jump = '1') then
                    addr <= jump_target_addr;
                else
                    addr <= addr + 1;
                end if;
                
                addr_p1 <= addr;
--                addr_p2 <= addr_p1;
                play_addr <= addr_p1;
            end if;


        end if;

        -- event data pipeline
        if(next_data_valid = '1') then
            if(load_state = Load OR load_state = Jump) then
                if(load_state /= Jump) then      --avoid overwritting jump_target_addr during a Jump
                    evt_value  <= next_evt_value;
                end if;
                
                evt_opcode <= next_opcode;
                delay      <= unsigned(next_evt_time);

                play_evt_value <= evt_value;    -- pipeline delay
            end if;
        end if;

        -- jumps
        if (jump_evt_p2 = '1' AND waiting = '1') then
            jump_evt_p2 <= '1';
        else
            if (next_waiting = '1') then
                jump_evt_p2 <= jump_evt;
            end if;
        end if;        

        -- data initialization (wait for valid data after the load state machine is Idle)
        if ( load_state = Idle OR load_state = Finish OR load_state = Abort ) then
            next_data_valid <= '0';
        else
            next_data_valid <= '1';
        end if;
        value_valid <= next_data_valid;

        -------- Play reg
        if (play_state = Play) then
            stf_play <= '1';
            stf_bus_reg <= play_evt_value;
        else
            stf_play <= '0';
        end if;

        -------- Save reg
        if (load_state = Save) then
            write_reg <= '1';
        else
            write_reg <= '0';
        end if;

      end if;
    end process;

--kill <= stop OR error;

write <= write_reg;
evt_addr <= STD_LOGIC_VECTOR(addr) when write_reg = '0' else
            STD_LOGIC_VECTOR(play_addr);


-- Event register breakout
next_evt_time  <= evt_data(63 downto 32);
next_evt_value <= evt_data(27 downto 0);
next_opcode    <= unsigned(evt_data(31 downto 28));

next_waiting <= '1' when (unsigned(next_evt_time) > 0) else '0';
waiting      <= '1' when (delay > 1) else '0';      -- Stop waiting at delay=1 so play happens at 0

load_next <= (NOT (next_data_valid AND next_waiting)) when (load_state = Load) else
             (NOT waiting) when (load_state = Hold) else
             '1' when (load_state = Jump) else
             '0';

jump_target_addr <= (addr(31 downto 28) & unsigned(next_evt_value)) when (NOT next_waiting = '1') else
                    (addr(31 downto 28) & unsigned(evt_value));

jump_evt <= '1' when ((delay = 0) AND ( opcode_lookup(next_opcode) = Jump )) else
            '0';

-- cannot jump if already jumping
perform_jump <= '1' when (load_state /= Jump) AND (jump_evt_p2 = '1' OR (jump_evt = '1' AND (NOT next_waiting = '1'))) else
                '0';

stf_bus <= stf_bus_reg;     -- output to the module, from the register

evt_data_out <= stf_data;   -- input data from the module, to the register


end timing_core_arch;
