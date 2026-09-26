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


def _load_hex_file_hex(filepath):
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
    return words


def to_c(val):
    g = 2.0**15-1
    g = 1.0
    im = (val >> 16) & 0xffff;
    re = val & 0xffff;
    return np.complex(re/g,im/g)


# (8192-1j)
# (8191+0j)
# (8191+1j)
# (8192+1j)
# (8191+0j)
# (8190+1j)
# (8192+1j)
# (8191+0j)
# (8192+0j)
# (8190+0j)
# (8192+1j)

def is_pilot(v):
    rr = np.abs((8190+1j) - v) #(8190+1j)
    
    if rr < 8:
        return True
    else:
        return False


chunk = 1024+1024+16

path = "./cs21_out.hex"
data11 = _load_hex_file_hex(path)

for i in range(0,len(data11)-chunk,chunk):
    ln = ''
    coutout = data11[i:i+chunk]

    tail = coutout[2048:2064]

    for tt in tail:
        print(hex(tt))

    print ""
    print ""
    # print(hex(coutout[]))
    








    # print data[0:1024]

# sample = 0x00001000

# aslist = []

# asnp = np.array([])

# data_clip0 = data[0:10000]

# ncplot(data)
# nplotqam(data)

# nplot(np.abs(data11), "");
# nplotshow()


