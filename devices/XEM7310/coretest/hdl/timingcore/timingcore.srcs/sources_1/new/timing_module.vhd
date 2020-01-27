----------------------------------------------------------------------------------
-- Company: Stanford University
-- Engineer: Jason Hogan
-- 
-- Create Date: 01/18/2020 05:23:37 PM
-- Design Name: STF bus timing module
-- Module Name: timing_module - Behavioral
-- Project Name: STI
-- Target Devices: XEM7310-A200 (xc7a200tfbg484-1)
-- Tool Versions: 
-- Description: Timing module for driving and STF daughter board.
--              Events from an event register are loaded by the timing core
--              and set to the STF bus.
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

entity timing_module is
    Port ( 
           reset    : in STD_LOGIC;

           -- Event register           
           ram_clk  : in STD_LOGIC;
           write    : in STD_LOGIC;
           addr_in  : in STD_LOGIC_VECTOR (31 downto 0);
           data_in  : in STD_LOGIC_VECTOR (31 downto 0);
           data_out : out STD_LOGIC_VECTOR (31 downto 0);

           -- Timing core
           core_clk : in STD_LOGIC;
           start    : in STD_LOGIC;
           stop     : in STD_LOGIC;
           ini_addr : in STD_LOGIC_VECTOR (31 downto 0);        
           
           -- STF module
           stf_bus    : out STD_LOGIC_VECTOR (27 downto 0);
           stf_data   : in STD_LOGIC_VECTOR (31 downto 0);
           stf_play   : out STD_LOGIC;
           stf_option : out STD_LOGIC;
           stf_write  : in STD_LOGIC;
           stf_error  : in STD_LOGIC
         );
end timing_module;

architecture timing_module_arch of timing_module is

    component blk_mem_gen_0
     port (
       clka     : IN STD_LOGIC;
       wea      : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
       addra    : IN STD_LOGIC_VECTOR(10 DOWNTO 0);
       dina     : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
       douta    : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
       clkb     : IN STD_LOGIC;
       web      : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
       addrb    : IN STD_LOGIC_VECTOR(9 DOWNTO 0);
       dinb     : IN STD_LOGIC_VECTOR(63 DOWNTO 0);
       doutb    : OUT STD_LOGIC_VECTOR(63 DOWNTO 0)
     );
    end component;

    component timing_core
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
       evt_data_out : out STD_LOGIC_VECTOR (31 downto 0);
           
       -- STF module
       stf_bus    : out STD_LOGIC_VECTOR (27 downto 0);
       stf_data   : in STD_LOGIC_VECTOR (31 downto 0);
       stf_play   : out STD_LOGIC;
       stf_option : out STD_LOGIC;
       stf_write  : in STD_LOGIC;
       stf_error  : in STD_LOGIC
      );
    end component;

    signal writeA     : STD_LOGIC_VECTOR(0 DOWNTO 0);
    signal writeB     : STD_LOGIC_VECTOR(0 DOWNTO 0);
    signal core_write : STD_LOGIC;

    signal evt_addr : STD_LOGIC_VECTOR(31 downto 0);
    signal evt_data : STD_LOGIC_VECTOR(63 downto 0);
    signal evt_data_out : STD_LOGIC_VECTOR(31 downto 0);
    signal evt_data_cat : STD_LOGIC_VECTOR(63 downto 0);

begin

writeA <= "1" when (write = '1') else "0";
writeB <= "1" when (core_write = '1') else "0";

--evt_data_out <= X"0000000000000000";    --temp

evt_data_cat <= evt_data(63 downto 32) & evt_data_out;  --keep existing time, replace value with new data

evt_register : blk_mem_gen_0
  PORT MAP (
    -- OK bus
    clka => ram_clk,
    wea => writeA,
    addra => addr_in(10 downto 0),
    dina => data_in,
    douta => data_out,
    
    -- Timing core
    clkb => core_clk,
    web => writeB,
    addrb => evt_addr(9 downto 0),
    dinb => evt_data_cat,
    doutb => evt_data
  );

core : timing_core
port map 
    (
    clk      => core_clk,
    reset    => reset,
    
    -- Core control
    start    => start,
    stop     => stop,
    ini_addr => ini_addr,

    -- Event register
    evt_addr => evt_addr,
    evt_data => evt_data,
    write => core_write,
    evt_data_out => evt_data_out,

    -- STF module
    stf_bus    => stf_bus,
    stf_data   => stf_data,
    stf_play   => stf_play,
    stf_option => stf_option,
    stf_write  => stf_write,
    stf_error  => stf_error
    );

end timing_module_arch;
