----------------------------------------------------------------------------------
-- Company: Stanford University
-- Engineer: Jason Hogan
-- 
-- Create Date: 01/26/2020 08:21:00 PM
-- Design Name: 
-- Module Name: tb_timing_module - Behavioral
-- Project Name: STI
-- Target Devices: 
-- Tool Versions: 
-- Description: Test bench for the timing module. 
--              This includes the timing_core and the BRAM event register.
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

entity tb_timing_module is
end tb_timing_module;

architecture Behavioral of tb_timing_module is

    component timing_module is
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
    
    -- for fake data generation
    signal evt_time   : STD_LOGIC_VECTOR (31 downto 0);
    signal evt_opcode : STD_LOGIC_VECTOR (3 downto 0) := X"0";
    signal evt_val    : STD_LOGIC_VECTOR (27 downto 0) := X"0000000";
    signal time_nb   : STD_LOGIC_VECTOR (3 downto 0);
    signal evt_addr   : STD_LOGIC_VECTOR (31 downto 0);    
    signal evt_data  : STD_LOGIC_VECTOR (63 downto 0);
    signal evt_data_out : STD_LOGIC_VECTOR (31 downto 0);
    
        
    -- timing module: evt_register side
    signal ram_clk  : std_ulogic := '1';
    signal write    : STD_LOGIC;
    signal addr_in  : STD_LOGIC_VECTOR (31 downto 0);
    signal data_in  : STD_LOGIC_VECTOR (31 downto 0);
    signal data_out : STD_LOGIC_VECTOR (31 downto 0);
    
    -- timing_module: timing_core side
    signal start     : STD_LOGIC;
    signal stop      : STD_LOGIC;
    signal ini_addr  : STD_LOGIC_VECTOR (31 downto 0);
    
    -- module under test
    signal stf_play   : STD_LOGIC;
    signal stf_option : STD_LOGIC;
    signal stf_bus    : STD_LOGIC_VECTOR (27 downto 0);
    signal stf_write  : STD_LOGIC;
    signal stf_write_fake : STD_LOGIC := '0';
    signal stf_error : STD_LOGIC;
    signal stf_data  : STD_LOGIC_VECTOR (31 downto 0);
        
    signal stf_pins  : STD_LOGIC_VECTOR (25 downto 0);
begin
    
    MUT: timing_module
    port map 
        (
        reset    => rst,

        ram_clk  => ram_clk,
        write    => write,
        addr_in  => addr_in,
        data_in  => data_in,
        data_out => data_out,

        core_clk => clk,
        start    => start,
        stop     => stop,
        ini_addr => ini_addr,
  

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

    -- fake data generation (when not using real BRAM)
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

        wait until falling_edge(RST);
        
        for j in 1 to 3 loop
          wait until rising_edge(CLK);
        end loop;
        
        start <= '1';
        
        wait until rising_edge(CLK);
        start <= '0';

      --de-activate this process
        wait on rst;

    end process stim_proc;    

    time_nb <= X"0";
    
    data_proc : process (clk)
    begin
        if rising_edge(clk) then
            --fake ram (when not using real BRAM)
            if (   evt_addr = X"00000000") then
                evt_time  <= X"0000000" & time_nb;
                evt_opcode <= X"0";
                evt_val   <= X"000000A";
            elsif (evt_addr = X"00000001") then
                evt_time  <= X"0000000" & time_nb;
                evt_opcode <= X"0"; -- 3=Jump
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
