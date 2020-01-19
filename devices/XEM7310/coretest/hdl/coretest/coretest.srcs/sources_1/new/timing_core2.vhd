----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 01/17/2020 07:27:40 PM
-- Design Name: 
-- Module Name: timing_core2 - Behavioral
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
use IEEE.NUMERIC_STD.ALL;
-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity timing_core2 is
    Port ( 
           clk    : in STD_LOGIC;
           reset  : in STD_LOGIC;
           
           go     : in STD_LOGIC;
           kill   : in STD_LOGIC;
           
           done : out STD_LOGIC
         );
end timing_core2;

architecture timing_core2_arch of timing_core2 is

    signal count : unsigned(7 downto 0);

    type state_labels is  (  Idle,
                             Active,
                             Finish,
                             Abort
                          );

    signal state_reg : state_labels;

begin

    State_Proc: process (clk, reset)
    begin
      if (reset = '1') then
         state_reg <= Idle;
      elsif rising_edge(clk) then
      
        case (state_reg) is
          when Idle =>
            if (go = '1') then          state_reg <= Active;
            end if;
          when Active =>
            if (kill = '1') then        state_reg <= Abort;
            elsif (count = X"24") then  state_reg <= Finish;
            end if;
          when Finish =>                state_reg <= Idle;
          when Abort =>
            if (kill /= '1') then       state_reg <= Idle;
            end if;
          when others =>                state_reg <= Idle;
        end case;
      
      end if;
    end process;

    Reg_Proc: process (clk, reset)
    begin
      if (reset = '1') then
        count <= X"00";
        done <= '0';
      elsif rising_edge(clk) then
        if ( state_reg = Finish
            OR state_reg = Abort
            ) then
            count <= X"00";
        elsif (state_reg = Active) then
            count <= count + 1;
        end if;
        
        -- output register
        if (state_reg = Finish) then
            done <= '1';
        else
            done <= '0';
        end if;
      end if;
    end process;

end timing_core2_arch;
