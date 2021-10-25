# Purpose

Similar to test_uart_frame_3 (see readme).  We dump 8 large vmem arrays.  This makes sure to touch all values of vmem.  These large vmem arrays are all pre-allocated at compile time.

# Flow
In addition for one of the dumps. we dump and then immediatly change the value.  The purpose of this is to try and find if block_until_dump_done() blocks long enough for testbench to grab data.

# Size
Here we also set the size to 65536.  In order to fit this value (0x10000) into 2 bytes. we actually subtract 1 from length before we send, as a length of 0 is invalid anyways.  So we dump all vmem bytes and verifies that we can represent this large number in 2 bytes
