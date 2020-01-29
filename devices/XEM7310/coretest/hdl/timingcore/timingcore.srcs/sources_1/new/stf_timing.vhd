----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 01/28/2020 06:34:14 PM
-- Design Name: 
-- Module Name: stf_timing - Behavioral
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

package stf_timing is

    type to_stf_module is record
        stf_bus    : STD_LOGIC_VECTOR (27 downto 0);
        stf_play   : STD_LOGIC;
        stf_option : STD_LOGIC;
    end record to_stf_module;
    
    type from_stf_module is record
        stf_data     : STD_LOGIC_VECTOR (31 downto 0);
        stf_write    : STD_LOGIC;
        stf_error    : STD_LOGIC;
        stf_err_code : STD_LOGIC_VECTOR (3 downto 0);
    end record from_stf_module;

    type stf_mod_bus is record
        to_stf   : to_stf_module;
        from_stf : from_stf_module;
    end record stf_mod_bus;


--    type to_timing_core is record
--        start    : STD_LOGIC;
--        stop     : STD_LOGIC;
--        ini_addr : STD_LOGIC_VECTOR (31 downto 0);
--    end record to_timing_core;

--    type from_timing_core is record
--        error    : STD_LOGIC;
--        err_code : STD_LOGIC_VECTOR (31 downto 0);
--    end record from_timing_core;
    
--    type to_evt_register is record      
--        write    : STD_LOGIC;
--        addr_in  : STD_LOGIC_VECTOR (31 downto 0);
--        data_in  : STD_LOGIC_VECTOR (31 downto 0);
--    end record to_evt_register;
    
--    type from_evt_register is record    
--        data_out : STD_LOGIC_VECTOR (31 downto 0);
--    end record from_evt_register;

--    type timing_mod_bus is record
--        to_core       : to_timing_core;
--        from_core     : from_timing_core;
--        to_evt_ram    : to_evt_register;
--        from_register : from_evt_register;
--    end record timing_mod_bus;


    type to_timing_mod is record
        --timing core
        start    : STD_LOGIC;
        stop     : STD_LOGIC;
        ini_addr : STD_LOGIC_VECTOR (31 downto 0);
        
        --event register memory
        write    : STD_LOGIC;
        addr_in  : STD_LOGIC_VECTOR (31 downto 0);
        data_in  : STD_LOGIC_VECTOR (31 downto 0);
    end record to_timing_mod;

    type from_timing_mod is record
        --timing core
        error    : STD_LOGIC;
        err_code : STD_LOGIC_VECTOR (31 downto 0);
        
        --event register memory
        data_out : STD_LOGIC_VECTOR (31 downto 0);
    end record from_timing_mod;

    type timing_mod_bus is record
        to_mod   : to_timing_mod;
        from_mod : from_timing_mod;
    end record timing_mod_bus;

end stf_timing;
