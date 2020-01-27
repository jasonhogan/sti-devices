----------------------------------------------------------------------------------
-- Company: Stanford University
-- Engineer: Jason Hogan
-- 
-- Create Date: 01/03/2020 08:53:00 PM
-- Design Name: 
-- Module Name: tb_timing_core - Behavioral
-- Project Name: STI
-- Target Devices: 
-- Tool Versions: 
-- Description: Test bench for the timing core.
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

entity tb_timing_core is
end tb_timing_core;

architecture Behavioral of tb_timing_core is

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
          stf_write  : in STD_LOGIC;                        -- The stf module asserts this when data is ready
          stf_error  : in STD_LOGIC
            );
    end component;

    component stf_output
    Port ( 
       clk        : in STD_LOGIC;
       reset      : in STD_LOGIC;

       stf_bus    : in STD_LOGIC_VECTOR (27 downto 0);
       stf_play   : in STD_LOGIC;
       stf_option : in STD_LOGIC;
       stf_write  : out STD_LOGIC;
       stf_error  : out STD_LOGIC;
       
       stf_pins   : out STD_LOGIC_VECTOR (25 downto 0)
          );
    end component;

    constant HALF_PERIOD    : time := 5 ns; --100 MHz
    
    --constant wr_delay    : time := 120 ns; --100 MHz

    signal clk  : std_ulogic := '1';
    signal rst  : STD_LOGIC := '1';
    
    signal evt_time   : STD_LOGIC_VECTOR (31 downto 0);
    signal evt_opcode : STD_LOGIC_VECTOR (3 downto 0) := X"0";
    signal evt_val    : STD_LOGIC_VECTOR (27 downto 0) := X"0000000";
    
    signal time_nb   : STD_LOGIC_VECTOR (3 downto 0);
    
    -- module under test inputs
    signal start     : STD_LOGIC;
    signal stop      : STD_LOGIC;
    signal ini_addr  : STD_LOGIC_VECTOR (31 downto 0);
    signal evt_data  : STD_LOGIC_VECTOR (63 downto 0);
    signal evt_data_out : STD_LOGIC_VECTOR (31 downto 0);
    signal stf_write : STD_LOGIC;
    signal stf_write_fake : STD_LOGIC := '0';
    signal stf_error : STD_LOGIC;
    signal stf_data  : STD_LOGIC_VECTOR (31 downto 0);
        
    -- module under test outputs
    signal evt_addr   : STD_LOGIC_VECTOR (31 downto 0);
    signal write      : STD_LOGIC;
    signal stf_play   : STD_LOGIC;
    signal stf_option : STD_LOGIC;
    signal stf_bus    : STD_LOGIC_VECTOR (27 downto 0);
    
    signal stf_pins  : STD_LOGIC_VECTOR (25 downto 0);
begin
    
    MUT: timing_core
    port map 
        (
        clk      => clk,
        reset    => rst,

        start    => start,
        stop     => stop,
        ini_addr => ini_addr,
  
        evt_addr => evt_addr,
        evt_data => evt_data,
        write    => write,
        evt_data_out => evt_data_out,
  
        -- STF module
        stf_bus    => stf_bus,
        stf_data   => stf_data,
        stf_play   => stf_play,
        stf_option => stf_option,
        stf_write  => stf_write_fake,
        stf_error  => stf_error
        );

    digital: stf_output
    Port map (
       clk       => clk,
       reset     => rst,
       stf_bus   => stf_bus,
       stf_play  => stf_play,
       stf_option => stf_option,
       stf_write => stf_write,
       stf_error => stf_error,
       stf_pins  => stf_pins
        );

    clk <= '0' after HALF_PERIOD when clk = '1' else
           '1' after HALF_PERIOD;
           
    evt_data <= evt_time & evt_opcode & evt_val;

    --stf_write_fake <= '1' after 141 ns when stf_write_fake = '0' else '0' after 10 ns;

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
    
        evt_data_out <= X"0000ABCD";

        start     <= '0';
        stop      <= '0';
        ini_addr  <= X"00000000";
       -- ini_addr  <= X"FFFFFFFF";
       -- evt_time  <= X"00000003";
       -- evt_val   <= X"00000000";
        --stf_write <= '0';
        --done      <= '0';

        wait until falling_edge(RST);
        
        for j in 1 to 3 loop
          --wait until falling_edge(CLK);
          wait until rising_edge(CLK);
        end loop;
        
        start <= '1';
        
        wait until rising_edge(CLK);
        start <= '0';



--        evt_val <= X"000d1111";

--        for j in 1 to 10 loop
--          wait until rising_edge(CLK);
--        end loop;
        
--        evt_val <= X"000D1111";
--        wait until rising_edge(CLK);
--        evt_val <= X"000D2222";
--        wait until rising_edge(CLK);
--        evt_val <= X"000D3333";
--        wait until rising_edge(CLK);
--        evt_val <= X"000D4444";        
        
      --de-activate this process
        wait on rst;

    end process stim_proc;    

--    data_proc : process
--    begin
----        if rising_edge(clk) then
----            evt_val <= evt_addr;
----        end if;
--        wait until rising_edge(CLK);
--        evt_val <= evt_addr;
--        --evt_time <= evt_addr;
--    end process data_proc;   


    time_nb <= X"2";
    
    data_proc : process (clk)
    begin
        if rising_edge(clk) then
            --fake ram
            if (   evt_addr = X"00000000") then
                evt_time  <= X"0000000" & time_nb;
                evt_opcode <= X"0";
                evt_val   <= X"000000A";
            elsif (evt_addr = X"00000001") then
                evt_time  <= X"0000000" & time_nb;
                evt_opcode <= X"6"; -- 3=Jump
                evt_val   <= X"000004F";
            elsif (evt_addr = X"00000002") then
                evt_time  <= X"0000000" & time_nb;
                --evt_time  <= X"00000006";
                evt_opcode <= X"0";    
                evt_val   <= X"000000C";
            elsif (evt_addr = X"00000003") then
                evt_time  <= X"0000000" & time_nb;
                evt_opcode <= X"0";
                evt_val   <= X"000000D";
            elsif (evt_addr = X"00000004") then
                evt_time  <= X"0000000" & time_nb;
                --evt_time  <= X"00000001";
                evt_opcode <= X"2"; --End
                evt_val   <= X"000000E";
            elsif (evt_addr = X"00000005") then
                evt_time  <= X"0000000" & time_nb;
                evt_opcode <= X"0";
                evt_val   <= X"000000F";
            elsif (evt_addr = X"0000004F") then
                evt_time  <= X"0000000" & time_nb;
                evt_opcode <= X"0";
                evt_val   <= X"00000FF";
            elsif (evt_addr = X"00000050") then
                evt_time  <= X"0000000" & time_nb;
                --evt_time  <= X"00000001";
                evt_opcode <= X"0";
                evt_val   <= X"00000AA";
            elsif (evt_addr = X"00000051") then
                evt_time  <= X"0000000" & time_nb;
                
                evt_opcode <= X"2";
                evt_val   <= X"00000BB";
            else
                evt_time  <= X"00000BAD";
                evt_opcode <= X"0";
                evt_val   <= X"0000BAD";           
            end if;
        end if;

    end process data_proc;   

end Behavioral;
