# Purpose
A unit test for the mover library.  Written to double check K13/K15

# Function
A modified version of the normal mapper / mover (as of 5/21/2018) is used.  The mapper is disabled and not tested.  Instead of mapped values, counter values starting at `0xf0000000` are fed into the mover.

The schedule is `[0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255]`.  The testbench writes `cs20_out.hex` to disk.  After the test runs, `consider_subcarriers.py` looks at the output file, and will exit code with pass fail.
