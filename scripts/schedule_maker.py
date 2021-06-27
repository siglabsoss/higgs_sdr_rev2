import sys
from subcarrier_math import *
import itertools

def ranges(i):
    for a, b in itertools.groupby(enumerate(i), lambda (x, y): y - x):
        b = list(b)
        yield b[0][1], b[-1][1]


# if you have a list of subcarriers and you want to collapse it into a bunch of range()'s for simplicity
# just run:
#    print get_ranges([1,2,3,4])
def get_ranges(i):
    return list(ranges(i))

##################
#
#  Provide a list of subcarriers, generates output schedules
#  Rule: input # of subcarriers must be a multiple of 16
#
#  Even if we have 32 subcarriers, there is no guarentee that we would use 2 schedules to move
#  due to conflicts
#

def build_dic(subcarriers, rotation, last_mapped_index):
    dic1 = dict()
    conf1 = dict()
    for i, item in enumerate(sorted(subcarriers)):
        column = item%16
        row = item/16
        prem = i-column+rotation
        if (prem < 0):
            prem = 16+prem

        if (prem >= 16):
            prem = prem-16

        index = item%16

        if (dic1.has_key(index)):
            print "Carrier %s is in conflict with %s" % (dic1[index][0],item)
            conf1[index] = ((item, prem, item/16, item%16))

        dic1[index] = dict(carrier=item, perm=prem, row=item/16, col=item%16, last=int((last_mapped_index+i)/16), origin=i+rotation)
    return dic1, conf1

def calculate_input_start(dic1):
    input_starting_off = {}
    # input_bump = {}
    for index in range(16):
        input_starting_off[index] = 0
        # input_bump[index] = 0
    for index in range(16):
        value = dic1.get(index,None)
        if (value):
            perm = value['perm']
            item = value['carrier']
            perm2 = (index + perm) % 16
            column = item%16
            in_off = value['last']
            # print "item", item, "idx", index, "in_off", in_off, "col", column, "perm2", perm2
            input_starting_off[perm2] = in_off
            # input_bump[perm2] = value[4]
    return input_starting_off

# Even a garbage output row could be the input of a valid row
# in this case, we need to calculate the source row based from output rows

def print_forward(dic1, input_stride, output_stride, prefix):
    input_starting_off = calculate_input_start(dic1)

    # loop through index (this is output index)
    for index in range(16):
        value = dic1.get(index,None)

        sp = ' '
        if index > 9:
            sp = ''

        if (value):
            print (prefix + "[" + str(index) + sp + \
                "] = (Schedule) {MOVER_SRC_ROW+%d, %d, (0x%x << 12) | (DST_ROW + %d), 0x%x}; //Column %d" \
                % (input_starting_off[index], input_stride,value['perm'],value['row'],output_stride,value['col']))
        else:
            print (prefix + "[" + str(index) + sp + \
                "] = (Schedule) {MOVER_SRC_ROW+%d, %d, (0 << 12) | (GARBAGE_ROW  ), 0x0};  //Column %d" \
                % (input_starting_off[index], input_stride, index))

def print_reverse(dic1, input_stride, output_stride, prefix):
    input_starting_off = calculate_input_start(dic1)

    dic2 = dict()
    for i in range(16):
        if dic1.has_key(i):
            dic2[dic1[i]['origin']] = 16-dic1[i]['perm'] 


    # print input_starting_off         

    # loop through index (this is output index)
    for index in range(16):
        value = dic1.get(index,None)

        sp = ' '
        if index > 9:
            sp = ''

        if (value):
            sys.stdout.write(prefix + "[" + str(index) + sp + \
                "] = (Schedule) {SRC_ROW_REV+%d, 0x%x, " % (value['row'], output_stride))
        else:
            sys.stdout.write(prefix + "[" + str(index) + sp + \
                "] = (Schedule) {SRC_ROW_REV+%d, 0x%x, " \
                % (input_starting_off[index], input_stride))

        
        value2 = dic2.get(index,None)
        if value2:
            sys.stdout.write("(0x%x << 12) | (DST_ROW_REV + %d), 0x%x};\n" % (value2, input_starting_off[index],input_stride))
        else:
            sys.stdout.write("(GARBAGE_ROW), 0x0}\n")


