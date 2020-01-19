----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 01/03/2020 06:38:26 PM
-- Design Name: 
-- Module Name: timing_core_tb - Behavioral
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

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity timing_core_tb is
end timing_core_tb;

architecture Behavioral of timing_core_tb is
    signal CLK : std_ulogic := '1';
    signal RST    : STD_LOGIC := '1';
    signal trigger     : STD_LOGIC := '0';
    signal evt_addr    : STD_LOGIC_VECTOR (31 downto 0);
    signal evt_data_in : STD_LOGIC_VECTOR (31 downto 0) := x"0000F001";
    signal output : STD_LOGIC_VECTOR (25 downto 0);
    signal debug  : STD_LOGIC_VECTOR (31 downto 0);
begin
    
    CLK <= not CLK after 5 ns;
    RST <= '0' after 20 ns;
    

      
    UUT : entity work.timing_core port map (clk => CLK, reset => RST, trigger => trigger, evt_addr => evt_addr, evt_data_in=>evt_data_in, output => output, debug => debug);

    stim_proc: process
    begin
        wait for 60 ns;
        trigger <= '1';
        wait for 10 ns;
        trigger <= '0';
        wait for 200 ns;
    end process;    


end Behavioral;
