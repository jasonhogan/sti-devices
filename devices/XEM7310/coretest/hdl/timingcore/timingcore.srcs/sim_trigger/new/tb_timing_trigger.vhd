----------------------------------------------------------------------------------
-- Company: Stanford University
-- Engineer: Jason Hogan
-- 
-- Create Date: 07/18/2020 17:43:20 PM
-- Design Name: 
-- Module Name: tb_timing_trigger - Behavioral
-- Project Name: STI
-- Target Devices: 
-- Tool Versions: 
-- Description: Test bench for timing_trigger module
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

entity tb_timing_trigger is
end tb_timing_trigger;

architecture Behavioral of tb_timing_trigger is

    component timing_trigger
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
    end component;

    constant HALF_PERIOD    : time := 5 ns; --100 MHz

    signal clk  : std_ulogic := '1';
    signal rst  : STD_LOGIC := '1';
      
    -- module under test inputs
    signal clear       : STD_LOGIC;
    signal mode        : STD_LOGIC;    
    signal ext_trigger : STD_LOGIC;
    signal trigger_in  : STD_LOGIC_VECTOR (7 downto 0);
        
    -- module under test outputs 
    signal trigger_out : STD_LOGIC_VECTOR (7 downto 0);
    
    
begin
    
    MUT: timing_trigger
    generic map (N   => 8)
    port map 
        (
        clk      => clk,
        reset    => rst,
        
        clear     => clear,
        mode    => mode,
        ext_trigger     => ext_trigger,
        
        trigger_in  => trigger_in,
        trigger_out => trigger_out
        );

    clk <= '0' after HALF_PERIOD when clk = '1' else
           '1' after HALF_PERIOD;

    Reset_Gen : process
    begin
      -- generate reset
      for i in 1 to 5 loop
        if (i < 4) then
            rst <= '1';
        else
            rst <= '0';
        end if;
        wait until falling_edge(clk);
      end loop;
      
      --de-activate this process
      wait on RST;
    end process Reset_Gen;

    stim_proc : process
    begin
        clear <= '0';
        mode <= '1';
        ext_trigger <= '0';
        trigger_in <= X"00";
        
        wait until falling_edge(RST);
        
        for j in 1 to 3 loop
          --wait until falling_edge(CLK);
          wait until rising_edge(CLK);
        end loop;

        trigger_in <= X"0F";
        wait until rising_edge(CLK);
        trigger_in <= X"00";

        for j in 1 to 5 loop
          wait until rising_edge(CLK);
        end loop;
        
        trigger_in <= X"40";
        wait until rising_edge(CLK);
        trigger_in <= X"00";
        
        for j in 1 to 6 loop
          wait until rising_edge(CLK);
        end loop;
                
        ext_trigger <= '1';
        
        for j in 1 to 3 loop
          wait until rising_edge(CLK);
        end loop;
        
        trigger_in <= X"cc";
        wait until rising_edge(CLK);
        trigger_in <= X"00";
        
        for j in 1 to 3 loop
          wait until rising_edge(CLK);
          --clear <= '1';
        end loop;
        clear <= '0';
        
        ext_trigger <= '0';
        wait until rising_edge(CLK);
        ext_trigger <= '1';
        
      --de-activate this process
        wait on RST;

    end process stim_proc;    


end Behavioral;
