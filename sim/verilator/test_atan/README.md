# Purpose

Trying to stream out of CS00 to the PC. Also developing s-modem alongside

# Additions

CS00 adds:
* ability to enable / disable output stream (without losing 1024 sync)
* ability to advance input
* Zhen's coarse sync code

# Flow
* can control adc counter.

# FPGAs
* CS00
* CS01 - reverse mover, using schedule

# Notes
I actually just put in the ADC so it would dump the counter for me.  But 
* The plot examples below require `python-osi` to be checked-out along side higgs_sdr

# Inputs
Secondary edits in this test have added `sc_16_144_880_1008_bpsk.hex ` which has 256 subcarriers on, each one transmitting it's own subcarrier # in bpsk followed by some zeros.

To plot and demod, for instance, run the test (cs00 will sync and fft.) and then something like:

* `python ../../../scripts/plot_file.py -hex ./cs00_out.hex -scr 22 -rotd 120 `
* `python ../../../scripts/demod_file.py -hex ./cs00_out.hex -scr 22 -rotd 120 -cut 2`


# Example
The output of above might look like:
```bash
subcarrier 22
[0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0]
22 0x16
22 0x16
22 0x16
22 0x16
```


# Flushing notes
I decided to have the flushing state only affect the input DMA
## cons
* this means in progress FFT will still trigger a new output dma even in flushing state.
* Longer time before sync starts (because of previous)
## pros
* Easier to code, less lines to change
* when we come out of flush, only need to reset input dma