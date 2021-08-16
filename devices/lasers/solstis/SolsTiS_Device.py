import STIPy
import socket
import json

# SolsTiS device

## Note: The IP address needs to be the address of the SolsTis.
### The ip_address sent in the startMessage must be the local IP, and must match one of the remote interfaces listed in the SolsTiS config.
### The TCP_PORT must match the port listed in the SolsTiS configuration (confusing: the "port" field in the config must be
### the port that the solistis will listen to, not any port associated with the remote interface.

#SolsTiS network setup
SolsTiS_IP = '192.168.1.222'
SolsTiS_TCP_PORT = 39933


class SolsTiS_Device(STIPy.STI_Device):
    def __init__(self, orb, name, ip, module):
        STIPy.STI_Device.__init__(self, orb, name, ip, module)

        #Get IP4 address from address name
        localAddress = (ip + '.') if ip.find('.') == -1 else ip
        self.IP4addr = socket.gethostbyname(localAddress)

        self.buffer_size = 1024

        # Initialize socket connection to SolsTiS
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((SolsTiS_IP, SolsTiS_TCP_PORT))

        # Send start_link message to laser with local IP
        startMess = str.encode('{\"message\":{ \"transmission_id\":[0],\"op\":\"start_link\",\"parameters\":{\"ip_address\":\"'
                               + self.IP4addr + '\"}}}')
        self.sendMessage(startMess)

        return

    def sendMessage(self, message) :
        
        self.sock.send(message)
        response = self.sock.recv(self.buffer_size)
        
        return response

    def getRawStatus(self) :
        statusMess = b'{\"message\":{ \"transmission_id\":[2],\"op\":\"get_status\"}}'
        return self.sendMessage(statusMess) #return full status string

    def getOutputPD(self) :
        rawStatus = self.getRawStatus()

        # parse JSON
        status = json.loads(rawStatus)

        return status["message"]["parameters"]["output_monitor"][0]
    
    def defineChannels(self):
        # Called during device initialization
        
        self.addInputChannel(0, STIPy.TData.DataDouble, STIPy.TValue.ValueNumber)

        return
    
    
    def readChannel(self, channel, valueIn, dataOut):

        if (channel == 0) :
            result = self.getOutputPD()
            #print(result)
            dataOut.setValue(float(result)) 
        
        return True
    
    def writeChannel(self, channel, value):
        return True

    def parseDeviceEvents(self, eventsIn, eventsOut):
        # This function is called during parsing of a timing file.
        # eventsIn: the list of raw events to this device from the server
        # eventsOut: the derived list of device-specific synchronous events 
        # This function must convert eventsIn->eventsOut by inspecting each event
        # from the server and using the data to generate the appropriate synchronous events.
        # This function also must check that the incoming eventsIn are valid and physically possible.

        #print(1/0)

        #return self.parseDeviceEventsDefault(eventsIn, eventsOut)


        for key in eventsIn:

            eventTime=key[0]
            rawEvent=key[1][0]  #for multiple events at the same time, iterate over events with key[1][j]


            evt = SolsTiSEvent(key[0], rawEvent.channel(), rawEvent.value(), self)
            evt.addMeasurement(rawEvent)
            eventsOut.append(evt)
          
		  
        return

class SolsTiSEvent(STIPy.SynchronousEvent):
    def __init__(self, time, channel, value, device):
        STIPy.SynchronousEvent.__init__(self, time, device)
        self.localtime=time
        self.localval=value
        self.locDev=device
        self.channel=channel
        return

    def playEvent(self):
        #self.data = 0.4
        #self.locDev.readChannel(self.channel, self.localval, self.data)
        self.data = float(self.locDev.getOutputPD())
        #print("play done")
        return
    
    def collectMeasurementData(self):
        #print("collectMeasurementData")
        try:
            m = self.getMeasurements()
            #print("success")
            #print("meas num: " + str(len(m)))
            #print(str(m[0]))
            #print("data: " + str(self.data))
            m[0].setData(self.data)
        except Exception as inst:
            print("Fail")
            print(type(inst))
            print(inst)
            print(inst.args)
        
        #print("length: " + len(self.eventMeasurements))
        
        #print(str(self.eventMeasurements))
        #self.eventMeasurements[0].setData(self.data)
        #print("done : " + self.data)
        #print("done")
        return

	
orb = STIPy.ORBManager()

solstisDevice = SolsTiS_Device(orb, "SolsTiS 689", "sr-optics", 0)

orb.run()

