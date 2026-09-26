import sys
# sys.path.insert(0,"../../../../python-osi")
# sys.path.insert(0,"../../../scripts")
# from sigmath import *
# from subcarrier_math import *

# returns non 0 for failure
def verify_counter(words):

    llen = len(words)
    bits = []

    counters = words

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

    print "All", lenc, "counters from", hex(start_range), "to", hex(end_range-1), "were correct"


def main():

    files = ["cs31_out.hex","cs32_out.hex","cs22_out.hex","cs21_out.hex","cs11_out.hex","cs12_out.hex", "cs02_out.hex"]

    for filepath in files:

        print "Considering", filepath

        lines = []

        with open(filepath) as bootprogram:
            lines = bootprogram.readlines()

        words = [int(l,16) for l in lines]

        llen = len(words)

        print("got", len(words))

        verify_counter(words)

        assert llen == 4096


if __name__ == '__main__':
    main()


