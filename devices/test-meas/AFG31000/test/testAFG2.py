import pyvisa
import math

rm = pyvisa.ResourceManager()
my_instrument=rm.open_resource('TCPIP0::192.168.1.3::inst0::INSTR')

message = '*IDN?'
s = my_instrument.query(message)

def valueToChar(x):
    if x > 1:
        return (2**14 - 1)
    elif x < -1:
        return 0
    else:
        return int((2**14 - 1)*((x + 1)/2.0))

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

data=[]

num_points=3000

for x in range(0,num_points) :
    v = valueToChar(0.4*( math.sin(2.5*x/120)) )
    data.append(chr(getMSBs(v)))
    data.append(chr(getLSBs(v)))

#print(str(data))
#print(("".join(data)))



my_instrument.write_raw("TRACE:DATA EMEM1,#46000"+("".join(data)))
my_instrument.write('FUNCTION EMEM1')
my_instrument.write('OUTPUT1 ON')
s=my_instrument.write("MMEM:STOR:TRAC EMEM1, \"M:/test13.tfwx\"")
print(s)
s=my_instrument.write("WLIST:WAV:IMP \"M:/test13.tfwx\"")
s=my_instrument.write("SEQ:LENG 1")
s=my_instrument.write("SEQ:ELEM1:WAV1 \"M:/test13.tfwx\"")  #SEQ:ELEM6:WAV1



### Triangle wave read from memory
##s=my_instrument.write("MMEM:LOAD:TRAC EMEM2, \"M:/Wave5.tfwx\"")
##s=my_instrument.query_binary_values('TRACE:DATA? EMEM2',datatype='c')
##my_instrument.write_raw("TRACE:DATA EMEM1,#3200"+("".join(s)))
##my_instrument.write("MMEM:STOR:TRAC EMEM1, \"M:/test12.tfwx\"")
##s=my_instrument.query_binary_values('TRACE:DATA? EMEM1',datatype='c')


my_instrument.close()
