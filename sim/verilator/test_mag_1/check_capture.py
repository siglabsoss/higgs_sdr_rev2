import sys
sys.path.insert(0,"../../../../python-osi")
sys.path.insert(0,"../../../scripts")
from sigmath import *
from subcarrier_math import *

from numpy.fft import ifft, fft, fftshift


# load_hex_file_uint32
# load_hex_file_rf
# write_hex_file_uint32

path = "./capture_1.hex"
data = load_hex_file_uint32(path)

ideal_cnt = 0
loaded = 0
value = 0
final = []

for x in data:
	tag = x&0xff000000;
	if tag != 0x55000000:
		print tag,'is wrong'
		break
	cnt = (x&0xff0000) >> 16;
	if cnt != ideal_cnt:
		print cnt,'is wrong',ideal_cnt
		break
	ideal_cnt = (ideal_cnt+1) % (0xff+1)

	data = (x&0xffff);

	# print(cnt)
	if loaded % 2 == 0:
		value = data
	else:
		value = value | (data<<16)
		final.append(value)
	loaded += 1

for x in final:
	print "{0:#0{1}x}".format(x,10)[2:]

write_hex_file_uint32('./capture_2.hex', final)

