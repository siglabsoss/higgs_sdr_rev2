# Test
Old / first ringbus test.  Has good ideas for testbench, but coverage of verilated verilog doesn't work?

# Flow
* has a global_ring
* edits `posClock()` in `tb.cpp` which NO OTHER TEST DOES
* tries to manually send ringbus commands with bit vectors

# Notes:
* Has an unfinished idea for running more than one test in a tb.cpp
* Directly manipulates `top->cs20_i_ringbus`
