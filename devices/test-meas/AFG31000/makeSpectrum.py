import math

def makePulse(duration, spectrum):
    sampleRate = 1000         #MS/s
    dt = (1000/sampleRate)    #ns
    fc = 50    #carrier frequenc in MHz
    units = 1e-3        # MHz * ns

    nPoints = int(sampleRate * duration * units)
    
    data=[]

    def makeSinPoint(t, freq, amplitude, phi=0):
        return amplitude * math.sin( 2*math.pi*(fc + freq)*t*dt*units + phi )
    
    for t in range(0, nPoints):
        sample = 0
        
        for s in spectrum:
            sample += makeSinPoint(t, *s)   #unpack spectral component list
        
        data.append(sample)
    
    return data


spectrum=[(-10,1), (10,1,3.1), (0,0.5)]
print(makePulse(200, spectrum))
