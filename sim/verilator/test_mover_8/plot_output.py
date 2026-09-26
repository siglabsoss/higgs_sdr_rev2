import sys
sys.path.insert(0,"../../../../python-osi")
sys.path.insert(0,"../../../scripts")
from sigmath import *
from subcarrier_math import *




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


x = []
y = []

# import csv

# with open('fill.csv') as csv_file:
#     csv_reader = csv.reader(csv_file, delimiter=',')
#     line_count = 0
#     for row in csv_reader:
#         x.append(int(row[0]))
#         y.append(int(row[1]))
#         # print()


# x = [1,2,3,4,5,6,0,1,2,3,4,5,6]

x = _load_hex_file("fb_stream.hex")

sc = 13
sc = 5

for idx in range(len(x)):
    if idx % 64 == sc:
        y.append(x[idx])

# print(y)

xx = []
yy = []

for p in y:
    xx.append(np.real(p))
    yy.append(np.imag(p))

ncplot(y)
nplotqam(y)
nplotshow()