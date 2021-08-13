import pyvisa
import math
import time
from decimal import Decimal


class AFG31000:
    def __init__(self, visaID):
        self.rm = pyvisa.ResourceManager()
        self.AFG_instrument = self.rm.open_resource(visaID)
        return
    def __del__(self):
        self.AFG_instrument.close()

    def write_visa(self, command):
        return self.AFG_instrument.write(command)
    def query_visa(self, command):
        return self.AFG_instrument.query(command)

    def write(self, channel, data):
        binblob = "".join(
            [AFG31000.valueToBytesString( AFG31000.valueToIntWord(e) ) for e in data]
            )
        #print(binblob)
        #print(len(binblob))
        command = AFG31000.makeTraceDataCommand(channel, 2*len(data)) + binblob + "\n"
        self.AFG_instrument.write_raw(command)

    def addToSequence(self, index, channel, data):

        #Write binary data to Edit Memory
        self.write(channel, data)

        filename = "\"M:/remote/pulse_" + str(index) + str(channel) + ".tfwx\""
        
        #Delete file if it exists
        self.AFG_instrument.write("MMEM:DEL " + filename)
        
        #time.sleep(1)
        #self.AFG_instrument.query('*IDN?')
        #self.write_visa("*OPC?")

        #Save data to file
        store_cmd = "MMEM:STOR:TRAC EMEM" + str(channel) + ", "
        self.AFG_instrument.write(store_cmd + filename)

        #self.write_visa("*OPC?")

        #Import file to waveform list (left panel of Advanced)
        self.AFG_instrument.write("WLIST:WAV:IMP " + filename)

        #self.write_visa("*OPC?")

        addToSeq_cmd = "SEQ:ELEM" + str(index) + ":WAV" + str(channel) + " "
        self.AFG_instrument.write(addToSeq_cmd + filename)  #e.g., SEQ:ELEM6:WAV1 ...

        #self.write_visa("*OPC?")

    def addBothToSequence(self, index, data1, data2):

        #Write binary data to Edit Memory
        self.write(1, data1)
        self.write(2, data2)

        filename1 = "\"M:/remote/pulse_" + str(index) + "1.tfwx\""
        filename2 = "\"M:/remote/pulse_" + str(index) + "2.tfwx\""
        
        #Delete file if it exists
        self.AFG_instrument.write("MMEM:DEL " + filename1)
        self.AFG_instrument.write("MMEM:DEL " + filename2)
        
        #time.sleep(1)
        #self.AFG_instrument.query('*IDN?')
        #self.write_visa("*OPC?")

        #Save data to file
        self.AFG_instrument.write("MMEM:STOR:TRAC EMEM1, " + filename1)
        self.AFG_instrument.write("MMEM:STOR:TRAC EMEM2, " + filename2)

        #self.write_visa("*OPC?")

        #Import file to waveform list (left panel of Advanced)
        self.AFG_instrument.write("WLIST:WAV:IMP " + filename1)
        self.AFG_instrument.write("WLIST:WAV:IMP " + filename2)

        #self.write_visa("*OPC?")

        addToSeq_cmd1 = "SEQ:ELEM" + str(index) + ":WAV1 "
        addToSeq_cmd2 = "SEQ:ELEM" + str(index) + ":WAV2 "
        self.AFG_instrument.write(addToSeq_cmd1 + filename1)  #e.g., SEQ:ELEM6:WAV1 ...
        self.AFG_instrument.write(addToSeq_cmd2 + filename2)  #e.g., SEQ:ELEM6:WAV1 ...

        #self.write_visa("*OPC?")

    def enableExtTrigger(self, index, enabled=True):
        self.AFG_instrument.write("SEQ:ELEM1:TWA:STAT 1")
        self.AFG_instrument.write("SEQ:ELEM" + str(index)
                                  + ":TWAIT:EVENT "
                                  + ("EXT" if enabled else "OFF"))

    def clearSequence(self):
        #self.AFG_instrument.write("SEQ:NEW")
        self.write_visa("WLIST:WAV:DEL ALL")
        #self.setSequenceLength(0)
        
    def setSequenceLength(self, length):
        self.AFG_instrument.write("SEQ:LENG " + str(length))

    def enableChannel(self, channel, enable=True):
        self.AFG_instrument.write("OUTPUT"+ str(channel) + (" ON" if enable else " OFF"))

    #Set rate in MS/s
    def setSamplingRate(self, samplingRate):
        self.write_visa("SEQControl:SRATE " + '%.5E' % Decimal(samplingRate*1e6))

    #Get rate in MS/s
    def getSamplingRate(self):
        rate = self.query_visa("SEQControl:SRATE?")
        return (float(rate) / 1e6)

    def setAmplitude(self, channel, amplitude, units="VPP"):
        self.write_visa("SOUR" + str(channel) + ":VOLT:IMM " + str(amplitude) + units)
        time.sleep(0.1)

    def getAmplitude(self, channel):
        amp = self.query_visa("SOUR" + str(channel) + ":VOLT:LEV?")
        return float(amp)

    #Generate a two byte word as string for a 14 bit input value
    @staticmethod
    def valueToBytesString(value) :
        return (chr(AFG31000.getMSBs(value)) + chr(AFG31000.getLSBs(value)))

    #Mask away MSBs
    @staticmethod
    def getLSBs(x):
        return x & 255

    #Bit shift MSBs to the left
    @staticmethod
    def getMSBs(x):
        return (x >> 8)

    #Converts input value in range {-1,1} to a 14-bit int word
    @staticmethod
    def valueToIntWord(x):
        # (2**14 - 1) == 0x3FFF == 16383
        # 16383 / 2 == 8191
        return min( 16383, max(0, int(8191*(x + 1)) ))

    @staticmethod
    def makeTraceDataCommand(channel, size):
        return ("TRACE:DATA EMEM" + str(channel) + ",#"
        + str(len(str(size))) + str(size))


##
####Test
##
##afg = AFG31000('TCPIP0::192.168.1.3::inst0::INSTR')
##
##num_points=1*2*500
##fc=40
##delta=1
##data1=[]
##data2=[]
##zeros=[]
##for x in range(0,num_points) :
##    #data1.append((1*( math.sin(80.36*x/120))))
##    data1.append((0.5*math.sin(math.pi*(fc-delta)*x/500) - 0.5*math.sin(math.pi*(fc+delta)*x/500)))
##    #data2.append((0.25*( math.sin(2.36*x/120))))
##    zeros.append(0)
##
##
##file1 = open("MyFile.txt","a")
##for e in data1:
##    file1.write(str(e)+"\n")
##
##file1.close()
##
###print(len(data1))
###print(len(zeros))
##
###afg.write(2, data)
##
##afg.clearSequence()
##afg.setSequenceLength(1)
##afg.enableChannel(1, True)
##afg.enableChannel(2, False)
##
##afg.addToSequence(1, 1, data1)
##afg.addToSequence(1, 2, zeros)



##afg.addToSequence(1, 1, data1)
##afg.addToSequence(1, 2, zeros)
##
##afg.addToSequence(2, 1, zeros)
##afg.addToSequence(2, 2, data1)
##
##afg.addToSequence(3, 1, data1)
##afg.addToSequence(3, 2, zeros)

#afg.addToSequence(2, 2, zeros)
#afg.addToSequence(1, 2, data2)





