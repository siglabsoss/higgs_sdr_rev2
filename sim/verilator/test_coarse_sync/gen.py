import struct

fp = open("input.dat", "wb+")

for n in range(1024):
	fp.write(struct.pack('<I',n))
