import pyvisa
import math

rm = pyvisa.ResourceManager()
my_instrument=rm.open_resource('TCPIP0::192.168.1.3::inst0::INSTR')

##message = 'MEAS:VOLT? (@' + str(ch) + ')'
##print(message)
##s = my_instrument.query(message)
##
##message = 'CURR ' + str(A) + ', (@' + str(ch) + ')'
##print(message)
##my_instrument.write(message)

message = '*IDN?'
s = my_instrument.query(message)
print(s)

#data=[45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0,45.0]
data=[0.0,0.0]
#for x in range(0,100) :
#    data.append((int(math.floor(10*math.sin(6*x/50.0)+100))))

num_points=3000
base=9000
offset=3000

for x in range(0,num_points) :
    data.append((((base + offset*math.sin(0.23*x/120))/256)))
#    data.append((int(math.floor(base + offset*math.sin(0.90*x/120))/256)))

#print(str(data))

#s = my_instrument.write_ascii_values("TRACE:DATA EMEM1, #3100", data)
s = my_instrument.write_binary_values("TRACE:DATA EMEM1,#43000", data, datatype='f',encoding='ascii',is_big_endian=False) #ascii
#s = my_instrument.write_ascii_values("TRACE:DATA EMEM1 ", (data))
#my_instrument.write_raw(

print(s)
#s=my_instrument.query('TRACE:DATA? EMEM1')
#print(s)
#print( map(lambda x: ord(x), list(str(s))) )

my_instrument.write('FUNCTION EMEM1')
my_instrument.write('FREQUENCY 8K')
my_instrument.write('OUTPUT1 ON')
#print(s)

#s=my_instrument.write_binary_values('WLISt:WAVeform:DATA somename,', data)
s=my_instrument.write("MMEM:STOR:TRAC EMEM1, \"M:/test12.tfwx\"")
print(s)
s=my_instrument.write("WLIST:WAV:IMP \"M:/test12.tfwx\"")
s=my_instrument.write("SEQ:LENG 6")
s=my_instrument.write("SEQ:ELEM6:WAV1 \"M:/test12.tfwx\"")

s=my_instrument.write("MMEM:LOAD:TRAC EMEM2, \"M:/Wave5.tfwx\"")
s=my_instrument.query_binary_values('TRACE:DATA? EMEM2',datatype='c')
s2=[]
for i in range(0,200,2):
    s2.append(s[i+1])
    s2.append(s[i])
print("s2: ")
print([hex(ord(x)) for x in s2])
#my_instrument.write_binary_values("TRACE:DATA EMEM1,#3206      ????", s2, datatype='c',encoding='ascii',is_big_endian=False, termination=None)
#print(s2)
#print("".join(s2))
my_instrument.write_raw("TRACE:DATA EMEM1,#3200"+("".join(s)))
#my_instrument.write_raw("TRACE:DATA EMEM1,#3200", s2, datatype='c',encoding='ascii',is_big_endian=False)
my_instrument.write("MMEM:STOR:TRAC EMEM1, \"M:/test12.tfwx\"")
#s=my_instrument.query_binary_values('TRACE:DATA? EMEM1',datatype='f')
s=my_instrument.query_binary_values('TRACE:DATA? EMEM1',datatype='c')
#print(s)
#print([hex(ord(x)) for x in s])

#print(map(lambda x: hex(x), s))
#print( map(lambda x: ord(x), list(str(s))) )

my_instrument.close()
