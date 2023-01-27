import STIPy
import visa
# Agilent N6700B device

class N6700B(STIPy.STI_Device):
    def __init__(self, orb, name, ip, module,visa):
        STIPy.STI_Device.__init__(self, orb, name, ip, module)
        self.visa = visa
        return
	
    def defineChannels(self):
        # Called during device initialization
        self.addOutputChannel(1, STIPy.TValue.ValueNumber, "X voltage (set)")
        self.addInputChannel(11, STIPy.TData.DataDouble, STIPy.TValue.ValueNumber)
        self.addOutputChannel(2, STIPy.TValue.ValueNumber, "Y voltage (set)")
        self.addInputChannel(12, STIPy.TData.DataDouble, STIPy.TValue.ValueNumber)
        self.addOutputChannel(3, STIPy.TValue.ValueNumber, "Z voltage (set)")
        self.addInputChannel(13, STIPy.TData.DataDouble, STIPy.TValue.ValueNumber)
        self.addOutputChannel(4, STIPy.TValue.ValueNumber, "X current (set)")
        self.addInputChannel(14, STIPy.TData.DataDouble, STIPy.TValue.ValueNumber)
        self.addOutputChannel(5, STIPy.TValue.ValueNumber, "Y current (set)")
        self.addInputChannel(15, STIPy.TData.DataDouble, STIPy.TValue.ValueNumber)
        self.addOutputChannel(6, STIPy.TValue.ValueNumber, "Z current (set)")
        self.addInputChannel(16, STIPy.TData.DataDouble, STIPy.TValue.ValueNumber)
        return
    
    def setVoltage(self, V, ch):
        message = 'VOLT ' + str(V) + ', (@' + str(ch) + ')'
        self.visa.write(message)

    def readVoltage(self, ch):
        message = 'MEAS:VOLT? (@' + str(ch) + ')'
        s = self.visa.query(message)
        return s

    def setCurrent(self, A, ch):
        message = 'CURR ' + str(A) + ', (@' + str(ch) + ')'
        self.visa.write(message)

    def readCurrent(self, ch):
        message = 'MEAS:CURR? (@' + str(ch) + ')'
        s = self.visa.query(message)
        return s

    
    def readChannel(self, channel, valueIn, dataOut):
        if (channel > 10 and channel <= 13):
            s = self.readVoltage(channel-10)
        elif (channel > 13 and channel <= 16):
            s = self.readCurrent(channel-13)
        else:
            return False
        dataOut.setValue(float(s))
        return True
    
    def writeChannel(self, channel, value):
        if (channel <= 3):
            self.setVoltage(value,channel)
        elif (channel > 3 and channel <= 6):
            self.setCurrent(value,channel-3)
        else:
            return False
        return True

    def parseDeviceEvents(self, eventsIn, eventsOut):
        return self.parseDeviceEventsDefault(eventsIn, eventsOut)
