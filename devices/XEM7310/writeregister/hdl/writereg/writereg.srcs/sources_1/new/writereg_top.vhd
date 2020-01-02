----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 01/01/2020 07:53:33 PM
-- Design Name: 
-- Module Name: writereg_top - Behavioral
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

entity writereg_top is
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
end writereg_top;

architecture arch of writereg_top is
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

	signal div1       : STD_LOGIC_VECTOR(23 downto 0);
	signal div2       : STD_LOGIC_VECTOR(23 downto 0);
	signal count1     : STD_LOGIC_VECTOR(7 downto 0);
	signal count2     : STD_LOGIC_VECTOR(7 downto 0);
	signal clk1div    : STD_LOGIC;
	signal clk2div    : STD_LOGIC;
	signal reset1     : STD_LOGIC;
	signal reset2     : STD_LOGIC;
	signal disable1   : STD_LOGIC;
	signal count1eq00 : STD_LOGIC;
	signal count1eq80 : STD_LOGIC;
	signal up2        : STD_LOGIC;
	signal down2      : STD_LOGIC;
	signal autocount2 : STD_LOGIC;
	signal count2eqFF : STD_LOGIC;
	signal temp : STD_LOGIC;
    signal temp2 : STD_LOGIC;
    
    signal  xemJ2 : STD_LOGIC_VECTOR(40 downto 1);   -- Connector J2 on XEM breakout (40 pin)
    signal  stfJP0 : STD_LOGIC_VECTOR(26 downto 1);   -- Connector JP0 on any STF daughter board (34 pin, first 26 are logic)
    
    signal regAddress : STD_LOGIC_VECTOR(31 downto 0) := x"00000000";
    signal regDataOut : STD_LOGIC_VECTOR(31 downto 0);
    signal regDataIn : STD_LOGIC_VECTOR(31 downto 0) := x"00000000";
    signal regRead : STD_LOGIC;
    signal regWrite : STD_LOGIC;
    signal writeEn : STD_LOGIC_VECTOR(3 downto 0);
    
begin

reset1     <= ep00wire(0);
disable1   <= ep00wire(1);
autocount2 <= ep00wire(2);
ep20wire   <= (x"000000" & count1);
ep21wire   <= (x"000000" & count2);
ep22wire   <= x"00000000";
reset2     <= ep40wire(0);
up2        <= ep40wire(1);
down2      <= ep40wire(2);
ep60trig   <= (x"0000000" & "00" & count1eq80 & count1eq00);
ep61trig   <= (x"0000000" & "000" & count2eqFF);

--led(7) <= '0' when (count1(7) = '1') else 'Z';
--led(6) <= '0' when (count1(6) = '1') else 'Z';
--led(5) <= '0' when (count1(5) = '1') else 'Z';
--led(4) <= '0' when (count1(4) = '1') else 'Z';
--led(3) <= '0' when (count1(3) = '1') else 'Z';
--led(2) <= '0' when (count1(2) = '1') else 'Z';
--led(1) <= '0' when (count1(1) = '1') else 'Z';
--led(0) <= '0' when (count1(0) = '1') else 'Z';

temp <= '1' when (count1(3) = '1') else '0';        --works!!!  (with xbusp as out, not inout)
temp2 <= '0' when (count1(2) = '1') else '1';
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

--piping out the count signals to a few outputs on the STF digital out board
--stfJP0 <= (1 => temp, 2 => temp2, 4 => temp2, 25 => '1', others => '0'); --pin 25 is enable (X24)

stfJP0 <= ep01wire(25 downto 0);

--xbusp(12) <= '0' when (count2(3) = '1') else '1';   --J2 pin 1

-- Counter 1
-- + Counting using a divided sys_clk
-- + Reset sets the counter to 0.
-- + Disable turns off the counter.
process (sys_clk) begin
	if rising_edge(sys_clk) then
		div1 <= div1 - "1";
		if (div1 = x"000000") then
			div1 <= x"400000";
			clk1div <= '1';
		else
			clk1div <= '0';
		end if;
   
		if (clk1div = '1') then
			if (reset1 = '1') then
				count1 <= x"00";
			elsif (disable1 = '0') then
				count1 <= count1 + "1";
			end if;
		end if;
   
		if (count1 = x"00") then
			count1eq00 <= '1';
		else
			count1eq00 <= '0';
		end if;

		if (count1 = x"80") then
			count1eq80 <= '1';
		else
			count1eq80 <= '0';
		end if;
	end if;
end process;


-- Counter #2
-- + Reset, up, and down control counter.
-- + If autocount is enabled, a divided sys_clk can also
--   upcount.
process (sys_clk) begin
	if rising_edge(sys_clk) then
		div2 <= div2 - "1";
		if (div2 = x"000000") then
			div2 <= x"100000";
			clk2div <= '1';
		else
			clk2div <= '0';
		end if;

		if (reset2 = '1') then
			count2 <= x"00";
		elsif (up2 = '1') then
			count2 <= count2 + "1";
		elsif (down2 = '1') then
			count2 <= count2 - "1";
		elsif ((autocount2 = '1') and (clk2div = '1')) then
			count2 <= count2 + "1";
		end if;

		if (count2 = x"FF") then
			count2eqFF <= '1';
		else
			count2eqFF <= '0';
		end if;
	end if;
end process;

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

ep00 : okWireIn     port map (okHE=>okHE,                                    ep_addr=>x"00", ep_dataout=>ep00wire);
--ep20 : okWireOut    port map (okHE=>okHE, okEH=>okEHx( 1*65-1 downto 0*65 ), ep_addr=>x"20", ep_datain=>ep20wire);
ep21 : okWireOut    port map (okHE=>okHE, okEH=>okEHx( 2*65-1 downto 1*65 ), ep_addr=>x"21", ep_datain=>ep21wire);
ep22 : okWireOut    port map (okHE=>okHE, okEH=>okEHx( 3*65-1 downto 2*65 ), ep_addr=>x"22", ep_datain=>ep22wire);
ep40 : okTriggerIn  port map (okHE=>okHE,                                    ep_addr=>x"40", ep_clk=>sys_clk, ep_trigger=>ep40wire);
ep60 : okTriggerOut port map (okHE=>okHE, okEH=>okEHx( 4*65-1 downto 3*65 ), ep_addr=>x"60", ep_clk=>sys_clk, ep_trigger=>ep60trig);
ep61 : okTriggerOut port map (okHE=>okHE, okEH=>okEHx( 5*65-1 downto 4*65 ), ep_addr=>x"61", ep_clk=>sys_clk, ep_trigger=>ep61trig);


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
    write => regWrite,
    read => regRead,
    addr_in => regAddress,
    data_in => regDataOut,
    data_out => regDataIn
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