def print_forward_vmem(dic1, input_stride, output_stride, prefix=''):
    input_starting_off = calculate_input_start(dic1)

    print(prefix + " = (VmemSchedule) {")
    print "{",
    for index in range(16):
        value = dic1.get(index,None)
        c = ',' if index < 15 else ''
        print "MOVER_SRC_ROW+%d%s" % (input_starting_off[index],c),
    print "},"

    print "{",
    for index in range(16):
        value = dic1.get(index,None)
        c = ',' if index < 15 else ''
        print "%d%s" % (input_stride,c),
    print "},"

    print "{",
    for index in range(16):
        value = dic1.get(index,None)
        c = ',' if index < 15 else ''
        if value:
            print "(0x%x << 12) | (DST_ROW + %d)%s" % (value['perm'],value['row'],c),
        else:
            print "(0 << 12) | (GARBAGE_ROW)%s" % (c),

    print "},"

    print "{",
    for index in range(16):
        value = dic1.get(index,None)
        c = ',' if index < 15 else ''
        print "0x%x%s" % (output_stride,c),
    print "}"
    print "};"


def print_reverse_vmem(dic1, input_stride, output_stride, prefix='', dst_postfix=''):
    input_starting_off = calculate_input_start(dic1)

    dic2 = dict()
    for i in range(16):
        if dic1.has_key(i):
            dic2[dic1[i]['origin']] = 16-dic1[i]['perm'] 

    print(prefix + " = (VmemSchedule) {")
    print "{",
    for index in range(16):
        c = ',' if index < 15 else ''
        if index in dic1:
            value = dic1.get(index,None)
            print "SRC_ROW_REV+%d%s" % (value['row'],c),
        else:
            print "SRC_ROW_REV+%d%s" % (0,c),
    print "},"

    print "{",
    for index in range(16):
        value = dic1.get(index,None)
        c = ',' if index < 15 else ''
        print "%d%s" % (output_stride,c),
    print "},"

    print "{",
    for index in range(16):
        value = dic1.get(index,None)
        value2 = dic2.get(index,None)
        c = ',' if index < 15 else ''
        if value2:
            print "(0x%x << 12) | (DST_ROW_REV%s + %d)%s" \
                % (value2, dst_postfix, input_starting_off[index], c),
        else:
            print "(GARBAGE_ROW)%s" % c,
    print "},"

    print "{",
    for index in range(16):
        value = dic1.get(index,None)
        c = ',' if index < 15 else ''
        print "0x%x%s" % (input_stride,c),
    print "}"
    print "};"


def print_common_stuff(header, input_stride, output_stride, var_postfix):
    if var_postfix is None:
        postfix = ""
    else:
        postfix = var_postfix
    print "//////////////////////////////////////////////////////////////////////////////////////////////////"
    print "//"
    print "//    ", header, " Schedule"
    print "//"
    print "//////////////////////////////////////////////////////////////////////////////////////////////////"
    print "//"
    print "// Total Subcarriers: %s" % subcarriers
    print "// input_stride =", input_stride
    print "// output_stride =", output_stride
    print "//"
    print "// global constants:"
    print "enabled_subcarriers" + postfix + " = " + str(len(subcarriers)) + ";"
    print "number_active_schedules" + postfix + " = " + str(schedules_required) + ";"
    print ""

# sorts a list of numbers by which column they fall into, assumes NREGS=16
def sort_by_column(llist):
    n = []
    # my_list.sort(key=lambda x: x[1])

    # make a list of tuples where the first element is the row it falls in, and the 2nd element is the number
    for x in llist:
        n.append( (x%16,x) )

    # Python automatically sorts a list of tuples by the first elements in the tuples, then by the second elements and so on...
    n.sort()

    # strip out first tuple value, so only the original number is left, but in the sorted order
    out = [x[1] for x in n]

    return out

# [497+(16 + 1) * x for x in range(16)]
# etc

# subcarriers = [0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255]
# subcarriers += [271, 286, 301, 316, 331, 346, 361, 376, 391, 406, 421, 436, 451, 466, 481, 496]
# subcarriers += [512, 529, 546, 563, 580, 597, 614, 631, 648, 665, 682, 699, 716, 733, 750, 767]
# subcarriers += [768, 785, 802, 819, 836, 853, 870, 887, 904, 921, 938, 955, 972, 989, 1006, 1023]

#subcarriers = [0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 254]

