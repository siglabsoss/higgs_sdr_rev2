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

def striptrunk(data):
	frames = []
	build = []
	for i,x in enumerate(data):
		if (i % 1040) <= 1023:
			build.append(x)

		if (i % 1040) == 1023:
			# print build
			frames.append(build)
			build = []
	return np.array(frames)
	pass


# load_hex_file_uint32
# load_hex_file_rf
# write_hex_file_uint32

path = "./cs22_out.hex"
longframesserial = load_hex_file_rf(path)
# longframesu32 = load_hex_file_uint32(path)
# longframes = intoframes(longframesserial)

outframes = striptrunk(longframesserial)

data_chan = [39,41]
clock_chan = 42


glom = []

# for i in [39,41,42]:
for i in [39]:
# for i in [39,38]:
# for i in [38,39,42]:
	extract = outframes[:,i:i+1]
	u32 = rf_to_uint32(np.array(extract))
	for s in u32:
		print hex(s)
	glom.extend(extract)
nplotqam(glom)

ncplot(glom)

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