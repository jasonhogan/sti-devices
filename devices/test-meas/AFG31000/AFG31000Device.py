import STIPy
import AFG31000
import math


class AFG31000Device(STIPy.STI_Device):
    def __init__(self, orb, name, ip, module):
        STIPy.STI_Device.__init__(self, orb, name, ip, module)
        self.afg = AFG31000.AFG31000('TCPIP0::192.168.1.3::inst0::INSTR')

        self.sampleRate = 1000   # MS/s
        self.fc = 50             # MHz
        self.units = 1e-3        # MHz * ns

        self.amplitude = [1, 1]       #Vpp [ch1, ch2]

        return
    
    def defineChannels(self):
        # Called during device initialization
        self.addOutputChannel(1, STIPy.TValue.ValueVector, "")
        self.addOutputChannel(2, STIPy.TValue.ValueVector, "")
    def defineAttributes(self):
        self.addAttribute("Sample Rate (MS/s)", self.sampleRate)
        self.addAttribute("Carrier Frequency (MHz)", self.fc)
        self.addAttribute("Amplitude 1 (Vpp)", self.amplitude[0])
        self.addAttribute("Amplitude 2 (Vpp)", self.amplitude[1])
        return
    
    def updateAttribute(self, key, value):

        if(key == "Sample Rate (MS/s)"):
            return self.setSampleRate(value)
        elif(key == "Carrier Frequency (MHz)"):
            return self.setCarrierFrequency(value)
        elif(key == "Amplitude 1 (Vpp)"):
            return self.setAmplitude(1, value)
        elif(key == "Amplitude 2 (Vpp)"):
            return self.setAmplitude(2, value)
        return False
    
    def refreshAttributes(self):
        self.setAttribute("Sample Rate (MS/s)", self.sampleRate)
        self.setAttribute("Carrier Frequency (MHz)", self.fc)

    def parseDeviceEvents(self, eventsIn, eventsOut):
        # This function is called during parsing of a timing file.
        # eventsIn: the list of raw events to this device from the server
        # eventsOut: the derived list of device-specific synchronous events 
        # This function must convert eventsIn->eventsOut by inspecting each event
        # from the server and using the data to generate the appropriate synchronous events.
        # This function also must check that the incoming eventsIn are valid and physically possible.

        dt = (1000.0/self.sampleRate)    #ns

        self.pulseDataCh1 = []
        self.pulseDataCh2 = []
        self.pulseData=[self.pulseDataCh1, self.pulseDataCh2]

        triggerTime = 0
        lastPulseOffTimes = [0,0]   #last time each pulse turned off, in integers steps of dt
        
        firstEvent = True
        lastPulseDuration = 0


        for nextEventsTuple in eventsIn:
            # nextEventsTuple is formated as (eventTime, {event list})

            if firstEvent:
                triggerTime = nextEventsTuple[0]
                firstEvent = False

            for evt in nextEventsTuple[1]: #iterate over channels

                self.checkValueFormat(evt.value())
                [pulseDuration, pulseSpectrum] = evt.value()

                rawPulseTime = nextEventsTuple[0]
                absPulseTime = int( (rawPulseTime - triggerTime) / dt )   #desired dt steps since trigger for this pulse
                
                nextPulseDelay = absPulseTime - lastPulseOffTimes[evt.channel()-1]

                if nextPulseDelay < 0:
                    raise ValueError('Pulse spacing too small.')

                
                #pulseDataCh1 += padding
                self.pulseData[evt.channel() - 1] += self.makeZeroPadding(nextPulseDelay)
                self.pulseData[evt.channel() - 1] += self.makePulse(pulseDuration, pulseSpectrum)


            lastPulseOffTimes = [ len(self.pulseData[0]), len(self.pulseData[1])]




