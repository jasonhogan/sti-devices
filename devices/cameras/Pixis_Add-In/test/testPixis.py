from stipy import *

ns = 1.0
us = 1000.0
ms = 1000000.0
s = 1000000000.0


time = 100*ms


pixisCamera = dev('Pixis','localhost',0)


meas(ch(pixisCamera, 0), time+ 33*us, ("test",1))


meas(ch(pixisCamera, 0), time+ 11*s, ("test",2))

meas(ch(pixisCamera, 0), time+ 15*s, ("test",2))