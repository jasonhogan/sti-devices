import STIPy
import AFG31000
import math


class AFG31000Device(STIPy.STI_Device):
    def __init__(self, orb, name, ip, module):
        STIPy.STI_Device.__init__(self, orb, name, ip, module)
        self.afg = AFG31000.AFG31000('TCPIP0::192.168.1.3::inst0::INSTR')

        self.sampleRate = self.afg.getSamplingRate()   # MS/s
        self.fc = 190             # MHz
        self.units = 1e-3        # MHz * ns
        self.nonlinearCorrection = False
        self.nonlinearCoefficient = 0
        #self.amplitude = [self.afg.getAmplitude(1), self.afg.getAmplitude(2)]       #Vpp [ch1, ch2]

        return
    
    def defineChannels(self):
        # Called during device initialization
        self.addOutputChannel(1, STIPy.TValue.ValueVector, "")
        self.addOutputChannel(2, STIPy.TValue.ValueVector, "")
    def defineAttributes(self):
        self.addAttribute("Sample Rate (MS/s)", self.sampleRate)
        self.addAttribute("Carrier Frequency (MHz)", self.fc)
        self.addAttribute("Amplitude 1 (Vpp)", self.afg.getAmplitude(1))
        self.addAttribute("Amplitude 2 (Vpp)", self.afg.getAmplitude(2))
        self.addAttribute("Enable nonlinear correction", "False", "True, False")
        self.addAttribute("Nonlinear coefficient", 0)
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
        elif(key == "Enable nonlinear correction"):
            self.nonlinearCorrection = True if (value == "True") else False
            return True
        elif(key == "Nonlinear coefficient"):
            self.nonlinearCoefficient = float(value)
            return True
        return False
    
    def refreshAttributes(self):
        self.setSampleRate( self.afg.getSamplingRate() )    #query AFG for current value
        self.setAttribute("Sample Rate (MS/s)", self.sampleRate)
        
        self.setAttribute("Carrier Frequency (MHz)", self.fc)

        self.setAmplitude(1, self.afg.getAmplitude(1))
        self.setAttribute("Amplitude 1 (Vpp)", self.afg.getAmplitude(1))

        self.setAmplitude(2, self.afg.getAmplitude(2))
        self.setAttribute("Amplitude 2 (Vpp)", self.afg.getAmplitude(2))

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

                if (pulseDuration == "Continuous"):
                    self.runContinuous = True
                    freq = (self.fc + pulseSpectrum[0][0]) * self.units

                    pulseDuration = (1 / freq)
                    print("pulseDuration = " + str(pulseDuration))

                    # Make sure pulse has at least 168 points
                    while ((pulseDuration / dt) < 168.0):
                        pulseDuration += (1 / freq)

                    print("pulseDuration = " + str(pulseDuration))
                    
                else:
                    self.runContinuous = False

                rawPulseTime = nextEventsTuple[0]
                absPulseTime = int( (rawPulseTime - triggerTime) / dt )   #desired dt steps since trigger for this pulse
                
                nextPulseDelay = absPulseTime - lastPulseOffTimes[evt.channel()-1]

                if nextPulseDelay < 0:
                    raise ValueError('Pulse spacing too small.')

                self.pulseData[evt.channel() - 1] += self.makeZeroPadding(nextPulseDelay)
                self.pulseData[evt.channel() - 1] += self.makePulse(pulseDuration, pulseSpectrum)


            lastPulseOffTimes = [ len(self.pulseData[0]), len(self.pulseData[1])]
            

        maxPoints = max(len(self.pulseDataCh1), len(self.pulseDataCh2), 168)    # min waveform length is 168

        #print(maxPoints)
        # makes list lengths equal by padding with zeros at the end
        self.pulseDataCh1 += [0] * (maxPoints - len(self.pulseDataCh1) )
        self.pulseDataCh2 += [0] * (maxPoints - len(self.pulseDataCh2) )

        afgEvt = AFG3100Event(0, self)  #initialization event for Advanced Mode
        #afgEvt = AFG3100BurstEvent(0, self, dt*maxPoints)  #initialization event for Basic Mode
        eventsOut.append(afgEvt)

        #self.setupAFG()

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
        #print("nPoints = " + str(nPoints))
        
        data=[]

        def makeSinPoint(t, freq, amplitude, phi=0):
            return amplitude * math.sin( 2*math.pi*(self.fc + freq)*t*dt*self.units + (phi * math.pi/180) )

        # Correct for nonlinear phase shift (intensity dependent phase)
        def makeSinPointNLCorrection(t, delta, phiA, phiB, freq, amplitude, phi=0):
            # Cos[delta*t + 0.5*(phiA - phiB)]
            phiNL = self.nonlinearCoefficient * math.cos( 2*math.pi*(delta)*t*dt*self.units + ((phiA * math.pi/180) - (phiB * math.pi/180))/2 )
            print("phiNL = " + str(phiNL))
            return amplitude * math.sin( 2*math.pi*(self.fc + freq)*t*dt*self.units + (phi * math.pi/180) + (phiNL * math.pi/180) )

        applyNLcorrection = False
        if (self.nonlinearCorrection and len(spectrum) == 2):
            print("Nonlinear enabled")
            applyNLcorrection = True
            delta = spectrum[0][0]
            if (len(spectrum[0]) == 3):
                phiA = spectrum[0][2]
            else:
                phiA=0
            if (len(spectrum[1]) == 3):
                phiB = spectrum[1][2]
            else:
                phiB=0
        else:
            delta = 0
            phiA = 0
            phiB = 0

        for t in range(0, nPoints):
            sample = 0
            
            for s in spectrum:  #sum over each spectal componet s of the form (freq, amp, [phase])
                if applyNLcorrection:
                    print("applying nonlinear")
                    sample += makeSinPointNLCorrection(t, delta, phiA, phiB, *s)    #Use the beat between two spectral components to determine the amplitude
                else:
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

        if (fc >= 0):
            self.fc = fc
            return True
        return False
    
    def setAmplitude(self, channel, value):
        amp = float(value)

        if (amp > 0):
            #self.amplitude[channel-1] = amp
            self.afg.setAmplitude(channel, amp)
            return True
        return False

    def setBasicChannel(self, channel, burstFreq):

        afg = self.afg

        #burstFreq = value
        afg.write_visa("SOUR" + str(channel) + ":FUNC:SHAP EMEM" + str(channel))
        afg.write_visa("SOUR" + str(channel) + ":FREQ:CW " + str(burstFreq) + "MHz")
        afg.write_visa("SOUR" + str(channel) + ":BURS:STAT ON")
        afg.write_visa("SOUR" + str(channel) + ":BURS:NCYC 1")

        return True
    
    def setupAFG(self):
        afg = self.afg

        #reset
        #afg.write_visa("SEQControl:STOP")
        #afg.write_visa("SEQControl:STAT OFF") # Exit sequence mode
        #afg.enableChannel(1, False)
        #afg.enableChannel(2, False)
        #afg.setAmplitude(1, 0.25)


        sequenceLength = len(self.pulseData[0])
        rowLength = 100000
        minLength = 168
        numRows = int(math.ceil(sequenceLength/float(rowLength)))

        # New sequence
        afg.clearSequence()
        afg.setSequenceLength(numRows)
        #afg.setSamplingRate(self.sampleRate)

        #print(self.pulseData[0])
        #print(len(self.pulseData[0]))

        pos = 0
        index = 1

        while (pos <= sequenceLength):
            remainder = sequenceLength - pos
            
            if (remainder < minLength):
                extraZero = self.makeZeroPadding(minLength - remainder)
                self.pulseData[0] += extraZero
                self.pulseData[1] += extraZero
                sequenceLength += minLength - remainder

            afg.addToSequence(index, 1, self.pulseData[0][pos : min(pos + rowLength, sequenceLength)])        
            afg.addToSequence(index, 2, self.pulseData[1][pos : min(pos + rowLength, sequenceLength)])
            pos += rowLength
            index += 1
            #time.sleep(1)


        #afg.addBothToSequence(1, self.pulseData[0], self.pulseData[1])

        # Loop infinite
        if self.runContinuous:
            afg.write_visa("SEQ:ELEM1:LOOP:INF 1")
            afg.query_visa("SEQ:ELEM1:LOOP:INF?")

        # Set goto
        ##afg.write_visa("SEQ:ELEM1:GOTO:STAT ON")
        ##afg.write_visa("SEQ:ELEM1:MARK:STAT 1")

        # Enable Wait and set trigger (external)
        afg.write_visa("SEQ:ELEM1:TWA:STAT 1")
        afg.write_visa("SEQ:ELEM1:TWA:EVEN EXT")
        ##afg.enableExtTrigger(1, True)

        # Enter sequence mode
        afg.write_visa("SEQControl:STAT OFF") # Exit sequence mode
        afg.write_visa("SEQControl:STAT ON")

        #afg.write_visa("SEQControl:RMOD TRIG")

        #afg.write_visa("SEQControl:RMOD SEQ")

        #afg.write_visa("SEQControl:RUN")
        afg.enableChannel(1, True)
        afg.enableChannel(2, True)

    def setupAFGBasic(self, duration):
        afg = self.afg

        afg.write(1, self.pulseData[0])
        afg.write(2, self.pulseData[1])


        burstFreq = 1000.0/duration # in MHz to give to 
        print(str(burstFreq) + "MHz")    

        self.setBasicChannel(1, burstFreq)
        self.setBasicChannel(2, burstFreq)

        afg.write_visa("TRIG:SOUR EXT")

        afg.enableChannel(1, True)
        afg.enableChannel(2, True)


class AFG3100Event(STIPy.SynchronousEvent):
    def __init__(self, time, device):
        STIPy.SynchronousEvent.__init__(self, time, device)
        self.device=device

    def loadEvent(self):
        self.device.setupAFG()
        return

    def playEvent(self):
        self.device.afg.write_visa("SEQControl:RUN")
        return

class AFG3100BurstEvent(STIPy.SynchronousEvent):
    def __init__(self, time, device, burstDuration):
        STIPy.SynchronousEvent.__init__(self, time, device)
        self.device=device
        self.duration=burstDuration

    def loadEvent(self):
        self.device.setupAFGBasic(self.duration)
        return

    def playEvent(self):
        return

        

orb = STIPy.ORBManager()

afgDevice = AFG31000Device(orb, "AFG31000", "SrInterferometer", 0)

orb.run()
