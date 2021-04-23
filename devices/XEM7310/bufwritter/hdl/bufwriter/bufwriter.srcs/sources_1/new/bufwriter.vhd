----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 01/01/2020 07:53:33 PM
-- Design Name: 
-- Module Name: bufwriter - Behavioral
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
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_misc.all;
use IEEE.std_logic_unsigned.all;
use work.FRONTPANEL.all;

Library UNISIM;
use UNISIM.vcomponents.all;

entity bufwriter is
	port (
	
		okUH      : in     STD_LOGIC_VECTOR(4 downto 0);
		okHU      : out    STD_LOGIC_VECTOR(2 downto 0);
		okUHU     : inout  STD_LOGIC_VECTOR(31 downto 0);
		okAA      : inout  STD_LOGIC;

		sys_clkp  : in     STD_LOGIC;
		sys_clkn  : in     STD_LOGIC;
		
		led       : out    STD_LOGIC_VECTOR(7 downto 0);

		xbusp     :	out  STD_LOGIC_VECTOR(29 downto 0);
		xbusn     :	out  STD_LOGIC_VECTOR(29 downto 0);
		ybusp     :	out  STD_LOGIC_VECTOR(29 downto 0);
		ybusn     :	out  STD_LOGIC_VECTOR(29 downto 0)
	);
end bufwriter;

architecture arch of bufwriter is
	signal sys_clk    : STD_LOGIC;
  
	signal okClk      : STD_LOGIC;
	signal okHE       : STD_LOGIC_VECTOR(112 downto 0);
	signal okEH       : STD_LOGIC_VECTOR(64 downto 0);
	signal okEHx      : STD_LOGIC_VECTOR(65*5-1 downto 0);
	
	signal ep00wire   : STD_LOGIC_VECTOR(31 downto 0);
	signal ep01wire   : STD_LOGIC_VECTOR(31 downto 0);     -- For sending to pins directly
	
	signal ep20wire   : STD_LOGIC_VECTOR(31 downto 0);
	signal ep21wire   : STD_LOGIC_VECTOR(31 downto 0);
	signal ep22wire   : STD_LOGIC_VECTOR(31 downto 0);
	signal ep40wire   : STD_LOGIC_VECTOR(31 downto 0);
	signal ep60trig   : STD_LOGIC_VECTOR(31 downto 0);
	signal ep61trig   : STD_LOGIC_VECTOR(31 downto 0);

    signal timing_start    : STD_LOGIC;
    signal timing_reset  : STD_LOGIC := '0';
    
    signal xemJ2 : STD_LOGIC_VECTOR(40 downto 1);   -- Connector J2 on XEM breakout (40 pin)
    signal stfJP0 : STD_LOGIC_VECTOR(26 downto 1);   -- Connector JP0 on any STF daughter board (34 pin, first 26 are logic)
    signal output_stf : STD_LOGIC_VECTOR(25 downto 0);
    
    signal regAddress : STD_LOGIC_VECTOR(31 downto 0) := x"00000000";
    signal regDataOut : STD_LOGIC_VECTOR(31 downto 0);
    signal regDataIn : STD_LOGIC_VECTOR(31 downto 0) := x"00000000";
    signal regRead : STD_LOGIC;
    signal regWrite : STD_LOGIC;
    signal writeEn : STD_LOGIC_VECTOR(3 downto 0);
    
    signal debug       : STD_LOGIC_VECTOR (31 downto 0);
    
begin

--reset1     <= ep00wire(0);
--disable1   <= ep00wire(1);
--autocount2 <= ep00wire(2);
--ep20wire   <= (x"000000" & count1);
--ep21wire   <= (x"000000" & count2);
--ep22wire   <= x"00000000";
--reset2     <= ep40wire(0);
--up2        <= ep40wire(1);
--down2      <= ep40wire(2);
--ep60trig   <= (x"0000000" & "00" & count1eq80 & count1eq00);
--ep61trig   <= (x"0000000" & "000" & count2eqFF);

ybusp <= (others => '0');
ybusn <= (others => '0');

-- New pin names
xbusp <= (  12 => xemJ2(1),  --R3
            13 => xemJ2(2),  --Y6
            14 => xemJ2(5),  --Y3
            15 => xemJ2(6),  --AA8
            16 => xemJ2(9),  --U2
            17 => xemJ2(10), --U3
            18 => xemJ2(13), --W2
            19 => xemJ2(14), --W1
            20 => xemJ2(19), --T1
            21 => xemJ2(20), --AB3
            22 => xemJ2(23), --AA1
            23 => xemJ2(24), --Y13
            24 => xemJ2(27), --AB16
            25 => xemJ2(28), --AA13
            26 => xemJ2(31), --AA15
            27 => xemJ2(32), --W15
            28 => xemJ2(35), --Y16
            29 => xemJ2(37), --V4
            others => '1');

