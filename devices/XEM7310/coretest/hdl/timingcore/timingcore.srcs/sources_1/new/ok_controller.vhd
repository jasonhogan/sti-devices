----------------------------------------------------------------------------------
-- Company: Stanford University
-- Engineer: Jason Hogan
-- 
-- Create Date: 01/28/2020 09:19:20 PM
-- Design Name: Opal Kelly controller
-- Module Name: ok_controller - Behavioral
-- Project Name: STI
-- Target Devices: XEM7310-A200 (xc7a200tfbg484-1)
-- Tool Versions: 
-- Description: The Opal Kelly (ok) computer interface. Connects the Opal Kelly 
--              bus to the timing_module instances so the computer can control them.
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


use work.FRONTPANEL.all;
use work.stf_timing.all;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity ok_controller is
    Port ( 
           sys_clk   : in STD_LOGIC;
           okClk     : out STD_LOGIC;
           
           global_rst : out STD_LOGIC;

           -- Timing module bus
           --mod_bus_out : out to_timing_mod;
           --mod_bus_in  : in from_timing_mod;
           
           -- Timing module bus
           mod_bus_outs : out to_timing_mod_array(7 downto 0);
           mod_bus_ins : in from_timing_mod_array(7 downto 0);
           
           soft_trigger  : out trigger_bus;      -- software trigger bus
           hard_triggers : in STD_LOGIC_VECTOR(7 downto 0);     --from HDL

           okUH      : in     STD_LOGIC_VECTOR(4 downto 0);
           okHU      : out    STD_LOGIC_VECTOR(2 downto 0);
           okUHU     : inout  STD_LOGIC_VECTOR(31 downto 0);
           okAA      : inout  STD_LOGIC
        );
end ok_controller;

architecture ok_controller_arch of ok_controller is

	signal okHE       : STD_LOGIC_VECTOR(112 downto 0);
	signal okEH       : STD_LOGIC_VECTOR(64 downto 0);
	signal okEHx      : STD_LOGIC_VECTOR(65*5-1 downto 0);

	signal ep00wire   : STD_LOGIC_VECTOR(31 downto 0);
	signal ep01wire   : STD_LOGIC_VECTOR(31 downto 0);     -- For sending to pins directly
	
	signal ep20wire   : STD_LOGIC_VECTOR(31 downto 0);
	signal ep21wire   : STD_LOGIC_VECTOR(31 downto 0);
	signal ep22wire   : STD_LOGIC_VECTOR(31 downto 0);
	
	signal ep40wire   : STD_LOGIC_VECTOR(31 downto 0);
	signal ep41wire   : STD_LOGIC_VECTOR(31 downto 0);
	
	signal ep60trig   : STD_LOGIC_VECTOR(31 downto 0);
	signal ep61trig   : STD_LOGIC_VECTOR(31 downto 0);


    signal regAddress : STD_LOGIC_VECTOR(31 downto 0) := x"00000000";
    signal regDataOut : STD_LOGIC_VECTOR(31 downto 0);
    signal regDataIn : STD_LOGIC_VECTOR(31 downto 0) := x"00000000";
    signal regRead : STD_LOGIC;
    signal regWrite : STD_LOGIC;

begin

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

okWO : okWireOR     generic map (N=>5) port map (okEH=>okEH, okEHx=>okEHx);

ep01 : okWireIn     port map (okHE=>okHE,                                    ep_addr=>x"01", ep_dataout=>ep01wire);

--ep00 : okWireIn     port map (okHE=>okHE,                                    ep_addr=>x"00", ep_dataout=>ep00wire);
--ep20 : okWireOut    port map (okHE=>okHE, okEH=>okEHx( 1*65-1 downto 0*65 ), ep_addr=>x"20", ep_datain=>ep20wire);
ep21 : okWireOut    port map (okHE=>okHE, okEH=>okEHx( 2*65-1 downto 1*65 ), ep_addr=>x"21", ep_datain=>ep21wire);
--ep22 : okWireOut    port map (okHE=>okHE, okEH=>okEHx( 3*65-1 downto 2*65 ), ep_addr=>x"22", ep_datain=>ep22wire);
ep40 : okTriggerIn  port map (okHE=>okHE,                                    ep_addr=>x"40", ep_clk=>sys_clk, ep_trigger=>ep40wire);
--ep60 : okTriggerOut port map (okHE=>okHE, okEH=>okEHx( 4*65-1 downto 3*65 ), ep_addr=>x"60", ep_clk=>sys_clk, ep_trigger=>ep60trig);
--ep61 : okTriggerOut port map (okHE=>okHE, okEH=>okEHx( 5*65-1 downto 4*65 ), ep_addr=>x"61", ep_clk=>sys_clk, ep_trigger=>ep61trig);

-- Software triggering:
ep41 : okTriggerIn  port map (okHE=>okHE, ep_addr=>x"41", ep_clk=>sys_clk, ep_trigger=>ep41wire);

mod_bus_outs(0).start <= ep40wire(0);   --timing start
mod_bus_outs(0).stop <= '0';
mod_bus_outs(0).ini_addr <= x"00000000";
mod_bus_outs(0).trigger <= hard_triggers(0);

soft_trigger.mode <= ep01wire(0);   --temp; need general scheme for setting trig params of all modules
soft_trigger.triggers <= ep41wire(7 downto 0);
soft_trigger.clear <= ep41wire(8);

--mod_bus_outs(0).start <= ep40wire(0);   --timing start

mod_bus_outs(0).write    <= regWrite;
mod_bus_outs(0).addr_in  <= regAddress;
mod_bus_outs(0).data_in  <= regDataOut;

regDataIn <= mod_bus_ins(0).data_out;  -- from RAM

global_rst <= ep40wire(1);

end ok_controller_arch;
