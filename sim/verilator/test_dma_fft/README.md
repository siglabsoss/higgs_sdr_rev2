# Purpose
Found a random bug where input dma affects output dma even though they are writing to different addresses.  tests can pick a time based random seed, or a fixed seed.  Input from eth is randomized.  We only dump the vcd file in the affected time range.

# Instructions
Run this to show the error:
* `make` or `make quick`
* Test bench will say `Breaking early at 1952040 (976000)`
* `make show` to see the truncated wave file

# Notes
* CS20 sends mapped/moved data to CS10
* CS10 is made of a hacked ping-pong streaming fft code, which sends an ringbus msg with value of `0xc000001` which signals an error.
   * it does this by a modified `pet_fft()` which is looking for the specific counter at the specific place
* The output data from CS20 should be a fft with data on channels `111, 122, 133, 144, 866, 877, 888, 899`
  * This means that the output is in chunks of 1024, where every index above is non-zero

# The Issue
Look at cs20_out.hex: 
* on line `111728`  we find our pattern,  `(111728-1) % 1024` is `111`
* on line `111728 + 1024` we would expect to find the same pattern, but it has shifted

# Tracing
This test is an example of partial vcd partial wave dump partial gtk wave dumps.