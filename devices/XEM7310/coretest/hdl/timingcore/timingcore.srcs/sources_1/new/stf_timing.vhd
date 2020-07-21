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

    -- The STF module logic controls the daughter board.
    -- The stf_mod_bus is the standard interface between this device-specific logic and the generic timing_core.
    
    type to_stf_module is record
        stf_bus    : STD_LOGIC_VECTOR (27 downto 0);
        stf_play   : STD_LOGIC;
        stf_option : STD_LOGIC;
        stf_affine : STD_LOGIC;
        stf_reset  : STD_LOGIC;     --clear all errors, return to intitial state, etc.
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

    type stf_mod_bus_array is array (integer range <>) of stf_mod_bus;

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
        pause    : STD_LOGIC;
        trigger  : STD_LOGIC;
        
        ini_addr : STD_LOGIC_VECTOR (31 downto 0);
        debug_code : STD_LOGIC_VECTOR (3 downto 0);
        
        --event register memory
        write    : STD_LOGIC;
        addr_in  : STD_LOGIC_VECTOR (31 downto 0);
        data_in  : STD_LOGIC_VECTOR (31 downto 0);
    end record to_timing_mod;

    type from_timing_mod is record
        --timing core
        err_flag  : STD_LOGIC;
        err_code  : STD_LOGIC_VECTOR (3 downto 0);
        debug_out : STD_LOGIC_VECTOR (31 downto 0);
        
        --event register memory
        data_out : STD_LOGIC_VECTOR (31 downto 0);
        evt_addr : STD_LOGIC_VECTOR (31 downto 0);  --current memory address of timing core (for monitoring state)
    end record from_timing_mod;

    type timing_mod_bus is record
        to_mod   : to_timing_mod;
        from_mod : from_timing_mod;
    end record timing_mod_bus;

--    type Array_ifx_t is array (0 to 2) of ifx_t;
--    type t_Integer_Array is array (integer range <>) of integer;
    type to_timing_mod_array is array (integer range <>) of to_timing_mod;
    type from_timing_mod_array is array (integer range <>) of from_timing_mod;
    type timing_mod_bus_array is array (integer range <>) of timing_mod_bus;
    
    -- Collection of software trigger signals
    type trigger_bus is record
        clear    : STD_LOGIC;     -- set all triggers to zero
        mode     : STD_LOGIC;     -- 0=software, 1=hardware
        triggers : STD_LOGIC_VECTOR (7 downto 0);      --all modules
    end record trigger_bus;


end stf_timing;
