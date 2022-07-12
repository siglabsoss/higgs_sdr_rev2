# Purpose

Developing c++ parser for consuming feedback bus packets exiting vex.

# Files
See:
* `feedback_bus.h`   - `typedef struct` types for different feedback bus objects
* `feedback_bus_tb.hpp`  -  make std::vector<> of words for shifting into verilated tb


# FPGA

## CS30
Flow:
* Picks a random number and dma's out the header and body packets corresponding
* TB parses.  The only verification is if TB gets out of sync and starts parsing illegal values
  ** (there is no secondary comunication to verify if received packets are same as send)

# Test
* Run with random seed and check paring functions for illegal values