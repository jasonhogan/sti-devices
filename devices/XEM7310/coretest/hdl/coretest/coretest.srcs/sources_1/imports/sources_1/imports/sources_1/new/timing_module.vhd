----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 01/01/2020 08:08:11 PM
-- Design Name: 
-- Module Name: timing_module - Behavioral
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

entity timing_module is
    Port ( 
        clk     : in STD_LOGIC;
        reset   : in STD_LOGIC;
        write   : in STD_LOGIC;
        read   : in STD_LOGIC;
        addr_in : in STD_LOGIC_VECTOR (31 downto 0);
        data_in : in STD_LOGIC_VECTOR (31 downto 0);
        data_out : out STD_LOGIC_VECTOR (31 downto 0);
        trigger : in STD_LOGIC;
        core_clk : in STD_LOGIC;
        output      : out STD_LOGIC_VECTOR (25 downto 0);
        debug       : out STD_LOGIC_VECTOR (31 downto 0)
        );
end timing_module;

architecture Behavioral of timing_module is

	signal event_addr : STD_LOGIC_VECTOR(31 downto 0); -- := x"00000000";
	signal writeA     : STD_LOGIC_VECTOR(0 downto 0);
	signal readA      : STD_LOGIC;
	signal writeB     : STD_LOGIC_VECTOR(0 downto 0) := "0";
    signal readB      : STD_LOGIC;
	signal evt_data_in : STD_LOGIC_VECTOR(31 downto 0) := x"00000000";
	signal evt_data_out : STD_LOGIC_VECTOR(31 downto 0) := x"00000000";
	
	signal not_clk     : STD_LOGIC;
	
COMPONENT blk_mem_gen_0
  PORT (
    clka : IN STD_LOGIC;
--    ena : IN STD_LOGIC;
    wea : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    addra : IN STD_LOGIC_VECTOR(9 DOWNTO 0);
    dina : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    douta : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
    clkb : IN STD_LOGIC;
--    enb : IN STD_LOGIC;
    web : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    addrb : IN STD_LOGIC_VECTOR(9 DOWNTO 0);
    dinb : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    doutb : OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
  );
END COMPONENT;

begin

process (write) begin
	if(write = '1') then
		writeA <= "1";
	else
		writeA <= "0";
	end if;
end process;

not_clk <= not clk;

writeB <= "0";

evt_register : blk_mem_gen_0
  PORT MAP (
    clka => not_clk,
--    ena => read,
    wea => writeA,
    addra => addr_in(9 downto 0),
    dina => data_in,
    douta => data_out,
    clkb => core_clk,
--    enb => readB,
    web => writeB,
    addrb => event_addr(9 downto 0),
    dinb => evt_data_out,
    doutb => evt_data_in
  );

core : entity work.timing_core port map(
    clk => core_clk,
    reset => reset,
    trigger => trigger,
    evt_addr => event_addr,
    evt_data_in => evt_data_in,  --into the core, out of the register
    output => output,    --to the STF board
    debug => debug
    );


end Behavioral;
