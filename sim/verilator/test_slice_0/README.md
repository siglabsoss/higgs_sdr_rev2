# Test Multiple XBB Commands


# run test

`make test`

# Code
See these files:

* tb.cpp
* override/fpgas/grav/eth/c/src/main.c

# History

This test started out with code from zhen's branch. but it turns out this code has to tricks:
* When compiled with O3 presents a memory hazard of reading vector memory directly after `VNOP_SK15`
* When compiled with O3 presents a vector_memory bug when doing quick READS only, not writes.

# Results
Test gives a few different results codes due to the above issues.  Enable or disable `DMEM_RSP_READY_FIX` for effect.
* Expected results bits: `0x7f7`
* with O3 and fast_dbus bug without patch: `0x417`
* With patch: `0x7f7`

