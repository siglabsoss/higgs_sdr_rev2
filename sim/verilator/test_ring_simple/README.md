# Purpose

Test ring in both directions

# Test

* Each FPGA sends a message at boot
* TB sends messages at a spacing of 9 or 10 us
  * Each FPGA replies
  * TB checks for replies

# Note
If TB sends at spacing of 10us, this causes a bug which was found and put in a bottle in the `test_ring_inout` test.
