############## AFG31000 Test Script ##############

# Jason Hogan and Megan Nantel
# August 16th, 2021

# This script reproduces 3 issues we found programming Advanced Mode sequences
# see last section: AFG31000 Usage Example (below)

import pyvisa
import math

############## AFG31000 support class ##############
# pyvisa interface class for programming the AFG31000

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
        # Convert data list to binary string array
        binblob = "".join(
            [AFG31000.valueToBytesString( AFG31000.valueToIntWord(e) ) for e in data]
            )

        # Generate TRACE:DATA command preamble
        traceDataCmd = AFG31000.makeTraceDataCommand(channel, 2*len(data))
        
        command = traceDataCmd + binblob + "\n"
        self.AFG_instrument.write_raw(command)
        
    # Generate a two byte word as string for a 14 bit input value
    @staticmethod
    def valueToBytesString(value) :
        return (chr(AFG31000.getMSBs(value)) + chr(AFG31000.getLSBs(value)))

    # Mask away MSBs
    @staticmethod
    def getLSBs(x):
        return x & 255

    # Bit shift MSBs to the left
    @staticmethod
    def getMSBs(x):
        return (x >> 8)

    # Converts input value in range {-1,1} to a 14-bit int word
    @staticmethod
    def valueToIntWord(x):
        # (2**14 - 1) == 0x3FFF == 16383
        # 16383 / 2 == 8191
        return min( 16383, max(0, int(8191*(x + 1)) ))

    # Generate command preamble of the form "TRACE:DATA EMEM[c],#[N][XX...X]"
    # where [XX...X] is the (decimal) length of the byte string, [N] is the number of
    # digits in [XX...X], and [c] is the channel number.
    @staticmethod
    def makeTraceDataCommand(channel, size):
        return ("TRACE:DATA EMEM" + str(channel) + ",#"
        + str(len(str(size))) + str(size))

    # Add data to Advanced Mode sequence list as a waveform
    def addToSequence(self, index, channel, data):
        # Write binary data to this channel's Edit Memory
        self.write(channel, data)

        # For Advanced Mode, data must be moved from Edit Memory into a file before
        # it can be added to a sequence. Construct file name:
        filename = "\"M:/remote/pulse_" + str(index) + str(channel) + ".tfwx\""
        
        # Delete file if it exists
        self.write_visa("MMEM:DEL " + filename)
        
        # Transfer data from Edit Memory to the above filename (on M: drive)
        store_cmd = "MMEM:STOR:TRAC EMEM" + str(channel) + ", "
        self.write_visa(store_cmd + filename)

        # Now import the file into the waveform list (left panel of Advanced Mode)
        self.write_visa("WLIST:WAV:IMP " + filename)

        # Add waveform to sequence list (right table in Advanced Mode)
        addToSeq_cmd = "SEQ:ELEM" + str(index) + ":WAV" + str(channel) + " "
        self.write_visa(addToSeq_cmd + filename)  #e.g., SEQ:ELEM6:WAV1 ...

    # Enable channel output
    def enableChannel(self, channel, enable=True):
        self.AFG_instrument.write("OUTPUT"+ str(channel) + (" ON" if enable else " OFF"))



############## Generate sample data ##############

# Make pulse of length 'duration' (ns) with 'spectrum' consisting of spectral components of the form
# [(frequency, amplitude, phase), ...]
def makePulse(duration, spectrum):
    sampleRate = 1000       # MS/s
    dt = (1000/sampleRate)  # ns
    units = 1e-3            # MHz * ns

    nPoints = int(sampleRate * duration * units)
        
    data=[]

    # Sample at times 't' spaced by 'dt' in ns.
    
    def makeSinPoint(t, freq, amplitude, phi=0):
        return amplitude * math.sin( 2*math.pi*freq*t*dt*units + (phi * math.pi/180) )

    for t in range(0, nPoints):
        sample = 0
            
        for s in spectrum:  #sum over each spectal componet 's' of the form (freq, amp, [phase])
            sample += makeSinPoint(t, *s)   #unpack spectral component tuple 's'
            
        data.append(sample)
        
    return data

pulseData1 = makePulse(300, [(199, 0.50)])   # 199 MHz sine pulse with duration 300 ns (50% amplitude)
pulseData2 = makePulse(300, [(205, 0.75)])   # 205 MHz sine pulse with duration 300 ns (75% amplitude)



############## AFG31000 usage example ##############

# Create AFG class instance (replace string with appropriate VISA identifier)
afg = AFG31000('TCPIP0::192.168.1.3::inst0::INSTR')

# Create new Advanced Mode sequence
afg.write_visa("WLIST:WAV:DEL ALL")

# Set sequence length
seqLength = 1
afg.write_visa("SEQ:LENG " + str(seqLength))

# Add example data to the Advanced Mode sequence
afg.addToSequence(1, 1, pulseData1)  #add channel 1 data to sequence index 1
afg.addToSequence(1, 2, pulseData2)  #add channel 2 data to sequence index 1

# Enable Wait and set trigger (manual)
afg.write_visa("SEQ:ELEM1:TWA:STAT 1")
afg.write_visa("SEQ:ELEM1:TWA:EVEN MAN")


# ******** ISSUE #1 ********
# Next it seems to be necessary to exit and then re-enter Advanced Mode before running the sequence.
# Otherwise, only channel 1 will work when triggered (channel 2 will either not appear
# or will be time-shifted and asynchronous with trigger). This seems like a bug.
# Exiting and re-entering in this way is very slow and is now the rate-limiting step in our application.
# To reproduce the bug, comment out the following lines:
afg.write_visa("SEQControl:STAT OFF") # Exit Advanced Mode
afg.write_visa("SEQControl:STAT ON")  # Re-enter Advanced Mode


# ******** ISSUE #2 ********
# Exiting Advanced Mode disables both outputs.  Need to re-enable each channel before running the sequence.
# This is also very slow (requires some relays to switch).
afg.enableChannel(1, True)
afg.enableChannel(2, True)


# ******** ISSUE #3 ********
# Need to enable Run mode.  Run mode is disabled during sequence setup.
# Enabling Run mode is slow (requires some relay to switch).
afg.write_visa("SEQControl:RUN")

