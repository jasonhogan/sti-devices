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
   type state_type is (idle, load, play);
   signal state, next_state : state_type;
   --Declare internal signals for all outputs of the state-machine
   signal evt_addr_i : STD_LOGIC_VECTOR(31 downto 0);  -- example output signal
   signal evt_data : STD_LOGIC_VECTOR(31 downto 0);
   --other outputs
   
   signal evt_addr_synch : STD_LOGIC_VECTOR(31 downto 0);

begin

RST <= reset;
--debug <= evt_data_in;
debug <= evt_addr_i;
evt_addr_synch <= evt_addr_i;
--evt_addr <= evt_addr_synch;
--evt_addr <= x"00001111";

SYNC_PROC: process (clk)
begin
  if (clk'event and clk = '1') then
     if (RST = '1') then
        state <= idle;
        evt_addr <= (others => '0');
        evt_addr_i <= (others => '0');
        --evt_data <= (others => '0');
        output <= (others => '0');
     else
        state <= next_state;
        --evt_addr <= evt_addr_i;
        
        output <= evt_data(25 downto 0);
     -- assign other outputs to internal signals
     end if;
  end if;
end process;   


----MEALY State-Machine - Outputs based on state and inputs
--OUTPUT_DECODE: process (state)
--begin
--  --insert statements to decode internal output signals
--  --below is simple example
--  if (state = idle) then
--     --evt_addr_i <= evt_addr_i;
--     evt_data <= evt_data;
--     debug <= (0 => '1', others => '0');
--  elsif state = load then
--     evt_addr_i <= evt_addr_i + "1";
--     evt_data <= evt_data;
--     debug <= (1 => '1', others => '0');
--  elsif state = play then    
--     --evt_addr_i <= evt_addr_i;
--     evt_data <= evt_data_in;
--     debug <= (2 => '1', others => '0');
--  else
--     --evt_addr_i <= evt_addr_i;
--     evt_data <= evt_data;
--     debug <= (3 => '1', others => '0');
--  end if;
--end process;

process(clk) 
begin 
    if rising_edge(clk) then
        if state = load then evt_addr_i <= evt_addr_i + 1;
        else evt_addr_i <= evt_addr_i;
        end if;
        if state = play then evt_data <= evt_data_in;
        end if;
    end if; 
end process;

NEXT_STATE_DECODE: process (state, trigger)
begin
  --declare default state for next_state to avoid latches
  next_state <= state;  --default is to stay in current state
  --insert statements to decode next_state
  --below is a simple example
  case (state) is
     when idle =>
        if trigger = '1' then
           next_state <= load;
         --  debug <= (0=>'1', others =>'0');
        end if;
     when load =>
        next_state <= play;
--         if trigger = '1' then
--            next_state <= play;
--         --   debug <= (1=>'1', others =>'0');
--         end if;
     when play =>
        --next_state <= idle;
           if trigger = '1' then
                next_state <= idle;
         --       debug <= (2=>'1', others =>'0');
           end if;
     when others =>
        --next_state <= idle;
        --debug <= (3=>'1', others =>'0');
  end case;
end process;

				
end Behavioral;
