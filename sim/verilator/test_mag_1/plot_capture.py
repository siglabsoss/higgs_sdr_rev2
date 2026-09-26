import sys
sys.path.insert(0,"../../../../python-osi")
sys.path.insert(0,"../../../scripts")
from sigmath import *
from subcarrier_math import *

from numpy.fft import ifft, fft, fftshift
import random


def pickframes(source, rows):
	out = []
	for x in rows:
		out.append(source[x])
	return np.array(out)


def streamify(source):
	out = []
	for x in source:
		out.extend(x)
	return out

def randomizechans(source, chans, rgain):
	for chan in chans:
		for row in source:
			pick = row[chan]
			row[chan] = np.complex( np.real(pick) + (random.random()*rgain),  np.imag(pick) + (random.random()*rgain))

def perfectchan(source, chan):
	g = 1000
	qp = np.array([np.complex(g, -g), np.complex(-g, -g,), np.complex(-g, g), np.complex(g, g)])
	filled = 0
	for row in source:
		row[chan] = qp[filled%4]
		# pick = row[chan]
		# row[chan] = np.complex( np.real(pick) + (random.random()*rgain),  np.imag(pick) + (random.random()*rgain))
		filled += 1


# load_hex_file_uint32
# load_hex_file_rf
# write_hex_file_uint32

path = "./capture_2.hex"
data = load_hex_file_rf(path)

# print 
# print len(data)
# print len(data)/1024.0
# print

fc = 12
samples = fc*1024

data = data[0:samples]


frames = []

build = []

for i,x in enumerate(data):
	build.append(x)
	if (i % 1024) == 1023:
		frames.append(build)
		build = []

# j = 0
# for i,x in enumerate(data):
# 	build.append(j)
# 	j += 1
# 	if (i % 1024) == 1023:
# 		frames.append(build)
# 		build = []
# 		j = 0

glom = []

frames = np.array(frames)

# rframes = [random.randint(0,11) for x in range(64)]
rframes = [3, 4, 1, 8, 10, 1, 5, 8, 8, 5, 7, 10, 8, 1, 8, 10, 3, 8, 4, 7, 9, 0, 4, 7, 9, 3, 0, 2, 6, 0, 4, 11, 8, 7, 0, 3, 8, 11, 11, 11, 0, 7, 9, 0, 8, 1, 6, 11, 11, 9, 1, 9, 9, 0, 7, 5, 0, 6, 8, 1, 9, 10, 9, 10]
rframes = [3, 4, 1, 8, 10, 1, 5, 8, 8, 5, 7, 10, 8, 1, 8, 10, 3, 8, 4, 7, 9, 0, 4, 7, 9, 3, 0, 2, 6, 0, 4, 11, 8, 7, 0, 3, 8, 11, 11, 11, 0, 7, 9, 0, 8, 1, 6, 11, 11, 9, 1, 9, 9, 0, 7, 5, 0, 6, 8, 1, 9, 10, 9]
longframes = pickframes(frames,rframes)

# print longframes

# for i in [36,37,38,39,40,41,42,43,44]:

data_chan = [39,41]
clock_chan = 38,42

# rgain = 300

# for row in longframes:
# 	pick = row[39]
# 	row[39] = np.complex( np.real(pick) + (random.random()*rgain),  np.imag(pick) + (random.random()*rgain))
random.seed(25345345)

randomizechans(longframes, [38,42], 300)
perfectchan(longframes, 39)

# for i in [39,41,42]:
# for i in [39]:
# for i in [39,38]:
for i in [39]:
	extract = longframes[:,i:i+1]
	glom.extend(extract)
nplotqam(glom)

writeout = streamify(longframes)

# writeout = writeout[0:8]

# print writeout

p1 = './override/fpgas/cs/cs32/c/src/cooked_2.h'
p2 = './longframes_2.hex'

if False:
	write_vmem_hex(p1, 'cooked_data', writeout)
	write_hex_file_rf(p2, writeout)
	print "Wrote files"
else:
	print "will not writeout"

# ss = writeout[1024+39]
# print ss
# xx = 
# print hex(xx)




# ncplot(data)
# nplotfft(data)
nplotshow()