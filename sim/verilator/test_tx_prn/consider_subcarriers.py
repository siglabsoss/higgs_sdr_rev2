import sys
sys.path.insert(0,"../../../scripts")
from subcarrier_math import *

# just prints lit subcarriers

def main():

    filepath = "cs20_out.hex"



    lines = []

    with open(filepath) as bootprogram:
        lines = bootprogram.readlines()

    words = [int(l,16) for l in lines]

    for i in range(0,len(words)-1023,1024):
        for j,w in enumerate(words[i:i+1024]):
            if w != 0:
                print j
            # print "0x{:08x}".format(w),
        print ""
        print ""

    # if do_reorder:
    #     words = reorder_lines(words, subcarriers, binned)

    llen = len(words)

    print("got", len(words))




if __name__ == '__main__':
    main()





