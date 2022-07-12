# Purpose

Copied from test_feedback_bus_2.  This was the first time I connected higgs up to s-modem
and had it consume DMA's from cs30

# Files
See:
* `feedback_bus.h`   - `typedef struct` types for different feedback bus objects
* `feedback_bus_tb.hpp`  -  make std::vector<> of words for shifting into verilated tb


# FPGA

## CS30
Flow:
* seeds a fixed random number and starts
* s-modem parses
  ** (sent to a peer s-modem if ok)

# Test
* Run with random seed and check paring functions for illegal values