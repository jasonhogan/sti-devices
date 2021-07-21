import pyvisa
import math
import time

class AFG31000:
    def __init__(self, visaID):
        self.rm = pyvisa.ResourceManager()
        self.AFG_instrument = self.rm.open_resource(visaID)
        return
    def __del__(self):
        self.AFG_instrument.close()

    def write(self, channel, data):
        binblob = "".join(
            [AFG31000.valueToBytesString( AFG31000.valueToIntWord(e) ) for e in data]
            )
        print(len(binblob))
        command = AFG31000.makeTraceDataCommand(channel, 2*len(data)) + binblob + "\n"
        print(self.AFG_instrument.write_raw(command))

    def addToSequence(self, index, channel, data):

        self.write(channel, data)

        filename = "\"M:/remote/pulse_" + str(index) + str(channel) + ".tfwx\""
        
        #Delete file if it exists
        self.AFG_instrument.write("MMEM:DEL " + filename)
        
        
        #time.sleep(1)
        #self.AFG_instrument.query('*IDN?')

        #Save data to file
        store_cmd = "MMEM:STOR:TRAC EMEM" + str(channel) + ", "
        self.AFG_instrument.write(store_cmd + filename)

        #Import file to waveform list (left panel of Advanced)
        self.AFG_instrument.write("WLIST:WAV:IMP " + filename)

        addToSeq_cmd = "SEQ:ELEM" + str(index) + ":WAV" + str(channel) + " "
        self.AFG_instrument.write(addToSeq_cmd + filename)  #e.g., SEQ:ELEM6:WAV1 ...

    def clearSequence(self):
        self.AFG_instrument.write("SEQ:NEW")
        
    def setSequenceLength(self, length):
        self.AFG_instrument.write("SEQ:LENG " + str(length))

    def enableChannel(self, channel, enable=True):
        self.AFG_instrument.write("OUTPUT"+ str(channel) + (" ON" if enable else " OFF"))

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



##Test

afg = AFG31000('TCPIP0::192.168.1.3::inst0::INSTR')

num_points=1*2*500
fc=40
delta=1
data1=[]
data2=[]
zeros=[]
for x in range(0,num_points) :
    #data1.append((1*( math.sin(80.36*x/120))))
    data1.append((0.5*math.sin(math.pi*(fc-delta)*x/500) - 0.5*math.sin(math.pi*(fc+delta)*x/500)))
    #data2.append((0.25*( math.sin(2.36*x/120))))
    zeros.append(0)


file1 = open("MyFile.txt","a")
for e in data1:
    file1.write(str(e)+"\n")

file1.close()

#print(len(data1))
#print(len(zeros))

#afg.write(2, data)

afg.clearSequence()
afg.setSequenceLength(1)
afg.enableChannel(1, True)
afg.enableChannel(2, False)

afg.addToSequence(1, 1, data1)
afg.addToSequence(1, 2, zeros)



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





