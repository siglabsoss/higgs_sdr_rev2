# Purpose
Each FPGA has a compile time seed (how to fix?). (upgrade to take pull from rb?)
Each FPGA pulls on random seed, and schedules random amounts of sending and getting a 4k counter


# Flow
* Each fpga starts sending and queueing small dma segments of random size
* After done, receiving FPGA does a liner check on all 4k values
* Each FPGA announces a pass and tb checks this
* `make test` will run some python after tb exits to verify tb wrote all values to disk
  * this acts as a dual test, failing when a FPGA skips some output values
  * or when tb messes up and can't write them to disk


# Todo
Test this out on hw and or write instructions for running it.  Also add a random seed so we can have long hw test runs.

# Files

See:
* `test_dma_torture.h`
* `consider_counters.py`

# Runtime
1700 us.


# Jenkins
Yes