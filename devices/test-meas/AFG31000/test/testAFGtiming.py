import pyvisa
import math
import timeit

rm = pyvisa.ResourceManager()
my_instrument=rm.open_resource('TCPIP0::192.168.1.3::inst0::INSTR')

#message = '*IDN?'
#s = my_instrument.query(message)


def valueToIntWord(x):
    if x > 1:
        return (2**14 - 1)
    elif x < -1:
        return 0
    else:
        return int((2**14 - 1)*((x + 1)/2.0))

def valueToIntWord2(x):
    return min( (2**14 - 1), max(0, int((2**14 - 1)*((x + 1)/2.0))) )

def valueToIntWord3(x):
    # (2**14 - 1) == 0x3FFF == 16383
    # 16383 / 2 == 8191
    return min( 16383, max(0, int(8191*(x + 1)) ))

def getLSBs(x):
    if x < 256:
        return x
    else:
        return x & 255

def getMSBs(x):
    if x < 256:
        return 0
    else:
        return (x >> 8)

def getLSBs2(x):
    return x & 255

def getMSBs2(x):
    return (x >> 8)

def makeTraceDataCommand(channel, size):
    return ("TRACE:DATA EMEM" + str(channel) + ",#"
    + str(len(str(size))) + str(size))

def valueToBytesString(value) :
    return (chr(getMSBs(value)) + chr(getLSBs(value)))

def valueToBytesString2(value) :
    return (chr(getMSBs2(value)) + chr(getLSBs2(value)))

data=[]

num_points=30000

tStart = timeit.default_timer()

for x in range(0,num_points) :
    v = valueToIntWord(0.4*( math.sin(2.5*x/120)) )
    data.append(chr(getMSBs(v)))
    data.append(chr(getLSBs(v)))

tEnd = timeit.default_timer()

binblob=("".join(data))

command="TRACE:DATA EMEM1,#46000"+binblob

print("Time: " + str(tEnd-tStart))

print(makeTraceDataCommand(1, 2*len(data)))

data=[]

tStart = timeit.default_timer()

for x in range(0,num_points) :
    data.append((0.4*( math.sin(2.5*x/120))))

tEnd = timeit.default_timer()
print("Time to make list: " + str(tEnd-tStart))

binblob2=("".join([valueToBytesString2(valueToIntWord2(e)) for e in data]))
#binblob2=("".join([(chr(getMSBs(e)) + chr(getLSBs(e))) for e in data]))

command=makeTraceDataCommand(1, 2*len(data)) + binblob2

#binblob2=""
#for e in data:
#    binblob2 += valueToBytesString(e)

tEnd = timeit.default_timer()
print("Time: " + str(tEnd-tStart))

print(binblob==binblob2)

##print (valueToIntWord2(-0.3))
##print (valueToIntWord3(-0.3))
##
##print (valueToIntWord2(-1+0.0002*2000))
##print (valueToIntWord3(-1+0.0002*2000))

tStart = timeit.default_timer()

my_instrument.write_raw(command)
tEnd = timeit.default_timer()
print("Write time: " + str(tEnd-tStart))

my_instrument.write('FUNCTION EMEM1')
my_instrument.write("MMEM:STOR:TRAC EMEM1, \"M:/test13.tfwx\"")

##my_instrument.write_raw("TRACE:DATA EMEM1,#46000"+("".join(data)))
##my_instrument.write('FUNCTION EMEM1')
##my_instrument.write('OUTPUT1 ON')
##s=my_instrument.write("MMEM:STOR:TRAC EMEM1, \"M:/test13.tfwx\"")
##print(s)
##s=my_instrument.write("WLIST:WAV:IMP \"M:/test13.tfwx\"")
##s=my_instrument.write("SEQ:LENG 1")
##s=my_instrument.write("SEQ:ELEM1:WAV1 \"M:/test13.tfwx\"")  #SEQ:ELEM6:WAV1



### Triangle wave read from memory
##s=my_instrument.write("MMEM:LOAD:TRAC EMEM2, \"M:/Wave5.tfwx\"")
##s=my_instrument.query_binary_values('TRACE:DATA? EMEM2',datatype='c')
##my_instrument.write_raw("TRACE:DATA EMEM1,#3200"+("".join(s)))
##my_instrument.write("MMEM:STOR:TRAC EMEM1, \"M:/test12.tfwx\"")
##s=my_instrument.query_binary_values('TRACE:DATA? EMEM1',datatype='c')


my_instrument.close()
