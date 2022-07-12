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



# a = [0,1,2]

# nplot(a)
# nplotshow()


# x = []
# y = []

# import csv

# with open('fill.csv') as csv_file:
#     csv_reader = csv.reader(csv_file, delimiter=',')
#     line_count = 0
#     for row in csv_reader:
#         x.append(int(row[0]))
#         y.append(int(row[1]))
#         # print()


# x = [1,2,3,4,5,6,0,1,2,3,4,5,6]



do_cut = True
cut_front = 1280 * 19 # because we adjust the level with ringbus


x = _load_hex_file("cs10_out.hex")
# x = _load_hex_file("cs10_ringbus.hex")



# offset = -410

# x = x[0:1280*15]

offset = 0

if do_cut:
    offset += cut_front


nocp = []
for i in range(offset,len(x),1280):
    # print i
    cut = x[i+256:i+1280]
    # cut.reverse()
    # nocp.extend([0])
    # ncplot(cut, "cut " + str(i))
    # nplotfft(cut)
    nocp.extend(cut)

asfft = []

N = 1024

for i in range(0,len(nocp),1024):
    data = nocp[i:i+1024]
    fftchunk = fft(data)/32.0 # gain down
    # ncplot(fftchunk, "fft " + str(i))
    asfft.extend(fftchunk)


allsc = [None]*1024

for i in range(1024):
    allsc[i] = [32767+32767j, 32767-32767j,-32767+32767j,-32767-32767j]



# asc = [32767+32767j, 32767-32767j,-32767+32767j,-32767-32767j]
# asc = []
# sc = 1013


for idx in range(len(asfft)):
    scidx = idx % 1024;
    # if idx % 1024 == sc:
    allsc[scidx].append(asfft[idx])



show_sc = [
1
# ,3
# ,13
# ,77
# ,1011
,1013
]

for sc in show_sc:
    nplotqam(allsc[sc], "a subcarrier " + str(sc) )
    # ncplot(allsc[sc], "a subcarrier " + str(sc) )


ncplot(asfft, "As fft")
# ncplot(x, "orig")


nplotshow()











# xshort = x[256:1024+256]

# for i,y in enumerate(xshort):
#     print "i ", i, " y ", y

# save_rf_grc("cs10_out.raw", nocp)

# sc = 13
# sc = 5

# for idx in range(len(x)):
#     if idx % 64 == sc:
#         y.append(x[idx])

# # print(y)

# xx = []
# yy = []

# for p in y:
#     xx.append(np.real(p))
#     yy.append(np.imag(p))
