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



path = "./cs11_out.hex"
data11 = _load_hex_file(path)

for i in range(0,len(data11)-1040,1040):
    ln = ''
    for j in data11[i:i+1024][::4]:
        if is_pilot(j):
            ln += '1'
        else:
            ln += '0'

    lifetime = int(np.real(data11[i+1030]))

    ln += ': '

    s1 = str(lifetime)
    ln += (3-len(s1)) * ' '
    ln += s1

    s2 = str(lifetime%128)
    ln += (4-len(s2)) * ' '
    ln += s2


    print(ln)


print('')
print('')
print('')
sys.exit(0)












path12 = "./cs12_out.hex"
data12 = _load_hex_file(path12)

for i in range(0,len(data12)-1024,1024):
    ln = ''
    for j in data12[i:i+1024][::4]:
        
        if is_pilot(j):
            ln += '1'
        else:
            ln += '0'

    print(ln)

print('')
print('')
print('')



path02 = "./cs02_out.hex"
data02 = _load_hex_file(path02)

for i in range(0,len(data02)-1024,1024):
    ln = ''
    for j in data02[i:i+1024][::4]:
        
        if is_pilot(j):
            ln += '1'
        else:
            ln += '0'

    print(ln)








    # print data[0:1024]

# sample = 0x00001000

# aslist = []

# asnp = np.array([])

# data_clip0 = data[0:10000]

# ncplot(data)
# nplotqam(data)

# nplot(np.abs(data11), "");
# nplotshow()


