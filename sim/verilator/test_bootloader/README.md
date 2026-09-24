# Test
Test the bootloader.  Only a very tiny partial program is loaded.  A full program would take too long.

# Flow
* CS20 was built and finalzied
  * Has ringbus callback to directly read imem and send back to tb
* After finalized and compiled, first 17 instructions were copied to tb.cpp
* We then modifiy one of these instructions which we know is never executed by riscv
  * we write 0xdeadbeef to this location `[7]`
  * we bootload
  * then after bootload we send a ringbus which causes riscv to readback that imem instruction
  * if we get the ringbus we know the partial bootload did not crash
    * if we get the correct instruction we know the bootload was successful
  * we write 0xfeedbabe to the same location
  * we bootload
  * we check the instruction again
