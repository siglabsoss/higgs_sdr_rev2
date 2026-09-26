import sys
sys.path.insert(0,"../../../../python-osi")
sys.path.insert(0,"../../../scripts")
from sigmath import *
from subcarrier_math import *

from numpy.fft import ifft, fft, fftshift

# from evm_result_0 import a


def _signed_value(value, bit_count):
    if value > 2**(bit_count - 1) - 1:
        value -= 2**bit_count 

    return value



def _load_hex_file(filepath):
    with open(filepath) as bootprogram:
        lines = bootprogram.readlines()

    words = [int(l,16) for l in lines]

    rf = []
    for w in words:
        real = _signed_value(w&0xffff, 16)
        imag = _signed_value((w >> 16) & 0xffff, 16)
        rf.append(np.complex(real,imag))
        # print "r", real, " i ", imag
        # print rf[0]
        # sys.exit(0)
    return rf

def to_c(val):
	g = 2.0**15-1
	g = 1.0
	im = (val >> 16) & 0xffff;
	re = val & 0xffff;
	return np.complex(re/g,im/g)



path = "./e_noise.hex"
data = _load_hex_file(path)

# sample = 0x00001000

# aslist = []

# asnp = np.array([])

# data_clip0 = data[0:10000]

nplot(np.abs(data), "in1 abs");
# ncplot(data)
# nplotqam(data)



nplotshow()