-- pin map from FPGA pins to XEM breakout board J2 connector 
-- note: xbus is the ouput of this component
xbusn <= (  12 => xemJ2(3),  --R2
            13 => xemJ2(4),  --AA6
            14 => xemJ2(7),  --AA3
            15 => xemJ2(8),  --AB8
            16 => xemJ2(11), --V2
            17 => xemJ2(12), --V3
            18 => xemJ2(17), --Y2
            19 => xemJ2(18), --Y1
            20 => xemJ2(21), --U1
            21 => xemJ2(22), --AB2
            22 => xemJ2(25), --AB1
            23 => xemJ2(26), --AA14
            24 => xemJ2(29), --AB17
            25 => xemJ2(30), --AB13
            26 => xemJ2(33), --AB15
            27 => xemJ2(34), --W16
            28 => xemJ2(36), --AA16
            29 => xemJ2(39), --W4
            others => '1');

-- pin map from XEM J2 connector to STF JP0 daughter board connector
-- stfJP0 is the input to this component (generated by daughter board logic)
xemJ2 <= (  10 => stfJP0(26),
            12 => stfJP0(24),
            13 => stfJP0(25),
            14 => stfJP0(22),
            17 => stfJP0(23),
            18 => stfJP0(20),
            19 => stfJP0(15),
            20 => stfJP0(18),
            21 => stfJP0(13),
            22 => stfJP0(16),
            23 => stfJP0(11),
            24 => stfJP0(14),
            25 => stfJP0(9),
            26 => stfJP0(12),
            27 => stfJP0(7),
            28 => stfJP0(10),
            29 => stfJP0(5),
            30 => stfJP0(8),
            31 => stfJP0(3),
            32 => stfJP0(6),
            33 => stfJP0(1),
            34 => stfJP0(4),
            35 => stfJP0(17),
            36 => stfJP0(2),
            37 => stfJP0(19),
            39 => stfJP0(21),          
            others => '1');

--sending FrontPanel wire to STF port
--stfJP0 <= ep01wire(25 downto 0);

stfJP0 <= output_stf(25 downto 0);


osc_clk : IBUFGDS port map (O=>sys_clk, I=>sys_clkp, IB=>sys_clkn);

-- Instantiate the okHost and connect endpoints
okHI : okHost port map (
	okUH=>okUH, 
	okHU=>okHU, 
	okUHU=>okUHU, 
	okAA=>okAA,
	okClk=>okClk, 
	okHE=>okHE, 
	okEH=>okEH
);

okWO : okWireOR     generic map (N=>5) port map (okEH=>okEH, okEHx=>okEHx);

ep01 : okWireIn     port map (okHE=>okHE,                                    ep_addr=>x"01", ep_dataout=>ep01wire);

--ep00 : okWireIn     port map (okHE=>okHE,                                    ep_addr=>x"00", ep_dataout=>ep00wire);
--ep20 : okWireOut    port map (okHE=>okHE, okEH=>okEHx( 1*65-1 downto 0*65 ), ep_addr=>x"20", ep_datain=>ep20wire);
ep21 : okWireOut    port map (okHE=>okHE, okEH=>okEHx( 2*65-1 downto 1*65 ), ep_addr=>x"21", ep_datain=>ep21wire);
--ep22 : okWireOut    port map (okHE=>okHE, okEH=>okEHx( 3*65-1 downto 2*65 ), ep_addr=>x"22", ep_datain=>ep22wire);
ep40 : okTriggerIn  port map (okHE=>okHE,                                    ep_addr=>x"40", ep_clk=>sys_clk, ep_trigger=>ep40wire);
--ep60 : okTriggerOut port map (okHE=>okHE, okEH=>okEHx( 4*65-1 downto 3*65 ), ep_addr=>x"60", ep_clk=>sys_clk, ep_trigger=>ep60trig);
--ep61 : okTriggerOut port map (okHE=>okHE, okEH=>okEHx( 5*65-1 downto 4*65 ), ep_addr=>x"61", ep_clk=>sys_clk, ep_trigger=>ep61trig);

timing_start <= ep40wire(0);
timing_reset <= ep40wire(1);

ep21wire <= debug;

--FrontPanel endpoint instantiation
regBridge : okRegisterBridge port map(
     okHE=>okHE,
     okEH=>okEHx(1*65-1 downto 0*65),
     ep_write=>regWrite,
     ep_read=>regRead,
     ep_address=>regAddress, 
     ep_dataout=>regDataOut,
     ep_datain=>regDataIn
     );

module0 : entity work.timing_module port map(
    clk => okClk,
    reset => timing_reset,
--    clk => sys_clk,
    write => regWrite,
    read => regRead,
    addr_in => regAddress,
    data_in => regDataOut,
    data_out => regDataIn,
    trigger => timing_start,
    core_clk => sys_clk,
    output => output_stf,
    debug => debug
    );

--led(7) <= '0' when (regDataOut(7) = '1') else 'Z';
--led(6) <= '0' when (regDataOut(6) = '1') else 'Z';
--led(5) <= '0' when (regDataOut(5) = '1') else 'Z';
--led(4) <= '0' when (regDataOut(4) = '1') else 'Z';
--led(3) <= '0' when (regDataOut(3) = '1') else 'Z';
--led(2) <= '0' when (regDataOut(2) = '1') else 'Z';
--led(1) <= '0' when (regDataOut(1) = '1') else 'Z';
--led(0) <= '0' when (regDataOut(0) = '1') else 'Z';

end arch;


