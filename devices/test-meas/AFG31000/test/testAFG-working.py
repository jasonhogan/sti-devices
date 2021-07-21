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

data=[]

#for x in range(0,100) :
#    data.append((int(math.floor(10*math.sin(6*x/50.0)+100))))

num_points=3000
base=9000
offset=7000

for x in range(0,num_points) :
    data.append((int(math.floor(base + offset*math.sin(0.20*x/120))/256)))

print(str(data))

#s = my_instrument.write_ascii_values("TRACE:DATA EMEM1, #3100", data)
s = my_instrument.write_binary_values("TRACE:DATA EMEM1,#43000,", data,datatype='b')
#s = my_instrument.write_ascii_values("TRACE:DATA EMEM1 ", (data))
print(s)
my_instrument.write('FUNCTION EMEM1')
my_instrument.write('FREQUENCY 9K')
my_instrument.write('OUTPUT2 ON')
#print(s)

#s=my_instrument.write_binary_values('WLISt:WAVeform:DATA somename,', data)
s=my_instrument.write("MMEM:STOR:TRAC EMEM1, \"M:/test11.tfwx\"")
print(s)

my_instrument.close()
