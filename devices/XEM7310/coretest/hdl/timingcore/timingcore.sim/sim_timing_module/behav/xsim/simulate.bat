@echo off
REM ****************************************************************************
REM Vivado (TM) v2017.3 (64-bit)
REM
REM Filename    : simulate.bat
REM Simulator   : Xilinx Vivado Simulator
REM Description : Script for simulating the design by launching the simulator
REM
REM Generated by Vivado on Sun Jan 26 21:37:04 -0800 2020
REM SW Build 2018833 on Wed Oct  4 19:58:22 MDT 2017
REM
REM Copyright 1986-2017 Xilinx, Inc. All Rights Reserved.
REM
REM usage: simulate.bat
REM
REM ****************************************************************************
call xsim tb_timing_module_behav -key {Behavioral:sim_timing_module:Functional:tb_timing_module} -tclbatch tb_timing_module.tcl -view C:/Users/SrLab/Documents/code/dev/sti-devices/devices/XEM7310/coretest/hdl/timingcore/timingcore.sim/sim_timing_module/tb_timing_module_behav.wcfg -log simulate.log
if "%errorlevel%"=="0" goto SUCCESS
if "%errorlevel%"=="1" goto END
:END
exit 1
:SUCCESS
exit 0
