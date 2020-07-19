----------------------------------------------------------------------------------
-- Company: Stanford University
-- Engineer: Jason Hogan
-- 
-- Create Date: 07/18/2020 04:44:30 PM
-- Design Name: STF trigger module
-- Module Name: timing_trigger - Behavioral
-- Project Name: STI
-- Target Devices: XEM7310-A200 (xc7a200tfbg484-1)
-- Tool Versions: 
-- Description: Trigger logic for all timing cores
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

entity timing_trigger is
    generic (N : integer);
    Port (
           clk      : in STD_LOGIC;
           reset    : in STD_LOGIC;
           
           clear    : in STD_LOGIC;     -- set all triggers to zero
           mode     : in STD_LOGIC;     -- 0=software, 1=hardware
           ext_trigger : in STD_LOGIC;  -- hardware trigger
                      
           trigger_in  : in STD_LOGIC_VECTOR (N-1 downto 0);      --all modules
           trigger_out : out STD_LOGIC_VECTOR (N-1 downto 0)      --all modules
          
          );
end timing_trigger;

architecture timing_trigger_arch of timing_trigger is

    signal trigger : STD_LOGIC;
    signal clear_buf : STD_LOGIC;
    signal ext_armed : STD_LOGIC;

    signal trig_in_reg  : STD_LOGIC_VECTOR(N-1 downto 0);
    signal trig_out_reg : STD_LOGIC_VECTOR(N-1 downto 0);

begin

    -- In Hardware trigger mode, the ext_trigger line will trigger all modules
    -- that have received a software trigger on the trigger_in lines. In this mode,
    -- trigger_in acts like enable.
    -- In Software trigger mode, trigger_in will directly trigger the module.

    Triger_State_Proc: process (clk, reset)
    begin
      if (reset = '1') then
         trig_in_reg  <= (others=>'0');
         trig_out_reg <= (others=>'0');
         ext_armed <= '0';
         
      elsif rising_edge(clk) then

         
         -- Buffer for module triggers
         if (clear_buf = '1') then
            trig_in_reg <= (others=>'0');
         else
            trig_in_reg <= trig_in_reg OR trigger_in;   -- stores all triggered modules
         end if;
         
         -- Perform trigger
         if (trigger = '1') then
            trig_out_reg <= trig_in_reg;
         else
            trig_out_reg <= (others=>'0');
         end if;

         -- make sure to trigger on rising edge of ext_trigger
         if (ext_trigger = '0') then
            ext_armed <= '1';
         else
            ext_armed <= '0';
         end if;
         
      end if;
    end process;

trigger <= '1' when ( mode = '0' ) else (ext_armed AND ext_trigger);

clear_buf <= (clear OR trigger) when ( mode = '1' ) else clear;     -- For Hardware mode, clear on trigger

trigger_out <= trig_out_reg;


end timing_trigger_arch;
