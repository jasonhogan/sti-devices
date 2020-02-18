----------------------------------------------------------------------------------
-- Company: Stanford University
-- Engineer: Jason Hogan
-- 
-- Create Date: 01/28/2020 09:35:34 PM
-- Design Name: 
-- Module Name: timing_mod_test_top - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: Test of full timing module (for digital board)
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
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

use work.stf_timing.all;

Library UNISIM;
use UNISIM.vcomponents.all;

entity timing_mod_test_top is
	port (
	
        okUH      : in     STD_LOGIC_VECTOR(4 downto 0);
        okHU      : out    STD_LOGIC_VECTOR(2 downto 0);
        okUHU     : inout  STD_LOGIC_VECTOR(31 downto 0);
        okAA      : inout  STD_LOGIC;
    
        sys_clkp  : in     STD_LOGIC;
        sys_clkn  : in     STD_LOGIC;
        
        led       : out    STD_LOGIC_VECTOR(7 downto 0);
    
        xbusp     :    out  STD_LOGIC_VECTOR(29 downto 0);
        xbusn     :    out  STD_LOGIC_VECTOR(29 downto 0);
        ybusp     :    out  STD_LOGIC_VECTOR(29 downto 0);
        ybusn     :    out  STD_LOGIC_VECTOR(29 downto 0)
    );
end timing_mod_test_top;

architecture Behavioral of timing_mod_test_top is

    component ok_controller
    Port ( 
           sys_clk   : in STD_LOGIC;
           okClk     : out STD_LOGIC;
           
           global_rst : out STD_LOGIC;

           -- Timing module bus
           mod_bus_out : out to_timing_mod;
           mod_bus_in  : in from_timing_mod;
           
           okUH      : in     STD_LOGIC_VECTOR(4 downto 0);
           okHU      : out    STD_LOGIC_VECTOR(2 downto 0);
           okUHU     : inout  STD_LOGIC_VECTOR(31 downto 0);
           okAA      : inout  STD_LOGIC
        );
    end component;

    component timing_module is
        Port ( 
           reset    : in STD_LOGIC;

           -- Event register           
           ram_clk  : in STD_LOGIC;
           
--           write    : in STD_LOGIC;
--           addr_in  : in STD_LOGIC_VECTOR (31 downto 0);
--           data_in  : in STD_LOGIC_VECTOR (31 downto 0);
--           data_out : out STD_LOGIC_VECTOR (31 downto 0);

           -- Timing core
           core_clk : in STD_LOGIC;
           
--           start    : in STD_LOGIC;
--           stop     : in STD_LOGIC;
--           ini_addr : in STD_LOGIC_VECTOR (31 downto 0);        
           
           -- Timing module bus
           mod_bus_in  : in to_timing_mod;
           mod_bus_out : out from_timing_mod;
           
           -- STF module
           stf_out : out to_stf_module;
           stf_in  : in from_stf_module
         
         );
    end component;
    
    component stf_output
    Port ( 
           clk        : in STD_LOGIC;
           reset      : in STD_LOGIC;
           
           -- STF module
           stf_in  : in to_stf_module;
           stf_out : out from_stf_module;
                  
           stf_pins   : out STD_LOGIC_VECTOR (25 downto 0)
          );
    end component;

    signal sys_clk  : std_ulogic;
    signal okClk  : std_ulogic;
    signal global_rst : std_ulogic;
    
    signal stf_mod   : stf_mod_bus;
    signal mod0_bus : timing_mod_bus;

    signal stf_pins  : STD_LOGIC_VECTOR (25 downto 0);
    signal stf_pins_buffer : STD_LOGIC_VECTOR (25 downto 0); 

    signal xemJ2 : STD_LOGIC_VECTOR(40 downto 1);   -- Connector J2 on XEM breakout (40 pin)
    signal stfJP0 : STD_LOGIC_VECTOR(26 downto 1);   -- Connector JP0 on any STF daughter board (34 pin, first 26 are logic)

begin

    osc_clk : IBUFGDS port map (O=>sys_clk, I=>sys_clkp, IB=>sys_clkn);

    ok_control : ok_controller
    port map
         ( 
           sys_clk => sys_clk,
           okClk   => okClk,
           
           global_rst => global_rst,
                      
           mod_bus_out => mod0_bus.to_mod,
           mod_bus_in  => mod0_bus.from_mod,
           
           okUH     => okUH,
           okHU     => okHU,
           okUHU    => okUHU,
           okAA     => okAA
        );

    mod0 : timing_module
    port map 
        (
            reset    => global_rst,
    
            ram_clk  => okClk,
            core_clk => sys_clk,
            
            -- Timing module bus
            mod_bus_in => mod0_bus.to_mod,
            mod_bus_out => mod0_bus.from_mod,
      
            -- STF module
            stf_out => stf_mod.to_stf,
            stf_in  => stf_mod.from_stf

        );

    digital: stf_output
    Port map (
           clk       => sys_clk,
           reset     => global_rst,

           stf_in  => stf_mod.to_stf,
           stf_out => stf_mod.from_stf,
           
           stf_pins  => stf_pins
        );


--stfJP0 <= stf_pins(25 downto 0);

    Reg_Proc: process (sys_clk, global_rst)
    begin
      if (global_rst = '1') then
        stf_pins_buffer <= X"000000" & '0' & '0';
        
      elsif rising_edge(sys_clk) then
        stf_pins_buffer <= stf_pins;
      end if;
    end process;



stfJP0 <= stf_pins_buffer(25 downto 0);

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

end Behavioral;
