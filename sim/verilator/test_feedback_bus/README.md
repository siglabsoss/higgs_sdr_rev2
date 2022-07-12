# Purpose

Developing ground for feedback bus.  the biggest challenge is the code on cs20 that parses input dma and needs to deal with different input sizes.

# Files
See:
* `feedback_bus.h`   - `typedef struct` types for different feedback bus objects
* `feedback_bus_tb.hpp`  -  make std::vector<> of words for shifting into verilated tb


# FPGA

## TX0
Flow:
* reads first 16 bytes as header from pet_dma_in
* `pet_parse()` decides how many bytes to read next (the body)
* After the body is queued, the next header is allowed to be scheduled
* After the body had arrived, `pet_parse()` uses `dma_in_extra_signal` to determine this, and then parse the body


# Test
Testbench generated one of 3 random types of vector message to send.  Each type of vector message has a known execution time, to tb will wait that amount of time before sending next


Flow:
* pull random number
* decide a packet type to send
  * pick a random number for specific packet
* both the type and specific random number are appended to `types_pulls`
* `next_pull` is the next time we will choose to send a random packet, this is added (in a separate switch())
* the tb verifies rb output (fixme)
* the test finishes and asks cs10 to flush rb
* the tb verifies cs10's output which allows tb to see what kind of rb messages that cs20 sent it