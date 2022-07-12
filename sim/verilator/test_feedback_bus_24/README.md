# Purpose

Putting the c++ parser for consuming feedback bus into higgs_helper.

Comparing results against old work done in test_feedback_bus_2.

`registerRawFbCb` and `registerFbCb` are tested

# FPGA

## CS20
Flow:
* Picks a random number and dma's out the header and body packets corresponding
* TB parses.  One verification is if TB gets out of sync and starts parsing illegal values
  ** (there is no secondary comunication to verify if received packets are same as send)

# Test
* The random frames are parsed by higgs_helper.  This is done each microsecond as the test runs.
* The frames are parsed a second time using the existing method (copy pasta per test)
* The hashes of the resulting frames are compared