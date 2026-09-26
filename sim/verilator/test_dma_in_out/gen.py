import struct

fp = open("input.dat", "wb+")

for n in range(32):
	fp.write(struct.pack('<I',n))