##            # Determine amount of zero padding needed before pulse (since last pulse)
##            if firstEvent:
##                tFrontPad = 0
##            else:
##                pulseTime = key[0]
##                tFrontPad = (pulseTime - lastEventTime) - lastPulseDuration
##
##            if tFrontPad >= 0 :
##                padding = self.makeZeroPadding(tFrontPad)
##            elif tFrontPad < 0:
##                raise ValueError('Pulse spacing too small.')

            
            #rawEvent=key[1][0]  #for multiple events at the same time, iterate over events with key[1][j]

            #self.checkValueFormat(rawEvent.value())

            #print(rawEvent.value())

            #raise STIPy.EventParsingException(rawEvent, "test exception")
            #raise ValueError('Represents a hidden bug, do not catch this')

            #evt = AFG3100Event(key[0], rawEvent.channel(), rawEvent.value(), self)   # Using device-specific custom event class, defined below
            #eventsOut.append(evt)

            #lastEventTime=key[0]

        #maxPoints = max([len(self.pulseDataCh1), len(self.pulseDataCh2)])  
        maxPoints = max(len(self.pulseDataCh1), len(self.pulseDataCh2), 168)    # min waveform length is 168
        print("maxPoints = " + str(maxPoints))

        print("len(pulseDataCh1) = " + str(len(self.pulseDataCh1)))
        print("len(pulseDataCh2) = " + str(len(self.pulseDataCh2)))

        # makes list lengths equal
        self.pulseDataCh1 += [0] * (maxPoints - len(self.pulseDataCh1) )
        self.pulseDataCh2 += [0] * (maxPoints - len(self.pulseDataCh2) )

        print("*len(pulseDataCh1) = " + str(len(self.pulseDataCh1)))
        print("*len(pulseDataCh2) = " + str(len(self.pulseDataCh2)))

        afgEvt = AFG3100Event(0, self)  #initialization event
        eventsOut.append(afgEvt)

        #print(pulseDataCh1)
        #print(pulseDataCh2)

        return

    def checkValueFormat(self, value):
        
        if (type(value)==tuple or type(value)==list) and len(value) == 2:
            #print(value[1])
            spectrumTest=[(type(e)==tuple or type(e)==list) and (len(e) > 1 and len(e) <= 3) for e in value[1]]
            #print(spectrumTest)
            if all(spectrumTest) == False:
                badIndices = [i for i,x in enumerate(spectrumTest) if x == False]
                for b in badIndices:
                    print(value[1][b])
                raise ValueError("Pulse event specturm must be a list with elements of the form (frequency, amplitude, [phase]). ")
            return
        else:
            raise ValueError("Pulse event value must be a list of the form (duration, spectrum)")

    def makePulse(self, duration, spectrum):
        dt = (1000/self.sampleRate)    #ns

        nPoints = int(self.sampleRate * duration * self.units)
        print("nPoints = " + str(nPoints))
        
        data=[]

        def makeSinPoint(t, freq, amplitude, phi=0):
            return amplitude * math.sin( 2*math.pi*(self.fc + freq)*t*dt*self.units + phi )
        
        for t in range(0, nPoints):
            sample = 0
            
            for s in spectrum:  #sum over each spectal componet s of the form (freq, amp, [phase])
                sample += makeSinPoint(t, *s)   #unpack spectral component tuple s
            
            data.append(sample)
        
        return data
        
    def makeZeroPadding(self, padSteps):
        return [0]*padSteps

    def setSampleRate(self, value):
        rate = float(value)

        if (rate <= 2000 and rate > 0):
            self.sampleRate = rate
            self.afg.setSamplingRate(self.sampleRate)
            return True
        return False

    def setCarrierFrequency(self, value):
        fc = float(value)

        if (fc > 0):
            self.fc = fc
            return True
        return False
    
    def setAmplitude(self, channel, value):
        amp = float(value)

        if (amp > 0):
            self.amplitude[channel-1] = amp
            self.afg.setAmplitude(channel, self.amplitude[channel-1])
            return True
        return False
    
    def setupAFG(self):
        afg = self.afg

        #reset
        afg.write_visa("SEQControl:STOP")
        #afg.write_visa("SEQControl:STAT OFF") # Exit sequence mode
        #afg.enableChannel(1, False)
        #afg.enableChannel(2, False)
        #afg.setAmplitude(1, 0.25)

        # New sequence
        afg.clearSequence()
        afg.setSequenceLength(1)
        #afg.setSamplingRate(self.sampleRate)

        #print(self.pulseData[0])
        print(len(self.pulseData[0]))

        

        afg.addToSequence(1, 1, self.pulseData[0])
        afg.addToSequence(1, 2, self.pulseData[1])

        # Loop infinite
        #afg.write_visa("SEQ:ELEM1:LOOP:INF 1")
        #afg.query_visa("SEQ:ELEM1:LOOP:INF?")

        # Set goto
        afg.write_visa("SEQ:ELEM1:GOTO:STAT ON")
        afg.write_visa("SEQ:ELEM1:MARK:STAT 1")

        # Enable Wait and set trigger (manual)
        afg.write_visa("SEQ:ELEM1:TWA:STAT 1")
        afg.write_visa("SEQ:ELEM1:TWA:EVEN MAN")
        #afg.enableExtTrigger(1, True)

        # Enter sequence mode
        #afg.write_visa("SEQControl:STAT ON")

        afg.write_visa("SEQControl:RUN")
        #afg.enableChannel(1, True)
        #afg.enableChannel(2, True)



class AFG3100Event(STIPy.SynchronousEvent):
    def __init__(self, time, device):
        STIPy.SynchronousEvent.__init__(self, time, device)
        self.device=device

    def loadEvent(self):
        self.device.setupAFG()
        return

    def playEvent(self):
        return


        

orb = STIPy.ORBManager()

afgDevice = AFG31000Device(orb, "AFG31000", "SrInterferometer", 0)

orb.run()
