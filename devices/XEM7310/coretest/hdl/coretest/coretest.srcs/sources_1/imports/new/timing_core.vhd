----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 01/02/2020 02:43:35 PM
-- Design Name: 
-- Module Name: timing_core - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
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

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

--use IEEE.std_logic_1164.all;
--use IEEE.std_logic_arith.all;
--use IEEE.std_logic_misc.all;
use IEEE.std_logic_unsigned.all;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity timing_core is
    Port ( 
        clk         : in STD_LOGIC;
        reset       : in STD_LOGIC;
        trigger     : in STD_LOGIC;
        evt_addr    : out STD_LOGIC_VECTOR (31 downto 0);
        evt_data_in : in STD_LOGIC_VECTOR (31 downto 0);
        output      : out STD_LOGIC_VECTOR (25 downto 0);
        debug       : out STD_LOGIC_VECTOR (31 downto 0)
        );
        
end timing_core;

architecture Behavioral of timing_core is
    
   signal next_evt_addr : STD_LOGIC_VECTOR(31 downto 0);

   signal RST : std_logic := '0';
   --Use descriptive names for the states, like st1_reset, st2_search
   type state_type is (initial, idle, load, load2, countdown, play);
   signal state, next_state : state_type;
   --Declare internal signals for all outputs of the state-machine
   signal evt_addr_i : STD_LOGIC_VECTOR(31 downto 0);  -- example output signal
   signal next_addr : STD_LOGIC_VECTOR(31 downto 0) := x"00000001";
   
   signal evt_data : STD_LOGIC_VECTOR(31 downto 0);
   --other outputs
   
   signal evt_addr_synch : STD_LOGIC_VECTOR(31 downto 0);
   
    signal evt_delay : STD_LOGIC_VECTOR(3 downto 0);
    signal opcode : STD_LOGIC_VECTOR(3 downto 0);
    signal evt_value : STD_LOGIC_VECTOR(23 downto 0);

    signal stop : std_logic := '0';
    signal done_counting : std_logic := '1';
    signal done_playing : std_logic := '1';
    
begin

RST <= reset;
                
--evt_delay <= evt_data(3 downto 0);
--opcode <= evt_data(3 downto 0);
--evt_value <= evt_data(23 downto 0);


SYNC_PROC: process (clk)
begin
  if (clk'event and clk = '1') then
     if (RST = '1') then
        state <= initial;
        
        evt_addr <= (others => '0');
        evt_addr_i <= (others => '0');
        --next_addr <= (1=>'1',others => '0');
        --evt_data <= (others => '0');
        output <= (others => '0');
     else
        state <= next_state;
        output <= evt_data(25 downto 0);
        
        --evt_addr <= evt_addr_i;

        case (state) is
             when idle =>
                done_counting <= '0';
                evt_addr_i <= next_addr;
                evt_addr <= next_addr;
             when load =>

                next_addr <= evt_addr_i + "1";  --or other logic...
                evt_data <= evt_data_in;
                
--                --event data decode
                evt_delay <= evt_data_in(3 downto 0);
                opcode <= evt_data_in(3 downto 0);
                evt_value <= evt_data_in(23 downto 0);
                
                done_playing <= '0';
--                if evt_delay = 0 then
--                     done_counting <= '1';
--                else
--                     done_counting <= '0';
--                end if;
                
             when countdown =>
                if evt_delay = 0 then
                    done_counting <= '1';
                else
                    evt_delay <= evt_delay - "1";
                end if;
             when play =>
                done_playing <= '1';
                --output <= evt_value(25 downto 0);
             when others =>
          end case;
  
     -- assign other outputs to internal signals
     end if;
  end if;
end process;   


NEXT_STATE_DECODE: process (state, trigger, done_counting, done_playing)
begin
  --declare default state for next_state to avoid latches
  next_state <= state;  --default is to stay in current state

  case (state) is
     when initial =>
         next_state <= idle;
     when idle =>
        if trigger = '1' then
           next_state <= load;
         --  debug <= (0=>'1', others =>'0');
        end if;
     when load =>
--        next_state <= load2;
        next_state <= countdown;
--        if done_playing = '1' then
--           next_state <= play;
--        else
--            next_state <= countdown;
--        end if;
--         if trigger = '1' then
--            next_state <= play;
--         --   debug <= (1=>'1', others =>'0');
--         end if;
     when load2 => next_state <= play;
     --when countdown => next_state <= play;
     when countdown =>
        if done_counting = '1' then
            next_state <= play;
        end if;
        --next_state <= idle;
--           if trigger = '1' then
--                next_state <= idle;
--         --       debug <= (2=>'1', others =>'0');
--           end if;
     when play => 
        if done_playing = '1' then
           next_state <= idle;
        end if;
        --next_state <= idle;
     when others =>
        --next_state <= idle;
        --debug <= (3=>'1', others =>'0');
  end case;
end process;

				
end Behavioral;
