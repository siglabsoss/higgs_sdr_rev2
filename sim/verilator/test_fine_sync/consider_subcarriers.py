import sys
sys.path.insert(0,"../../../../python-osi")
sys.path.insert(0,"../../../scripts")
from sigmath import *
from subcarrier_math import *

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


# returns non 0 for failure
def verify_counter(words, subcarriers):

    llen = len(words)
    bits = []
    counters = []

    expect_counter=True

    # i is the subcarrier
    # j is the fft frame number
    # idx is the line number
    for j in range(llen/1024):
        for i in range(1024):
            idx = (j*1024)+i
            w = words[idx]

            if i in subcarriers:

                if not expect_counter:
                    pass
                else:
                    counters.append(w)

            else:
                if w != 0:
                    print "non zero value at fft:", j, " subcarrier:", i, "line:", idx+1
                    return 0xE0000000 | idx

    lenc = len(counters)
    print "Found", lenc, "counter values"

    start_range = 0xF0000000
    end_range = 0xF0000000+lenc

    expected_values = range(start_range, end_range)

    did_pass = 1

    for i in range(lenc):
        if counters[i] != expected_values[i]:
            did_pass = 0
            print "at", i, "expected:", hex(expected_values[i]), "got:", hex(counters[i])
    
    assert did_pass

    print "All", lenc, "counters from", hex(start_range), "to", hex(end_range), "were correct"

    # for xx in counters:
    #     print hex(xx)

def recover_bits_de_moved(words, debug_qpsk=False):
    llen = len(words)
    bits = []
    counters = []

    expect_counter = False

    # i is the subcarrier
    # j is the fft frame number
    # idx is the line number
    for j in range(llen/1024):
        for i in range(1024):
            idx = (j*1024)+i
            w = words[idx]

           

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



