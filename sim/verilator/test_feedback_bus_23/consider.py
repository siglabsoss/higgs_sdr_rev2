from __future__ import print_function
import math
import sys
sys.path.insert(0,"../../../scripts")
from subcarrier_math import *


def main():

    filepath = "cs20_out.hex"

    lines = []

    with open(filepath) as bootprogram:
        lines = bootprogram.readlines()

    words = [int(l,16) for l in lines]

    llen = len(words)
    frames = int(math.floor(llen/1024.0))

    print("got ", len(words), "words")


    print("got ", frames, "frames")

    for r in range(frames):
        for w in words[r*1024:(r+1)*1024]:
            print('%#10x' % w, end='')
            print(', ', end='')
        print('')
        # print(  ) 


    # looks at subcarriers complans when not there
    # extracts data from bpsk
    # recover_bits(words, subcarriers, False)
    # recover_bits(words, subcarriers, False, True)

    # just displays all
    # display_only(words, subcarriers)

    # expects a counter starting at value 0xf0000000
    # verify_counter(words, subcarriers)

    # recovers original bits from de-moved subcarriers
    # recover_bits_de_moved(words, False)


if __name__ == '__main__':
    main()