# subcarriers = [2, 3, 4, 14, 16, 18, 21, 27, 28, 29, 30, 33, 34, 36, 38, 39, 43, 45, 47, 48, 49, 52, 53, 56, 57, 60, 65, 66, 67, 70, 73, 75, 77, 84, 85, 87, 91, 92, 97, 99, 102, 104, 106, 107, 109, 110, 113, 115, 120, 121, 123, 124, 127, 128, 133, 136, 138, 140, 142, 145, 147, 148, 153, 155, 157, 158, 161, 162, 163, 164, 168, 169, 171, 172, 174, 176, 177, 181, 186, 189, 190, 192, 193, 195, 196, 200, 202, 204, 205, 206, 207, 210, 211, 214, 215, 219, 220, 225, 227, 229, 232, 236, 239, 241, 242, 243, 244, 246, 247, 249, 250, 252, 256, 257, 259, 260, 261, 264, 269, 271, 272, 275, 280, 283, 284, 287, 289, 290, 292, 294, 296, 297, 298, 299, 305, 306, 313, 314, 315, 320, 324, 326, 331, 335, 336, 337, 338, 339, 341, 344, 345, 355, 356, 366, 371, 378, 379, 380, 381, 387, 388, 389, 392, 394, 396, 397, 401, 405, 406, 408, 410, 411, 413, 418, 420, 423, 424, 425, 427, 428, 431, 432, 433, 434, 436, 439, 441, 447, 448, 455, 457, 458, 460, 461, 462, 470, 473, 474, 475, 480, 481, 487, 490, 491, 492, 500, 501, 503, 506, 508, 511, 517, 518, 523, 524, 529, 530, 532, 537, 541, 542, 553, 554, 557, 560, 564, 569, 570, 573, 577, 578, 582, 585, 589, 595, 596, 598, 599, 601, 602, 604, 606, 609, 610, 611, 615, 619, 620, 621, 622, 624, 627, 628, 630, 631, 633, 636, 644, 647, 650, 651, 655, 656, 658, 659, 669, 671, 672, 673, 674, 676, 678, 679, 680, 681, 682, 685, 686, 689, 695, 697, 698, 701, 706, 707, 709, 711, 712, 713, 714, 716, 722, 724, 725, 727, 728, 733, 739, 744, 746, 747, 749, 754, 755, 757, 762, 765, 766, 769, 770, 774, 780, 781, 783, 786, 790, 791, 792, 796, 797, 798, 801, 808, 810, 811, 813, 825, 826, 827, 828, 829, 834, 844, 845, 848, 849, 851, 852, 856, 859, 861, 864, 867, 871, 872, 875, 876, 882, 886, 887, 888, 891, 894, 895, 896, 899, 900, 903, 904, 905, 910, 911, 912, 914, 916, 918, 924, 930, 931, 933, 934, 935, 943, 946, 947, 948, 949, 956, 957, 959, 961, 962, 964, 965, 966, 968, 969, 971, 972, 974, 976, 980, 981, 982, 984, 988, 995, 996, 997, 1007]

# subcarriers = [16, 48, 128, 176, 192, 256, 272, 320, 336, 432, 448, 480, 560, 624, 656, 672, 848, 864, 896, 912, 976, 33, 49, 65, 97, 113, 145, 161, 177, 193, 225, 241, 257, 289, 305, 337, 401, 433, 481, 529, 577, 609, 673, 689, 2, 18, 34, 66, 162, 210, 242, 290, 306, 338, 418, 434, 530, 578, 610, 658, 674, 706, 722, 754, 770, 786, 834, 882, 914, 3, 67, 99, 115, 147, 163, 195, 211, 227, 243, 259, 275, 339, 355, 371, 387, 595, 611, 627, 659, 707, 739, 755, 851, 867, 4, 36, 52, 84, 148, 164, 196, 244, 260, 292, 324, 356, 388, 420, 436, 500, 532, 564, 596, 628, 644, 676, 724, 852, 900, 21, 53, 85, 133, 181, 229, 261, 341, 389, 405, 501, 517, 709, 725, 757, 933, 949, 965, 981, 997, 38, 70, 102, 214, 246, 294, 326, 406, 470, 518, 582, 598, 630, 678, 774, 790, 886, 918, 934, 966, 982, 39, 87, 215, 247, 423, 439, 455, 487, 503, 599, 615, 631, 647, 679, 695, 711, 727, 791, 871, 887, 903, 935, 56, 104, 120, 136, 168, 200, 232, 264, 280, 296, 344, 392, 408, 424, 680, 712, 728, 744, 792, 808, 856, 872, 888, 904, 968, 57, 73, 121, 153, 169, 249, 297, 313, 345, 425, 441, 457, 473, 537, 553, 569, 585, 601, 633, 681, 697, 713, 825, 905, 969, 106, 138, 186, 202, 250, 298, 314, 378, 394, 410, 458, 474, 490, 506, 554, 570, 602, 650, 682, 698, 714, 746, 762, 810, 826, 27, 43, 75, 91, 107, 123, 155, 171, 219, 283, 299, 315, 331, 379, 411, 427, 475, 491, 523, 619, 651, 747, 811, 827, 859, 28, 60, 92, 124, 140, 172, 204, 220, 236, 252, 284, 380, 396, 428, 460, 492, 508, 524, 604, 620, 636, 716, 780, 796, 828, 29, 45, 77, 109, 157, 189, 205, 269, 381, 397, 413, 461, 541, 557, 573, 589, 621, 669, 685, 701, 733, 749, 765, 781, 14, 30, 110, 142, 158, 174, 190, 206, 366, 462, 542, 606, 622, 686, 766, 798, 894, 910, 974, 47, 127, 207, 239, 271, 287, 335, 431, 447, 511, 655, 671, 783, 895, 911, 943, 959, 1007]

