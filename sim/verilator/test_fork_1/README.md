# Purpose
This test proves and tests that forking works for a 2 Board setup.

# Forking Flow
* We are trying to wire the output of cs01 to the input of cs31
* We fork the process twice and use zmq between the two children.  At each tick (high and low) we send a zmq message with information about the valid/ready/data lines
* After we are done, the two children send their exit code to the parent over zmq


# Test Flow 
* We use `test_dma_torture.h`
  * Higgs 0 cs01 sends to Higgs 1 cs31
  * Higgs 1 cs01 sends to Higgs 0 cs31
* We use `tb_inject_mem.h` to inject a different random seed to each Higgs board (each fpga gets the same seed)
* We use `get_higgs_id()` so each cs01/cs31 pair is using the same `dma_torture_output_type`, `dma_torture_input_type`
  * `dma_torture_output_type`, `dma_torture_input_type` are a new feature of `test_dma_torture.h` which allows for two different vectors to be sent / received
* Because of this we can be sure no cross-talk is occuring
