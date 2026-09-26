zero = 0x16a1
one = 0xe95f

def vval(iin):
	if iin == 0:
		return zero << 16 | zero;
	if iin == 1:
		return zero << 16 | one;
	if iin == 2:
		return one << 16 | zero;
	if iin == 3:
		return one << 16 | one;

# ab = vval(0)
# print hex(ab)

# ab = vval(1)
# print hex(ab)

# ab = vval(2)
# print hex(ab)

# ab = vval(3)
# print hex(ab)

print "VMEM_SECTION unsigned int vmem0[1040] = {"

for x in range(1040):
	iq = vval(0)
	if( x != 0 ):
		print ",",
	if x >= 1024:
		print hex(0),
	else:
		print hex(iq),

print "};"


print "VMEM_SECTION unsigned int vmem1[1040] = {"

for x in range(1040):
	iq = vval(1)
	if( x != 0 ):
		print ",",
	if x >= 1024:
		print hex(0),
	else:
		print hex(iq),

print "};"


print "VMEM_SECTION unsigned int vmem2[1040] = {"

for x in range(1040):
	iq = vval(2)
	if( x != 0 ):
		print ",",
	if x >= 1024:
		print hex(0),
	else:
		print hex(iq),

print "};"

print "VMEM_SECTION unsigned int vmem3[1040] = {"

for x in range(1040):
	iq = vval(3)
	if( x != 0 ):
		print ",",
	if x >= 1024:
		print hex(0),
	else:
		print hex(iq),

print "};"
