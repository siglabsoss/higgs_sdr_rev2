# Verilator Folder
This folder has 50+ subfolders which each contain an entire set of `main.c` programs for each FPGA on a Higgs (see `override/fpgas`).
* Almost all of the folders have a "test" of some kind.
* Some are "self testing" and are run by jenkins with every build.
* Some can be booloaded onto the board.


# Good Examples To start with
A list of a few folders you can copy to make what you want.
* `test_mover_4` - Example of dumping every fpga in/out dma to disk.
* `test_eth_ring` - Each fpga sends to eachother / tb.  
* `test_vmalloc` - Example of using a single `CS30` to run tests on a library.  Results   of running multiple subtests, building a single flags/bitfield to send back via rb, and then catching from tb and failing against flags.
* `test_fft_lib_1` - Example of running Zhen's FFT library.
* `test_mover_4` - Example of fft data on receive side, and picking out subcarriers
* `test_tx` - Precalculated data fed into fft
* `test_tx2` - BPSK mapped / moved into subcarriers and fed into fft
* `test_dma_fft` - Example of running a long test, but only tracing the end / interesting parts

# Higgs Helper
This is a common class which each test includes to run the inputs and outputs.  Currently messsy and needs getters/setters.  If we collapse a lot of common code, test benches would be easier to write

# Self testing
Tests that are self testing, have code that runs some sort of a unit test.  This unit test can be 100% inside vex (`test_vmalloc` `test_cir_buf`).  Or these unit tests can accept data from the testbench, process it, and then return it some how.

## Overview of what's happening inside a Self Test
* The code should send a pass / fail ringbus, or raw data to the testbench.
* The tesbench should compute if needed, asserting on values that should match
* Testbench should return 0 exit code to indicate success.  non-zero exit code is treated as failure.


# Where to start writing a test
How to make a test.  You can also follow this procedure to write a sandbox playground area.
* Pick a good folder to copy from
* Decide which FPGA's do you want to enable?
  * if test is mostly tb->dut than consider cs20 only
  * if test is mostly dut->tb than consider cs30 only
* Edit the `Makefile` in your folder.  Consider:
  * `OVERRIDE_CS20_C` - set if we have code to run
  * `CS20_NO_RISCV` - set if you want to keep RB but remove fpga (for speed)
  * `TB_USE_CS20`   - set if you want to yank entire FPGA (for speed) (will break RB)
* Edit 1README.md1 and put in general what it does
* For every given FPGA you want to use you must have an `override/` setup
  * Note: Some tests have folders deleted (if you were to copy them you many need to add skeleton folder)
* Edit tb.cpp and decide how long you want the test to run for
  * Either dump or inject data into ringbus.
  * In test-bench, loop over data.
  * Return an intelligent exit code
* Edit `Makefile` and find the `test` target.  Add python or anything else if you want to incorporate those tools into your test.
  * Most tests do not do this and rely on tb exit code alone, but we should be using python to test where it makes sense.
 

# Running your test
General flow of running things: 
* `make`
  * Run make the first time.
  * Must be re-done with any verilog changes
* `make quick`
  * For changes to `tb.cpp` or `main.c` (but not Makefile or Verilog)
* `make show`
  * Open GTKwave.
* `make quickt`
  * Run quickly, and do not dump `VCD` file.  With this option `make show` will not work, OR WILL SHOW STALE DATA

# Jenkins
By default tests in this folder are NOT run by jenkins.  To add a test to Jenkins, edit the `Makefile` in this folder
* Add a test section
* Add a clean section
That's it.

## Order
Please sort the tests you add to Jenkins:
* Put "sure fire" or easy to pass tests first
* Put short test first

# Updates for  "make show"  command
Good news! We can save/load GTK Wave gui's instead of hand editing the `.tcl` file!
* Instructions:
  * copy `test_mover_4/dma_out.gtkw` to your project
  * Edit your Makefile to copy `test_mover_4/Makefile`
  * run `make show`
* Now you can edit the gui, press ctrl+s and your settings will save
* Don't forget to commit your changes to `dma_out.gtkw`

# Verify folders
As a general rule, folders that start with `verify_` are meant to bootload onto the board and perform some sort of (admitidly human-in-the-loop test).  The more "hands off" these tests can be, the eaiser.  New bitfiles should be able to pass these:
* `verify_edge` - edge to edge test
* `verify_fft_lib_opt`
* `test_eth`
* `test_ring`
* `test_rx_chain`


# Verify Script
Please add steps:
* Instructions should cover which commands to run or a single script.  Should be readable by a lab-tech and will be followed during mass production.
* Clear pass fail conditions
