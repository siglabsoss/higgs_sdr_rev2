# Project

A playground to develop a "fill level" where the board sends results back to the PC


# Structure
3 circular buffers run.  the 2nd cirbuf tracks a large chunk of vmem that is statically allocated.  the fill level is taken from how much is written here

# Notes
* usese old fft, but this is a playground