# subcarriers = [16, 33, 2, 3, 4, 21, 38, 39, 56, 57, 106, 27, 28, 29, 14, 47] + [48, 49, 18, 67, 36, 53, 70, 87, 104, 73, 138, 43, 60, 45, 30, 127] + [128, 65, 34, 99, 52, 85, 102, 215, 120, 121, 186, 75, 92, 77, 110, 207] + [176, 97, 66, 115, 84, 133, 214, 247, 136, 153, 202, 91, 124, 109, 142, 239] + [192, 113, 162, 147, 148, 181, 246, 423, 168, 169, 250, 107, 140, 157, 158, 271] + [256, 145, 210, 163, 164, 229, 294, 439, 200, 249, 298, 123, 172, 189, 174, 287] + [272, 161, 242, 195, 196, 261, 326, 455, 232, 297, 314, 155, 204, 205, 190, 335] + [320, 177, 290, 211, 244, 341, 406, 487, 264, 313, 378, 171, 220, 269, 206, 431]
# subcarriers = [16, 33, 2, 3, 4, 21, 38, 39, 56, 57, 10, 27, 28, 29, 14, 47] + [100, 116, 101, 117, 102, 118, 103, 119, 104, 120, 105, 121, 106, 122, 107, 132]


# was using this
# subcarriers = range(16,128+16)+range(1024-16-128,1024-16)

# if forward is range(16,64+16)
# subcarriers = range(16,16+128)
#subcarriers = range(16,16+128) + range(1024-16-128, 1024-16)
# subcarriers = range(1024-146,1024-130)
# reverse would be range(1024-16-64,1024-16)
#subcarriers = range(1024-16-64,1024-16)
# subcarriers = range(19,19+32,2)


use_vmem_schedule = True

output_forward_schedule = False

# Variables here allow "cohabitation" between multiple mover schedules
# the existing, and fixed, mover schedule is the one we use for display data
# the one we are changing and adding is the one for sliced data

global_var_postfix = "_data" # set to "" for empty string
dst_row_postfix = "2"  # set to "" for empty string
array_index_postfix = "+4" # set to "" for empty string




# previous, before I understood the difference between zhen and my mapping
# subcarriers = [17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99, 101, 103, 105, 107, 109, 111, 113, 115, 117, 119, 121, 123, 125, 127, 129, 131, 133, 135, 137, 139, 141, 143, 881, 883, 885, 887, 889, 891, 893, 895, 897, 899, 901, 903, 905, 907, 909, 911, 913, 915, 917, 919, 921, 923, 925, 927, 929, 931, 933, 935, 937, 939, 941, 943, 945, 947, 949, 951, 953, 955, 957, 959, 961, 963, 965, 967, 969, 971, 973, 975, 977, 979, 981, 983, 985, 987, 989, 991, 993, 995, 997, 999, 1001, 1003, 1005, 1007]

# this was used before and after the shutdown. this corresponds to MAPMOV_MODE_128_CENTERED
# subcarriers = [1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95,97,99,101,103,105,107,109,111,113,115,117,119,121,123,125,127,897,899,901,903,905,907,909,911,913,915,917,919,921,923,925,927,929,931,933,935,937,939,941,943,945,947,949,951,953,955,957,959,961,963,965,967,969,971,973,975,977,979,981,983,985,987,989,991,993,995,997,999,1001,1003,1005,1007,1009,1011,1013,1015,1017,1019,1021,1023]

# centered 400
subcarriers = range(1,320,2) + range(705,1024,2)











# subcarriers = [122, 111, 133, 144, 866, 877, 888, 899, 900, 901, 902, 903, 904, 905, 906, 907]

# subcarriers = range(0, 32)

