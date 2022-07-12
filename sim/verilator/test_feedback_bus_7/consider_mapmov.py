import sys


def main():

    filepath = "cs11_in.hex"
    lines = []

    with open(filepath) as bootprogram:
        lines = bootprogram.readlines()

    words = [int(l,16) for l in lines]

    print "read out", len(words), "words"

    offset = 16
    end = 48*1024


    onlydata = []


    for x in range(offset,end+offset):
        idx = (x - offset) % 1024
        # print "idx", idx, " : ", hex(words[x])
        if idx % 2 == 1:
            if idx < 129 or idx > 896:
                onlydata.append(words[x])

    print "appended: ", len(onlydata)

    bitpairs = [None]*len(onlydata)

    for x in range(len(onlydata)):
        bitpairs[x] = ((onlydata[x]&0x80000000)>>(31)) | ((onlydata[x]&0x8000)>>(15-1))
        # print "w:", hex(onlydata[x])
        # print hex(bitpairs[x]),


    for x in range(len(bitpairs)):
        print bitpairs[x],
        if( x % 16 == 15):
            print ""

    reconstructed = [None] * int(len(bitpairs)/16)

    print "Will reconstruct", len(reconstructed)

    for x in range(0,len(bitpairs)-16,16):
        w = 0
        for j in range(16):
            w = (w >> 2) | (bitpairs[j+x]<<30)
        # flip bits
        reconstructed[x/16] = (~w)&0xffffffff
        # print hex( (~w)&0xffffffff)


    correct = 0
    wrong = 0
    # tail is chopped off but whatever
    for x in range(0,250):
        idx = x + 128
        print hex(reconstructed[idx]), hex(x)
        ideal = x + 0xdead0000

        if reconstructed[idx] == ideal:
            correct += 1
        else:
            wrong += 1


    print "correct", correct
    print "wrong", wrong

    if wrong != 0:
        sys.exit(1)

    print "Got ", correct, " values. All tests passed"



if __name__ == '__main__':
    main()
