from AFG31000 import *


##Test

afg = AFG31000('TCPIP0::192.168.1.3::inst0::INSTR')

#MS/s
samplingRate = 500

deltaT_ns = 1000.0/samplingRate

num_points=0*2*500+168
fc=40

freq=50  #in MHz
delta=2
data1=[]
data2=[]
zeros=[]
for x in range(0,num_points) :

    #data1.append((0.5*math.sin(math.pi*(fc-delta)*x/500) - 1*0.5*math.sin(math.pi*(fc+delta)*x/500)))
    #data1.append((0.5*math.cos(2*math.pi*(freq*1e-3)*x*deltaT_ns ) ))
    data1.append((0.25*math.sin(2*math.pi*((freq-delta)*1e-3)*x*deltaT_ns) - (0.25*math.sin(2*math.pi*((freq+delta)*1e-3)*x*deltaT_ns) )))

    zeros.append(0)

#event(ch(afgdev, 1), 10*ms, (delta, W1, W2, W3, W4) )
#event(ch(afgdev, 1), 10*ms+10*ns, ((d1, W1), (d2, W2, phi2)) )
#event(ch(afgdev, 1), 10*ms+10*ns+200*ns, ((d1, 0) ) )
#event(ch(afgdev, 1), 10*ms+10*ns, (dt, ((d1, W1), (d2, W2)) ) )


#for t in range(20):
#    for e in evt.value:
#        data[t] += e[1]*sin((e[0]+50)*t)

##file1 = open("MyFile.txt","a")
##for e in data1:
##    file1.write(str(e)+"\n")
##
##file1.close()

#print(len(data1))
#print(len(zeros))

#afg.write(2, data)


afg.write_visa("SEQControl:STOP")
afg.write_visa("SEQControl:STAT OFF") # Exit sequence mode
afg.enableChannel(1, False)
afg.enableChannel(2, False)
afg.setAmplitude(1, 0.25)


afg.clearSequence()
afg.setSequenceLength(1)
afg.setSamplingRate(samplingRate)


afg.addToSequence(1, 1, data1)
afg.addToSequence(1, 2, data1)

#afg.enableExtTrigger(1, False)

#print(afg.write_visa("SEQ:ELEM1:JUMP:EVEN MAN"))
#print(afg.write_visa("SEQ:ELEM1:JTAR:TYPE NEXT"))
#print(afg.query_visa("SEQ:ELEM1:JTAR:TYPE?"))

# Loop infinite
afg.write_visa("SEQ:ELEM1:LOOP:INF 1")
afg.query_visa("SEQ:ELEM1:LOOP:INF?")

# Set goto
afg.write_visa("SEQ:ELEM1:GOTO:STAT ON")
afg.write_visa("SEQ:ELEM1:MARK:STAT 1")
#print(afg.query_visa("SEQ:ELEM1:GOTO:STAT?"))


# Enable Wait and set trigger (manual)
afg.write_visa("SEQ:ELEM1:TWA:STAT 1")
#print(afg.query_visa("SEQ:ELEM1:TWA:STAT?"))
afg.write_visa("SEQ:ELEM1:TWA:EVEN MAN")
#print(afg.query_visa("SEQ:ELEM1:TWA:EVEN?"))



#afg.enableExtTrigger(1,True)


# Enter sequence mode
afg.write_visa("SEQControl:STAT ON")


afg.write_visa("SEQControl:RUN")
afg.enableChannel(1, True)
afg.enableChannel(2, True)

#afg.write_visa("SEQControl:RUN")

#afg.write_visa("*TRG")

#afg.write_visa("TRIG:SEQ:IMM")

#afg.write_visa("SEQControl:STAT ON")
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