print ""
print "// Total Subcarrier #: %s" % len(subcarriers)
print ""
assert len(subcarriers) != 0, "Number of enabled subcarriers must not be 0"
assert len(subcarriers) % 16 == 0, "Number of enabled subcarriers must be multiple of 16"
assert len(subcarriers) == len(set(subcarriers)), "You cannot have duplicate subcarriers numbers in the input"

print "\n\n\n\n\n\n"



# required
subcarriers = sorted(subcarriers)


conflicts_removed = bin_subcarriers(subcarriers)

# print conflicts_removed
# sys.exit(0)


# prune = list(subcarriers)  # copy constructor

# a list of duples that have (dictionary, conflicts)
tuples = []

i = 0

# index of input words that we've most recently mapped
last_mapped_index = 0

for i, chunk in enumerate(conflicts_removed):
    # print "// Run:", i

    # rotation = (16-(last_mapped_index % 16))%16
    rotation = last_mapped_index % 16

    # get the results into tmp_ and add to tuples
    tmp_dic, tmp_conf = build_dic(chunk, rotation, last_mapped_index)

    last_mapped_index += len(tmp_dic)

    # TODO: consider conflicts list, add additional schedules

    tuples.append((tmp_dic, tmp_conf, chunk))
    i += 1

schedules_required = len(tuples)

input_stride = len(subcarriers)/16
output_stride = 64 # I think only change this if we decide to violate minimum of 16 subcarriers, unsure


print ""
print ""
print ""
print ""
print ""
print ""
print ""
print ""
print ""
print ""
print ""

if output_forward_schedule:

    print_common_stuff("Forward", input_stride, output_stride, global_var_postfix)
    
    for i,(dic, conflicts, chunk) in enumerate(tuples):
    
        # input_starting_offset = i # offset added to source row
        
        print "// Forward for chunk("+str(i)+"):", chunk
        print "//                   "+" "*len(str(i))+"  ", sort_by_column(chunk)
    
        if use_vmem_schedule:
            prefix = 'vmem_schedules[' + str(i) + ']'
            print_forward_vmem(dic, input_stride, output_stride, prefix)
        else:
            prefix = 'schedules[' + str(i) + ']'
    
            print_forward(dic, input_stride, output_stride, prefix)
            print ""
    
    
    print ""
    print ""
    print ""
    print ""
    print ""
    print ""
    print ""
    print ""

print_common_stuff("Reverse", input_stride, output_stride, global_var_postfix)

for i,(dic, conflicts, chunk) in enumerate(tuples):

    # input_starting_offset = i # offset added to source row
    
    print "// Reverse for chunk("+str(i)+"):", chunk
    print "//                   "+" "*len(str(i))+"  ", sort_by_column(chunk)

    if use_vmem_schedule:
        prefix = 'vmem_schedules[' + str(i) + array_index_postfix + ']'
        print_reverse_vmem(dic, input_stride, output_stride, prefix, dst_row_postfix)
    else:
        prefix = 'schedules[' + str(i) + ']'

        print_reverse(dic, input_stride, output_stride, prefix)
        print ""

print ""
print ""
print ""
print ""
print ""

# print_forward(dic, input_stride, output_stride, prefix)




# for key, value in dic1.iteritems():

# if (len(conf1)>0):
#   print "Conflict schedule"
#   for index in range(16):
#       value = conf1.get(index,None)
#       if (value):
#           print ("(Schedule) {MOVER_SRC_ROW, 0x%x, (0x%x << 12) | (DST_ROW + %d), 0x%x}, //Column %d" % (input_stride, value[1],value[2],output_stride,value[3]))     
#       else:
#           print ("(Schedule) {MOVER_SRC_ROW, 0x0, (0 << 12) | (GARBAGE_ROW), 0x0} //Column %d" % (index))

# print("Reverse")
# conf2 = dict()
# dic2 = dict()
# for i, item in enumerate(sorted(subcarriers)):
#     column = item%16
#     row = item/16
#     prem = column -i+ rotation

#     if (prem < 0):
#         prem = 16+prem

#     if (prem >= 16):
#         prem = prem-16

#     dic2[i+ rotation] = (item, prem, row, column)

# for index in range(16):
#     value = dic1.get(index,None)
#     if (value):
#         sys.stdout.write("(Schedule) {(DST_ROW + %d), 0x%x," % (value[2],output_stride))       
#     else:
#         sys.stdout.write("(Schedule) {(GARBAGE_ROW), 0x0,")

#     value = dic2.get(index,None)
#     if (value):
#         sys.stdout.write("((0x%x << 12) |MOVER_SRC_ROW), 0x%x}, \n" % (value[1],input_stride))
#     else:
#         sys.stdout.write("(GARBAGE_ROW), 0x0}, \n")