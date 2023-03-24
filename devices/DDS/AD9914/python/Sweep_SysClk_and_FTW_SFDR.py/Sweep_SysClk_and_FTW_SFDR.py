import sys,time
sys.path.append('z:/neweeks/python')
import Gpib_Handler
import Functions as io
from Class import AD9914
from gmpy import mpq,mpf
from math import log

dds=AD9914.ad9914(reset=True)

instruments=[]

gpib=Gpib_Handler.gpib()
sma=Gpib_Handler.instr()
sma.name='sysclk'
sma.type='clk'
sma.pref='sma'
sma.addr=30
instruments.append(sma)

fse=Gpib_Handler.instr()
fse.name='DAC out'
fse.type='spec an'
fse.pref='fsu'
fse.addr=14
instruments.append(fse)

gpib.Handler(instruments,Verify=0)

#-------------------------------------------------------------------------------
# Test Parameters
#-------------------------------------------------------------------------------
sysclk_low  = 0.5e9
sysclk_high = 4.0e9
sysclk_step = 1e8

dac_out_low  = 10e6
dac_out_high = "1/2" #Ratio with respect to system clock
NUMsteps = 100

rbw=50e3        # Default: 50e3 (Hz)
RFatten=20      # Default: 20 (dB)

carrier_zoom=False

NUMspurs = 3

# Start with the high end of SysCLK frequencies
start_high=False

# Amplitude Scale Factor to full scale
Full_scale_Amplitude=False

# Naming
dut_number=1
dut_name="cr1_socket"

part = "AD9914"
dir="Z:\\AD9914\\AD9914c_r1\\AutoTest\\SFDR\\No_ate_screen\\"
extension=".csv"

#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
fse.gpib.ResBand=rbw
fse.gpib.AttLevel=RFatten

#-------------------------------------------------------------------------------

test="DACout_SFDR_vs_FTW_Sweep"
param=["GND_Ext_Sync"]

if Full_scale_Amplitude:
    dds.write(0x0C,[None,None,0xFF,0x0F],1)
    param.append('ASF=Full')

ranges=range(int(sysclk_low),int(sysclk_high+sysclk_step),int(sysclk_step))
sysclk_in=sorted(ranges,reverse=start_high)

# If you loop above this resolve naming issue with dac_out_high
ratio=mpq(int(dac_out_high.split("/")[0]),int(dac_out_high.split("/")[1]))

io.mkdir(dir)

fse.gpib.instr.write("INIT:CONT OFF")
fse.gpib.instr.write("FREQ:STAR %f" % (1e6))

for sysclk_freq in sysclk_in:
    sma.gpib.Frequency=sysclk_freq
    fse.gpib.instr.write("FREQ:STOP %f" % ((sysclk_freq/2.)+20e6))
    param.append("SysCLK=%4.2fGHz" % (sysclk_freq/1e9))
    time.sleep(0.2)
    dds.CAL()
    # Enable Profile
    dds.write(0x01,[None,None,0x80,None],1)
    time.sleep(0.1)
    #-------------------------------------------------------------------------------
    # Create DAC out Array
    #-------------------------------------------------------------------------------
    dac_out_high=float(mpf(sysclk_freq)*ratio)
    dac_out_step=float(mpq((log(dac_out_high,10)-log(dac_out_low,10)),NUMsteps))
    arry=[log(dac_out_low,10)]
    i=0
    while(max(arry)<log(dac_out_high,10)):
        i+=1
        arry.append(log(dac_out_low,10)+i*dac_out_step)
    #-------------------------------------------------------------------------------
    csvout=["SysCLK (GHz)=%f\n" % (sysclk_freq/1e9),"Intended DACout (MHz),,Measured DACout Frequency(MHz),Power(dBm),,"]
    for i in range(NUMspurs):
        csvout[2]+="Spur Frequency Offset (MHz),Spur power (dBc),,"
    csvout[2]+="\n"
    for power in arry:
        spurs=[]
        DACout=10**power
        if arry.index(power)==(len(arry)-1): DACout-=10e6
        ftw=dds.calcFTW(sysclk_freq,DACout)
        time.sleep(0.05)
        #------------------------ FSE Sweep --------------------------------------------
        fse.gpib.sweep()
        #-------------------------------------------------------------------------------
        spurs=fse.gpib.__GetSpurs__(NUMspurs+1,carrier_zoom)
        csvout.append("%f,,%f,%f,," %(DACout*1e-6,spurs[0][0],spurs[0][1]))
        numspurs=(len(spurs)-1)
        for spur_num in range(numspurs):
            csvout[arry.index(power)+3]+="%f,%f,," %(spurs[1+spur_num][0],(spurs[1+spur_num][1]))
        csvout[arry.index(power)+3]+="\n"
    filename=io.naming(test,part,param,dut_name,dut_number,extension)
    filename= dir+filename
    open(filename,'w').writelines(csvout)
    param.remove("SysCLK=%4.2fGHz" % (sysclk_freq/1e9))

fse.gpib.instr.write("INIT:CONT ON")