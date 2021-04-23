----------------------------------------------------------------------------------
-- Company: Stanford University
-- Engineer: Jason Hogan
-- 
-- Create Date: 01/03/2020 06:38:26 PM
-- Design Name: 
-- Module Name: tb_stf_output - Behavioral
-- Project Name: STI
-- Target Devices: 
-- Tool Versions: 
-- Description: Test bench for stf_output module
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

entity tb_stf_output is
end tb_stf_output;

architecture Behavioral of tb_stf_output is

    component stf_output
      Port ( 
          clk      : in STD_LOGIC;
          reset    : in STD_LOGIC;
    
          stf_bus  : in STD_LOGIC_VECTOR (25 downto 0);
          play     : in STD_LOGIC;
          write    : out STD_LOGIC;
          done     : out STD_LOGIC;
          
          stf_pins : out STD_LOGIC_VECTOR (25 downto 0)
         );
    end component;

    constant HALF_PERIOD    : time := 5 ns; --100 MHz

    signal clk  : std_ulogic := '1';
    signal rst  : STD_LOGIC := '1';
      
    -- module under test inputs
    signal stf_bus  : STD_LOGIC_VECTOR (31 downto 0);
    signal play     : STD_LOGIC;
    
    -- module under test outputs 
    signal write    : STD_LOGIC;
    signal done     : STD_LOGIC;
    signal stf_pins : STD_LOGIC_VECTOR (25 downto 0);
    
    
begin
    
    MUT: stf_output
    port map 
        (
        clk      => clk,
        reset    => rst,
        
        stf_bus  => stf_bus(25 downto 0),
        play     => play,
        write    => write,
        done     => done,
        
        stf_pins => stf_pins
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
        play <= '0';
        stf_bus <= X"00000000";
        
        wait until falling_edge(RST);
        
        for j in 1 to 3 loop
          --wait until falling_edge(CLK);
          wait until rising_edge(CLK);
        end loop;

        stf_bus <= X"0110ABCD";
        play <= '1';
        
        wait until rising_edge(CLK);
        
        play <= '0';

        for j in 1 to 10 loop
          wait until rising_edge(CLK);
        end loop;
        
        play <= '1';
        
        -- burst mode test
        stf_bus <= X"000D1111";
        wait until rising_edge(CLK);
        stf_bus <= X"000D2222";
        wait until rising_edge(CLK);
        stf_bus <= X"000D3333";
        wait until rising_edge(CLK);
        stf_bus <= X"000D4444";        
        
      --de-activate this process
        wait on RST;

    end process stim_proc;    


end Behavioral;
