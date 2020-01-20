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

           -- STF module
           stf_bus   : out STD_LOGIC_VECTOR (27 downto 0);
           stf_play  : out STD_LOGIC;
           stf_write : in STD_LOGIC;                        -- The stf module asserts this when data is ready
           done      : in STD_LOGIC
         );
end timing_core;

architecture timing_core_arch of timing_core is
    
    -- Load state machine
    signal addr : unsigned(31 downto 0);

    type load_state_labels is ( Idle, Load, Hold, Finish, Abort );
    signal load_state : load_state_labels;
    
    signal kill      : STD_LOGIC;

    signal next_waiting   : STD_LOGIC;
    signal waiting   : STD_LOGIC;
    signal load_next : STD_LOGIC;
    signal last_data_valid : STD_LOGIC;  --refers to whether the last data loading is valid

    signal data_valid : STD_LOGIC;
    
--    signal latch_evt : STD_LOGIC;
--    signal new_evt   : STD_LOGIC;

    -- Alias for evt_data bits (just loaded)
--    signal load_evt_time  : STD_LOGIC_VECTOR (31 downto 0);
--    signal load_evt_value : STD_LOGIC_VECTOR (27 downto 0);
--    signal load_opcode    : unsigned (3 downto 0);

    -- Play state machine
    type play_state_labels is ( Idle, Play, Count, Finish, Abort );
    signal play_state : play_state_labels;
    
--    type play_state_labels2 is ( Idle, Running, Finish, Abort );
 --   type play_state_labels3 is ( Idle, Play, Count, WaitTrig, Jump );

    -- Next data
    signal next_evt_time   : STD_LOGIC_VECTOR (31 downto 0);
    signal next_evt_value  : STD_LOGIC_VECTOR (27 downto 0);
    signal next_opcode : unsigned (3 downto 0);
    
    -- Latched data
--    signal evt_time   : STD_LOGIC_VECTOR (31 downto 0);
    signal evt_value  : STD_LOGIC_VECTOR (27 downto 0);
    signal evt_opcode : unsigned (3 downto 0);
        
    signal play_evt_value  : STD_LOGIC_VECTOR (27 downto 0);

    

    signal delay : unsigned(31 downto 0);

    signal stf_bus_reg  : STD_LOGIC_VECTOR (27 downto 0);


begin

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
            if (stop = '1') then            load_state <= Abort;
            elsif (addr = X"00000024") then load_state <= Finish;   --temp
            elsif (load_next = '0') then    load_state <= Hold;
            end if;
          when Hold =>
            if (stop = '1') then            load_state <= Abort;
            elsif (load_next = '1') then    load_state <= Load;
            end if;
          when Finish =>                    load_state <= Idle;
          when Abort =>
            if (stop /= '1') then           load_state <= Idle;
            end if;
          when others =>                    load_state <= Idle;
        end case;
      
      end if;
    end process;

    Load_Reg_Proc: process (clk, reset)
    begin
      if (reset = '1') then
          addr <= X"00000000";
        
 --       latch_evt <= '0';
 --       new_evt <= '0';
         
          last_data_valid <= '0';
          data_valid <= '0';

 ---------- play
          stf_play <= '0';
          stf_bus_reg <= x"0000000";
          
          delay <= x"00000000";    

      elsif rising_edge(clk) then
     
     -------- Load regs
     
             if (last_data_valid = '1') then
         if (delay > 0) then
             delay <= delay - 1;
         end if;
     end if;
     
     
        if ( load_state = Finish
            OR load_state = Abort
            ) then
            addr <= X"00000000";
        elsif (load_state = Load OR load_state = Hold) then
            -- Latch data
--            next_evt_time   <= load_evt_time;
--            next_evt_value  <= load_evt_value;
--            next_evt_opcode <= load_opcode;
            
            if(load_state = Load) then
                evt_value <= next_evt_value;
                delay      <= unsigned(next_evt_time);  --need both?
                evt_opcode <= next_opcode;
                
                play_evt_value <= evt_value;
            end if;
            
            last_data_valid <= '1';
            data_valid <= last_data_valid;
            
            -- Advance address
            if (load_next = '1') then
                addr <= addr + 1;
            --    latch_evt <= '1';
            end if;

            
        elsif (load_state = Idle) then
            addr <= unsigned(ini_addr);
            last_data_valid <= '0';
        end if;
        
        
        -------- Play regs
        
        if ( play_state = Finish
            OR play_state = Abort
            ) then
        elsif (play_state = Play) then
          stf_play <= '1';
          stf_bus_reg <= play_evt_value;

        elsif (play_state = Count) then

        end if;
        



        
      end if;
    end process;

--kill <= stop OR error;

evt_addr <= STD_LOGIC_VECTOR(addr);
write <= '0';

--load_evt_time  <= evt_data(63 downto 32);
--load_evt_value <= evt_data(27 downto 0);
--load_opcode    <= unsigned(evt_data(31 downto 28));

next_evt_time  <= evt_data(63 downto 32);
next_evt_value <= evt_data(27 downto 0);
next_opcode    <= unsigned(evt_data(31 downto 28));

next_waiting <= '1' when (unsigned(next_evt_time) > 0) else
                '0';

waiting  <= '1' when (delay > 1) else
            '0';

--load_next <= (NOT next_waiting) or (NOT last_data_valid);

--load_next <= NOT (last_data_valid AND next_waiting);

load_next <= (NOT (last_data_valid AND next_waiting)) when (load_state = Load) else
             (NOT waiting) when (load_state = Hold) else
             '0';


------- Play state machine

    Play_State_Proc: process (clk, reset)
    begin
      if (reset = '1') then
         play_state <= Idle;
      elsif rising_edge(clk) then
      
        case (play_state) is
          when Idle =>
            if (data_valid = '1') then
                if (delay > 0) then     play_state <= Count;
                else                    play_state <= Play;
                end if;
--                case (evt_opcode) is
--                  when x"0" =>
--                end case;

            end if;
          when Play =>
            if (stop = '1') then                play_state <= Abort;
            elsif (data_valid = '0') then       play_state <= Idle;
            elsif (delay > 0) then              play_state <= Count;
            end if;
          when Count =>
            if (stop = '1') then            play_state <= Abort;
            elsif (delay = 0) then          play_state <= Play;
            end if;
          when Finish =>                    play_state <= Idle;
          when Abort =>
            if (stop /= '1') then           play_state <= Idle;
            end if;
          when others =>                    play_state <= Idle;
        end case;
      
      end if;
    end process;

--    Play_Reg_Proc: process (clk, reset)
--    begin
--      if (reset = '1') then
--          stf_play <= '0';
--          stf_bus_reg <= x"0000000";
--      elsif rising_edge(clk) then
--          if ( play_state = Finish
--              OR play_state = Abort
--              ) then
--          elsif (play_state = Play) then
--            stf_play <= '1';
--            stf_bus_reg <= evt_value;

--          elsif (play_state = Count) then
--              delay <= delay - 1;
--          end if;

--      end if;
--    end process;


stf_bus <= stf_bus_reg;

end timing_core_arch;
