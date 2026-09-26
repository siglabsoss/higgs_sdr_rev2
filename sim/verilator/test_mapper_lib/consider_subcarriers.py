import sys
sys.path.insert(0,"../../../scripts")
from subcarrier_math import *

def bits_to_str(bits):
    chars = []
    for b in range(len(bits) / 8):
        byte = bits[b*8:(b+1)*8]
        chars.append(chr(int(''.join([str(bit) for bit in byte]), 2)))
    return ''.join(chars)

def demap_debug_bpsk(val):
    if   val == 0xF0000003:
        return [1,1]
    elif val == 0xF0000002:
        return [0,1]
    elif val == 0xF0000001:
        return [1,0]
    elif val == 0xF0000000:
        return [0,0]
    else:
        return None

def demap_bits(val):
    QPSK_POS = (0x5a81)  # //+0.7
    QPSK_NEG = (0xa57e)  # //-0.7
    if   val == (QPSK_POS << 16 | QPSK_POS):
        return [1,1]
    elif val == (QPSK_POS << 16 | QPSK_NEG):
        return [0,1]
    elif val == (QPSK_NEG << 16 | QPSK_POS):
        return [1,0]
    elif val == (QPSK_NEG << 16 | QPSK_NEG):
        return [0,0]
    else:
        return None

def rev_8(n):
    # return n
    return int('{:08b}'.format(n)[::-1], 2)

def swap_8(n):
    ssc = '{:08b}'.format(n)
    # ssc =  ss[7] + ss[6] + ss[5] + ss[4] + ss[3] + ss[2] + ss[1] + ss[0]
    # ssc =  ss[7] + ss[2] + ss[5] + ss[1] + ss[3] + ss[6] + ss[4] + ss[0]
    return int(ssc,2)
    # return 0

def swap_32(n):
    ss = '{:032b}'.format(n)
    print ss
    return 0

# this works when tb.cpp does an inverse swizzle
def swap_64(n):
    s = '{:064b}'.format(n)
    sc = \
    s[03+4*7] + \
    s[03+4*6] + \
    s[03+4*5] + \
    s[03+4*4] + \
    s[03+4*3] + \
    s[03+4*2] + \
    s[03+4*1] + \
    s[03+4*0] + \
    s[02+4*7] + \
    s[02+4*6] + \
    s[02+4*5] + \
    s[02+4*4] + \
    s[02+4*3] + \
    s[02+4*2] + \
    s[02+4*1] + \
    s[02+4*0] + \
    s[35+4*7] + \
    s[35+4*6] + \
    s[35+4*5] + \
    s[35+4*4] + \
    s[35+4*3] + \
    s[35+4*2] + \
    s[35+4*1] + \
    s[35+4*0] + \
    s[34+4*7] + \
    s[34+4*6] + \
    s[34+4*5] + \
    s[34+4*4] + \
    s[34+4*3] + \
    s[34+4*2] + \
    s[34+4*1] + \
    s[34+4*0] + \
    s[01+4*7] + \
    s[01+4*6] + \
    s[01+4*5] + \
    s[01+4*4] + \
    s[01+4*3] + \
    s[01+4*2] + \
    s[01+4*1] + \
    s[01+4*0] + \
    s[00+4*7] + \
    s[00+4*6] + \
    s[00+4*5] + \
    s[00+4*4] + \
    s[00+4*3] + \
    s[00+4*2] + \
    s[00+4*1] + \
    s[00+4*0] + \
    s[33+4*7] + \
    s[33+4*6] + \
    s[33+4*5] + \
    s[33+4*4] + \
    s[33+4*3] + \
    s[33+4*2] + \
    s[33+4*1] + \
    s[33+4*0] + \
    s[32+4*7] + \
    s[32+4*6] + \
    s[32+4*5] + \
    s[32+4*4] + \
    s[32+4*3] + \
    s[32+4*2] + \
    s[32+4*1] + \
    s[32+4*0]
    # print sc
    return int(sc,2)

def recover_bits(words, subcarriers, debug_qpsk=False, expect_counter=False):

    llen = len(words)
    bits = []
    counters = []
    res = []

    # i is the subcarrier
    # j is the fft frame number
    # idx is the line number
    for j in range(llen/1024):
        for i in range(1024):
            idx = (j*1024)+i
            w = words[idx]

            if i in subcarriers:

                if not expect_counter:
                    demapped = demap_debug_bpsk(w) if debug_qpsk else demap_bits(w)
                    if demapped is None:
                        if( w == 0 ):
                            print "zero found at fft:", j, " subcarrier: ", i
                        else:
                            print str(hex(w)) + " was not found in lookup"
                    else:
                        bits.extend(demapped)
                else:
                    counters.append(w)

            else:
                if w != 0:
                    print "non zero value at fft:", j, " subcarrier:", i, "line:", idx+1

    as_str = bits_to_str(bits)


    if expect_counter:
        for xx in counters:
            print hex(xx)


    if not expect_counter:
        for i in range(0,len(as_str)-8,8):
            w64 = \
                (ord(as_str[i]))   << 8*7 |   \
                (ord(as_str[i+1])) << 8*6 |   \
                (ord(as_str[i+2])) << 8*5 |   \
                (ord(as_str[i+3])) << 8*4 |   \
                (ord(as_str[i+4])) << 8*3 |   \
                (ord(as_str[i+5])) << 8*2 |   \
                (ord(as_str[i+6])) << 8*1 |   \
                (ord(as_str[i+7])) << 0

            w64_transformed = swap_64(w64)

            w32a = 0xffffffff & w64_transformed
            w32b = 0xffffffff & (w64_transformed>>32)

            # print '{:064b}'.format(w)
            # print '{:016x}'.format(w)
            # print hex(w64_transformed)
            print hex(w32a)
            print hex(w32b)
            res.append(w32a)
            res.append(w32b)
    return res



def display_only(words, subcarriers):

    llen = len(words)
    bits = []
    counters = []

    expect_counter = True

    # i is the subcarrier
    # j is the fft frame number
    # idx is the line number
    for j in range(llen/1024):
        for i in range(1024):
            idx = (j*1024)+i
            w = words[idx]

            if w != 0:
                strr = ""
                if i in subcarriers:
                    strr = "(in subcarrier)"

                print hex(w), " at fft:", j, " subcarrier:", i, "line:", idx+1, strr


def main():

    filepath = "cs11_out.hex"

    subcarriers = [0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255]

    subcarriers = sorted(subcarriers)

    # set if subcarriers were reordered during moving
    do_reorder = True

    if do_reorder:
        binned = bin_subcarriers(subcarriers, True)


    lines = []

    with open(filepath) as bootprogram:
        lines = bootprogram.readlines()

    words = [int(l,16) for l in lines]

    if do_reorder:
        words = reorder_lines(words, subcarriers, binned)

    llen = len(words)

    print("got", len(words))


    # looks at subcarriers complans when not there
    # extracts data from bpsk
    vals = recover_bits(words, subcarriers, False, False)

    assert len(vals) >= 8, "didn't get enough values"
    vals = vals[0:8]

    start_counter = 0x192d3afa

    assert vals == range(start_counter, start_counter+8), "first 8 counters were wront"

    print "Mapped values are correct"



if __name__ == '__main__':
    main()
