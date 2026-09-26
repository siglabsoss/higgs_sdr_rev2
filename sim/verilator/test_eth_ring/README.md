# Description
Rather complicated test where each fpga has a complicated set of ringbus instructions it can respond to.

# Commands
Sending a ringbus command to a FPGA can do one of:
* Increment, decrement, or add a 5 bit value to either of two counters
* Generate a number of ringbus messages to be sent
* Dump counters out to the testbench
* Seed a random value to be used during rb message generation

# Command Structure
The 32 bit ringbus command is broken into these fields:
* src
* dsc
* field1 (op code)
* field2
* field3
* field4


# Command: Set Seed
* field1 (op code)
* field2,3,4 (seed broken into 5 bit chunks)



# Files
* See the `riscv-baseband/c/inc/unit_test_ring.h` file
  * This file uses `VERILATE_TESTBENCH` which is set for the tb when including, but not set for riscv.  This allows the same file to be included from both riscv and tb so shared includes can be read


# Flow
* The test bench creates a ringbus using `gen_type_2(target)`
  * This ringbus is sent to a specific fpga
  * Upon receiving, the FPGA will send `15` ringbus packets to the `target` with random delays between packets.
  * The `target` fpga will receive the `15` packets, and incerement a counter each time one is received
* The testbench reads the counters out from all fpgas, and asserts that some are 15 and some are 0
