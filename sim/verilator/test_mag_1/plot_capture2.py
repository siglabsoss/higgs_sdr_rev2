import sys
sys.path.insert(0,"../../../../python-osi")
sys.path.insert(0,"../../../scripts")
from sigmath import *
from subcarrier_math import *

from numpy.fft import ifft, fft, fftshift
import random


def streamify(source):
	out = []
	for x in source:
		out.extend(x)
	return out

def intoframes(data):
	frames = []
	build = []
	for i,x in enumerate(data):
		build.append(x)
		if (i % 1024) == 1023:
			frames.append(build)
			build = []
	return np.array(frames)



# load_hex_file_uint32
# load_hex_file_rf
# write_hex_file_uint32

path = "./longframes_2.hex"
longframesserial = load_hex_file_rf(path)
longframesu32 = load_hex_file_uint32(path)
longframes = intoframes(longframesserial)

data_chan = [39,41]
clock_chan = 42

# rgain = 300

# for row in longframes:
# 	pick = row[39]
# 	row[39] = np.complex( np.real(pick) + (random.random()*rgain),  np.imag(pick) + (random.random()*rgain))


# u32 = rf_to_uint32([(1+2j), (3-4j)])
# print u32

glom = []

# for i in [39,41,42]:
# for i in [39, 41]:
# for i in [39, 40]:
for i in [39, 38, 42]:
# for i in [39,38]:
# for i in [38,39,42]:
	extract = longframes[:,i:i+1]
	u32 = rf_to_uint32(np.array(extract))
	for s in u32:
		print hex(s)
	glom.extend(extract)
nplotqam(glom)

# writeout = streamify(longframes)

# writeout = writeout[0:8]

# print writeout

# write_vmem_hex('./cooked_1.h', 'cooked_data', writeout)

# write_hex_file_rf('longframes_1.hex', writeout)

# ss = writeout[1024+39]
# print ss
# xx = 
# print hex(xx)


# ncplot(data)
# nplotfft(data)
nplotshow()