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
           stf_bus   : out STD_LOGIC_VECTOR (25 downto 0);
           play      : out STD_LOGIC;
           stf_write : in STD_LOGIC;                        -- The stf module asserts this when data is ready
           done      : in STD_LOGIC
         );
end timing_core;

architecture timing_core_arch of timing_core is

begin


end timing_core_arch;
