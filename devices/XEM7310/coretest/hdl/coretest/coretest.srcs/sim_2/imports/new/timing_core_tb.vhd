----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 01/03/2020 06:38:26 PM
-- Design Name: 
-- Module Name: timing_core_tb - Behavioral
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

entity timing_core_tb is
end timing_core_tb;

architecture Behavioral of timing_core_tb is

    component timing_core2
      Port ( 
           clk    : in STD_LOGIC;
           reset  : in STD_LOGIC;
           
           go     : in STD_LOGIC;
           kill   : in STD_LOGIC;
           
           done : out STD_LOGIC
         );
    end component;

    constant HALF_PERIOD    : time := 5 ns; --100 MHz

    signal CLK : std_ulogic := '1';
    signal RST    : STD_LOGIC := '1';
    
    signal go   : STD_LOGIC := '0';
    signal kill : STD_LOGIC := '0';
    signal done : STD_LOGIC;
    
begin
    
    MUT: timing_core2
    port map 
        (
        clk    => CLK,
        reset  => RST,
        
        go     => go,
        kill   => kill,
        
        done => done
        );

    CLK <= '0' after HALF_PERIOD when CLK = '1' else
           '1' after HALF_PERIOD;

    Reset_Gen : process
    begin
      -- generate reset
      for i in 1 to 5 loop
        if (i < 4) then
            RST <= '1';
        else
            RST <= '0';
        end if;
        wait until falling_edge(CLK);
      end loop;
      
      --de-activate this process
      wait on RST;
    end process Reset_Gen;

    stim_proc : process
    begin
        go <= '0';
        kill <= '0';
        
        wait until falling_edge(RST);
        
        for j in 1 to 3 loop
          wait until falling_edge(CLK);
        end loop;
        
        --wait for 100 ns;
        
        go <= '1';
      
        for k in 1 to 50 loop
          wait until falling_edge(CLK);
        end loop;
        
        kill <= '1';
        
      --de-activate this process
        wait on RST;

--        wait for 10 ns;
--        trigger <= '0';
--        wait for 500 ns;
    end process stim_proc;    


end Behavioral;
