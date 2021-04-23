----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 01/18/2020 07:28:48 PM
-- Design Name: 
-- Module Name: stf_output - Behavioral
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

use work.stf_timing.all;

entity stf_output is
    Port ( 
       clk       : in STD_LOGIC;
       reset     : in STD_LOGIC;

       -- STF module
       stf_in  : in to_stf_module;
       stf_out : out from_stf_module;
       
       stf_pins : out STD_LOGIC_VECTOR (25 downto 0)
     );
end stf_output;

architecture stf_output_arch of stf_output is

begin

    Reg_Proc: process (clk, reset)
    begin
      if (reset = '1') then
        stf_pins <= X"000000" & '0' & '0';
        
      elsif rising_edge(clk) then

        if (stf_in.stf_play = '1') then    -- here is where we can check for "ready" and raise an error if needed
            stf_pins <= stf_in.stf_bus(25 downto 0);
        end if;
      end if;
    end process;

stf_out.stf_write <= '0';
stf_out.stf_error <= '0';
stf_out.stf_data <= x"00000000";

end stf_output_arch;
