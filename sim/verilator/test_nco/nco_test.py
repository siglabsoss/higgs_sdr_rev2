import unittest
import sys
sys.path.insert(0, "../../")

from scipy import signal

import numpy as np
import matplotlib.pyplot as pt
from math import floor
import struct
from struct import *
from scipy.signal import max_len_seq
from scipy.signal import correlate

class working(unittest.TestCase):

    def test_nco(self):
        samples_per_period=1<<12
        nco_signal = np.array([])
        with open('cs20_out.hex', 'rb') as file:
            temp=file.readline()
            while(temp):
                real=int(temp, 16) & 0xffff
                if real>0x7fff:
                    real=real-0x10000

                imag=(int(temp, 16) >> 16) & 0xffff
                if imag>0x7fff:
                    imag=imag-0x10000
                nco_signal = np.append(nco_signal, complex(real, imag))
                temp=file.readline()

        print len(nco_signal)
        print nco_signal

        pt.figure()
        pt.subplot(411)
        pt.plot(abs(nco_signal))
        pt.title('Mag')
        pt.subplot(412)
        pt.plot(np.angle(nco_signal)*180/np.pi)
        pt.title('Angle')
        pt.subplot(413)
        pt.plot(np.real(nco_signal))
        pt.title('Real')
        pt.subplot(414)
        pt.plot(np.imag(nco_signal))
        pt.title('imag')

        
        index=[a for a in range(samples_per_period)]*8
        data=[]
        for a in range(len(index)):
            data.append((2**15-1)*np.exp(1j*2*np.pi*index[a]/(1.0*samples_per_period)+1j*3*np.pi/2))
            
        data=np.asarray(data, dtype=np.complex128)
        data = data[0:32768]
        print len(data), len(nco_signal)

        pt.figure()
        pt.subplot(411)
        pt.plot(abs(data))
        pt.title('Mag')
        pt.subplot(412)
        pt.plot(np.angle(data) * 180 / np.pi)
        pt.title('Angle')
        pt.subplot(413)
        pt.plot(np.real(data))
        pt.title('Real')
        pt.subplot(414)
        pt.plot(np.imag(data))
        pt.title('imag')

        pt.figure()
        pt.subplot(211)
        pt.hold(True)
        pt.plot(np.real(data), 'r')
        pt.plot(np.real(nco_signal), 'b')
        pt.title('Real')
        pt.subplot(212)
        pt.plot(np.real(nco_signal)-np.real(data))
        pt.title('Diff Real')
        pt.show()


if __name__ == '__main__':
    unittest.main()
