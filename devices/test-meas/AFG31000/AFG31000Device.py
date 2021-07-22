import STIPy
import AFG31000


class AFG31000Device(STIPy.STI_Device):
    def __init__(self, orb, name, ip, module):
        STIPy.STI_Device.__init__(self, orb, name, ip, module)
        self.afg = AFG31000.AFG31000('TCPIP0::192.168.1.3::inst0::INSTR')
        return
    def defineChannels(self):
        # Called during device initialization
        self.addOutputChannel(1, STIPy.TValue.ValueVector, "")
        self.addOutputChannel(2, STIPy.TValue.ValueVector, "")


orb = STIPy.ORBManager()

afgDevice = AFG31000Device(orb, "AFG31000", "SrInterferometer", 0)

orb.run()
