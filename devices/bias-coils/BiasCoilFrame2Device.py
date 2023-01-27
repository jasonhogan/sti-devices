import STIPy
import visa
from N6700BDevice import *
# Agilent N6700B device

rm = visa.ResourceManager()
N6700Bvisa = rm.open_resource('TCPIP0::A-N6700B-20887::INSTR')    # Currently this does not work as a reference.  Reason unknown.

orb = STIPy.ORBManager()

biasCoilsFrame2 = N6700B(orb, "N6700B_biasCoils_fr2", "sr-gradiometer", 0,N6700Bvisa)

orb.run()
N6700Bvisa.close()
