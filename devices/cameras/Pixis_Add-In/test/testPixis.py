from stipy import *

ns = 1.0
us = 1000.0
ms = 1000000.0
s = 1000000000.0


time = 100*ms


pixisCamera = dev('Pixis AddIn','localhost2',0)
slow = dev('Slow Analog Out','ep-timing1.stanford.edu',4)

meas(ch(pixisCamera, 0), time+ 37*us, ("test",1))

event(ch(slow,3), time+100*ms, 3.3)
#meas(ch(pixisCamera, 0), time+ 11*s, ("test",2))

#meas(ch(pixisCamera, 0), time+ 15*s, ("test",2))
