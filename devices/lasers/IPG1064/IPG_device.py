import STIPy
import socket
import json

# IPG1064 device

## Note: The IP address needs to be the address of the laser.
### The ip_address sent in the startMessage must be the local IP, and must match that listed in the laser's network submenu.
### The TCP_PORT must be 10001, since this is the (apparently hard coded) port the laser listens on. 

# Network setup
#IPG1064_IP = '192.168.1.199'
IPG1064_IP = '192.168.1.198'
IPG1064_TCP_PORT = 10001


class IPG1064_Device(STIPy.STI_Device):
	def __init__(self, orb, name, ip, module):
		STIPy.STI_Device.__init__(self, orb, name, ip, module)

		#Get IP4 address from address name
		localAddress = (ip + '.') if ip.find('.') == -1 else ip
		self.IP4addr = socket.gethostbyname(localAddress)

		self.buffer_size = 1024

		# Initialize socket connection to laser
		# Actually, this times out too fast.  Instead, I'll open the socket each
                	# time I send a message and close it right afterwards.
		#self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		#self.sock.connect((IPG1064_IP, IPG1064_TCP_PORT))

		return

	def sendMessage(self, message) :
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.connect((IPG1064_IP, IPG1064_TCP_PORT))
		if ("\r" in message):
			self.sock.send(message.encode())
		else:
			self.sock.send((message + "\r").encode())
		response = self.sock.recv(self.buffer_size)
		self.sock.close()
		
		return response.decode()

	def getRawStatus(self) :
		return self.sendMessage("STA") #return full status string

	def getReadableStatus(self) :
		bitCode = int(self.getRawStatus()[5:-1])
		setBits = [i for i in range(32) if bitCode & 1 << i != 0]
		meanings = {	0 : "Command Buffer Overload",
						1 : "Overheat",
						2 : "Emission On",
						3 : "High Back Reflection Level",
						4 : "Analog Power Control Enabled",
						5 : "Pulse Too Long",
						6 : "",
						7 : "",
						8 : "Aiming Beam ON",
						9 : "Pulse too Short",
						10 : "Pulsed Mode",
						11 : "Power Supply OFF",
						12 : "Modulation Enabled",
						13 : "",
						14 : "",
						15 : "Emission is in the 3 second start-up state.",
						16 : "Gate Mode Enabled",
						17 : "High Pulse Energy",
						18 : "Hardware Emission Control Enabled",
						19 : "Power Supply Failure",
						20 : "Front Panel Display is Locked",
						21 : "Keyswitch is in REM position",
						22 : "Waveform Pulse Mode ON",
						23 : "Duty Cycle Too High",
						24 : "Low Temperature",
						25 : "Power Supply Alarm",
						26 : "",
						27 : "Hardware Aiming Beam Control Enabled",
						28 : "",
						29 : "Critical Error",
						30 : "Fiber Interlock Active",
						31 : "High Average Power"}
		return [meanings[i] for i in setBits]
	
	def getOutputCS(self) :
		# Returns output current setpoint as percent of maximum
		return float(self.sendMessage("RCS")[5:-1])

	def setOutputDC(self,p) : 
		# Sets output laser power as percent of maximum. 
		return self.sendMessage("SDC " +str(p))
	
	def enableEmission(self) : 
		return self.sendMessage("EMON")
	
	def disableEmission(self) : 
		return self.sendMessage("EMOFF")
	
	def enableAC(self) : 
		return self.sendMessage("EEC")
	
	def disableAC(self) :
		return self.sendMessage("DEC")
	
	def enableGate(self) : 
		return self.sendMessage("EGM")
	
	def disableGate(self) : 
		return self.sendMessage("DGM")
	
	def defineChannels(self):
		# Called during device initialization
		self.addOutputChannel(0, STIPy.TValue.ValueString, "IPG 1064 Command")
		self.addInputChannel(1, STIPy.TData.DataString, STIPy.TValue.ValueString)
		return
	
	def readChannel(self, channel, valueIn, dataOut):
		print(type(dataOut))
		print(dataOut)
		if (channel == 1) :
			result = self.getReadableStatus()
			statusString = ""
			for r in result:
				statusString += r + ", "
			statusString = statusString[0:-2]
			print(statusString)
			dataOut.setValue(statusString)
		return True
	
	def writeChannel(self, channel, value):
		if (channel == 0):
			print("Sending command to IPG 1064: '" + str(value) + "'")
			resp = self.sendMessage(value)
			print("Response: " + str(resp))
		return True

	def parseDeviceEvents(self, eventsIn, eventsOut):
		# return self.parseDeviceEventsDefault(eventsIn, eventsOut)
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
			evt = IPG1064Event(key[0], rawEvent.channel(), rawEvent.value(), self)
			if rawEvent.channel() == 1:
				evt.addMeasurement(rawEvent)
			eventsOut.append(evt)
		  
		  
		return

class IPG1064Event(STIPy.SynchronousEvent):
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
		#self.data = float(self.locDev.getOutputCS())
		#print("play done")
		print("playEvent")
		if self.channel == 0:
	    		response = self.locDev.sendMessage(self.localval)
		    	print("Played IPG 1064 event.  Response: " + str(response))
		if self.channel == 1:
			self.status = self.locDev.getReadableStatus()
		return
	
	def collectMeasurementData(self):
		#print("collectMeasurementData")
		if self.channel == 0:
			return 
		try:
			#tempData = STIPy.MixedData()
			print("collectMeasurementData")
			#self.locDev.readChannel(self.channel, self.localval, tempData)
			m = self.getMeasurements()
			#print("success")
			#print("meas num: " + str(len(m)))
			#print(str(m[0]))
			#print("data: " + str(self.data))
			m[0].setData(self.status)
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

#ipg1064Device = IPG1064_Device(orb, "IPG 1064 Optics Area", "sr-gradiometer", 0)
ipg1064Device = IPG1064_Device(orb, "IPG 1064 Frame 2", "sr-gradiometer", 0)

orb.run()