def main():

    filepath = "cs10_out.hex"

    subcarriers = [111, 122, 133, 144, 866, 877, 888, 899]
    # subcarriers = range(264, 264+16) + range(640, 640+16)
    # subcarriers = [11, 22, 33, 44, 66, 77, 88, 99] + [111, 122, 133, 144, 866, 877, 888, 899]

    subcarriers = [0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255, 271, 286, 301, 316, 331, 346, 361, 376, 391, 406, 421, 436, 451, 466, 481, 496]
    subcarriers = [0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255, 271, 286, 301, 316, 331, 346, 361, 376, 391, 406, 421, 436, 451, 466, 481, 496, 497, 514, 531, 548, 565, 582, 599, 616, 633, 650, 667, 684, 701, 718, 735, 752]
    subcarriers = [0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255, 271, 286, 301, 316, 331, 346, 361, 376, 391, 406, 421, 436, 451, 466, 481, 496, 497, 514, 531, 548, 565, 582, 599, 616, 633, 650, 667, 684, 701, 718, 735, 752, 752, 769, 786, 803, 820, 837, 854, 871, 888, 905, 922, 939, 956, 973, 990, 1007]
    subcarriers = [0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255, 271, 286, 301, 316, 331, 346, 361, 376, 391, 406, 421, 436, 451, 466, 481, 496, 512, 529, 546, 563, 580, 597, 614, 631, 648, 665, 682, 699, 716, 733, 750, 767, 768, 785, 802, 819, 836, 853, 870, 887, 904, 921, 938, 955, 972, 989, 1006, 1023]
    subcarriers = [16, 33, 2, 3, 4, 21, 38, 39, 56, 57, 106, 27, 28, 29, 14, 47] + [48, 49, 18, 67, 36, 53, 70, 87, 104, 73, 138, 43, 60, 45, 30, 127] + [128, 65, 34, 99, 52, 85, 102, 215, 120, 121, 186, 75, 92, 77, 110, 207] + [176, 97, 66, 115, 84, 133, 214, 247, 136, 153, 202, 91, 124, 109, 142, 239] + [192, 113, 162, 147, 148, 181, 246, 423, 168, 169, 250, 107, 140, 157, 158, 271] + [256, 145, 210, 163, 164, 229, 294, 439, 200, 249, 298, 123, 172, 189, 174, 287] + [272, 161, 242, 195, 196, 261, 326, 455, 232, 297, 314, 155, 204, 205, 190, 335] + [320, 177, 290, 211, 244, 341, 406, 487, 264, 313, 378, 171, 220, 269, 206, 431]
    subcarriers = [16, 33, 2, 3, 4, 21, 38, 39, 56, 57, 106, 27, 28, 29, 14, 47]
    subcarriers = [16, 33, 2, 3, 4, 21, 38, 39, 56, 57, 10, 27, 28, 29, 14, 47] + [100, 116, 101, 117, 102, 118, 103, 119, 104, 120, 105, 121, 106, 122, 107, 132]
    subcarriers = [16, 33, 2, 3, 4, 21, 38, 39, 56, 57, 10, 27, 28, 29, 14, 47, 100, 116, 101, 117, 102, 118, 103, 119, 104, 120, 105, 121, 106, 122, 107, 132]
    subcarriers = [16, 33, 2, 3, 4, 21, 38, 39, 56, 57, 106, 27, 28, 29, 14, 47, 48, 49, 18, 67, 36, 53, 70, 87, 104, 73, 138, 43, 60, 45, 30, 127, 128, 65, 34, 99, 52, 85, 102, 215, 120, 121, 186, 75, 92, 77, 110, 207, 176, 97, 66, 115, 84, 133, 214, 247, 136, 153, 202, 91, 124, 109, 142, 239, 192, 113, 162, 147, 148, 181, 246, 423, 168, 169, 250, 107, 140, 157, 158, 271, 256, 145, 210, 163, 164, 229, 294, 439, 200, 249, 298, 123, 172, 189, 174, 287, 272, 161, 242, 195, 196, 261, 326, 455, 232, 297, 314, 155, 204, 205, 190, 335, 320, 177, 290, 211, 244, 341, 406, 487, 264, 313, 378, 171, 220, 269, 206, 431]
    subcarriers = range(600, 600+256)
    subcarriers = range(0, 0+32)

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
    # recover_bits(words, subcarriers, False)
    # recover_bits(words, subcarriers, False, True)

    # just displays all
    # display_only(words, subcarriers)

    # expects a counter starting at value 0xf0000000
    # verify_counter(words, subcarriers)

    # recovers original bits from de-moved subcarriers
    recover_bits_de_moved(words, False)


if __name__ == '__main__':
    main()









# for i,xx in enumerate(as_str):
#     print hex(ord(xx))
#     if i % 4 == 3:
#         print ""

# for i in range(0,len(as_str),4):
#     # w = \
#     #     rev_8(ord(as_str[i])) << 24 |   \
#     #     rev_8(ord(as_str[i+1])) << 16 | \
#     #     rev_8(ord(as_str[i+2])) << 8 |  \
#     #     rev_8(ord(as_str[i+3])) << 0

#     # w = \
#     #     rev_8(ord(as_str[i])) << 0 |   \
#     #     rev_8(ord(as_str[i+1])) << 8 | \
#     #     rev_8(ord(as_str[i+2])) << 16 |  \
#     #     rev_8(ord(as_str[i+3])) << 24

#     w = \
#         swap_8(ord(as_str[i])) << 24 |   \
#         swap_8(ord(as_str[i+1])) << 16 | \
#         swap_8(ord(as_str[i+2])) << 8 |  \
#         swap_8(ord(as_str[i+3])) << 0

#     w = \
#         (ord(as_str[i])) << 24 |   \
#         (ord(as_str[i+1])) << 16 | \
#         (ord(as_str[i+2])) << 8 |  \
#         (ord(as_str[i+3])) << 0

#     print hex(w)





    # if i % 4 == 3:
    #     print ""



# for i,xx in enumerate(as_str):
#     if i % 8 == 0:
#         print bin(ord(xx))

#     if i % 8 == 4:
#         print bin(ord(xx))

# print as_str
 