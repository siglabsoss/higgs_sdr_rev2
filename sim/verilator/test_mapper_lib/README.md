# Purpose
A unit test for the mapper library.  Written to double check K13/K15

# Function
Copied from `test_mover_lib`.  The mapper is tested in QPSK mode.  A counter starting at `0x192d3afa` is fed from the test bench.  The mover IS assumed to be working here, but it's not the focus of this test (again see `test_mover_lib` test).

The schedule for the relied upon mover is is `[0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255]`.  The testbench writes `cs20_out.hex` to disk.  After the test runs, `consider_subcarriers.py` looks at the output file, and will exit code with pass fail.
