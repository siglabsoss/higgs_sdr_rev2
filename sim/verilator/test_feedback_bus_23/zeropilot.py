import sys
sys.path.insert(0,"../../../../python-osi")
sys.path.insert(0,"../../../scripts")
from sigmath import *
from subcarrier_math import *

from numpy.fft import ifft, fft, fftshift


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

def _file(filename, max_samples=None):
    file = open(filename, 'r')
    piece_size = 8
    dout = []
    sample_count = 0
    while True:
        bytes = file.read(piece_size)

        if bytes == "":
            break  # end of file
        dout.append(bytes_to_floats(bytes))
        dout.append(bytes)
        sample_count += 1
        if max_samples is not None and sample_count >= max_samples:
            break

    return dout


# data = _file('/home/x/Desktop/output.raw', 2)
data = read_rf_grc('/home/x/Desktop/outputc.raw')

# print data

chans = [None]*64
# print chans
for i in range(len(chans)):
    chans[i] = []
# print chans

k = 0
for i in range(len(data)):
    k = i % 64
    chans[k].append(data[i])


ncplot(chans[0], "0")
ncplot(chans[1], "1")
ncplot(chans[2], "2")
ncplot(chans[3], "3")
ncplot(chans[62], "62")


# 

# in1 = _load_hex_file("input.hex");
# in2 = in1[3179000:]
# in3 = _load_hex_file("/mnt/overflow/work/software_parent_repo/higgs_sdr_rev2/libs/s-modem/soapy/productionjs/residue_upstream3.txt")
# # in3 = in3[3000000:]

# in3res = _load_hex_file("/mnt/overflow/work/software_parent_repo/higgs_sdr_rev2/libs/s-modem/soapy/productionjs/save_residue3_strip.txt")

# nplot(np.abs(in3), "in3");
# nplot(np.abs(in3res), "in3res");

# for x in in3res:
# 	re = np.real(x)
# 	im = np.imag(x)

# 	mag2 = (re*re) + (im*im)
# 	if( mag2 < 2000):
# 		print(x)


# dump28 = _load_hex_file("/mnt/overflow/work/software_parent_repo/higgs_sdr_rev2/libs/s-modem/soapy/productionjs/dump29.hex");

# nplot(np.abs(dump28), "28")
# nplot(np.abs(in3res), "file")


# y1 = 187121672.0
# y2 = 187127447.0

# x1 = 5796.0
# x2 = 156.0


# slope = (x1-x2) / (y2-y1)

# print("slope", slope)


# nplotmulti([y],[x],["x"]);

# nplot(np.abs(in1), "in1 abs");

# nplot(in1)
# nplotqam(in2)
# nplot(np.abs(in2), "amp")

# nplot(np.abs(in3), "in3 real")

# nplot(np.abs(in3res), "in3 res")

nplotshow()