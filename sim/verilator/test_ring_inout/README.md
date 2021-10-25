# Purpose

Copied from test_ring_simple.  This is a limited example which exploits a timing bug during input / output of ringbus.  This project overrides ETH even though in general we shouldn't be doing that.

# Test
* Every fpga sends a ring at boot
* CS30 sends packets frequently until 35us
* TB sends 3 messages at 10 us spacing
  * Each FPGA should respond to these 3 messages
  * They do respond, however the `0x300a3` message is dropped at eth and queues a faulty packet that rips through ringbus for 255 iterations.

# Eth FPGA
Is used, don't take out

# Results
This bug is under `q-engine`
* fixed in `be70a732d57ba65697f2b651d4ba5e822b0260d3` 
* Discovered in `c1ddf48a422e72b6328e1dc31d947efa75dc8e63`
