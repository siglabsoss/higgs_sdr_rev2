# Purpose
Test new duplex schedule modulation and demodulation.  This test contains an entire full tx-ifft-ifft-rx chain for the sliced data.

# Setup
* We boot self sync
* We set the lifetime_32 counter
* We Modulate N words into cs11 using feedback bus, at a target time (measured against lifetime_32)
* cs12 and cs02 perform roles of a tx and rx radio, resulting in sliced feedback bus data
* testbench compares output with predicted output from emulateHiggsToHiggs()


# Commentary
* We skip an entire compliated code by skipping the Demodulate step.  Instead we simply modulate once with riscv and once with s-modem AirPacket and compare.  This makes life a lot more simple
* We are developing this before merging janson so we can simply cut off the 640 header.


# FPGAs
* cs11 untouched
* cs12 simulate fft in out  (doubles output to copy q-engine)
* cs02 - copied from cs20 (mover mapper)


# Problems
(sovled)Currently cs11 is only outputting one frame 43 but cs12 is picking up 2
* Seems like setting lifetime_32 adds a glitch in frame phase, but this causes it to output the same frames 5 times.  so weird. (Maybe the glitch causes us to overwrite memory that is already queued for output, causing it to go out twice)




# Tricky
* This was very hard to get working.  ifft(ifft()) was not working, so I simulate in/out with a reverse DMA.
* The input that cs20 requires has d-engine ahead of it, forgot to consider that


# Source
Copied from test_duplex_1