# Purpose

Verify we can dump imem.  tries various non aligned with 4 offsets.


# Flow
We use a struct with `__attribute__((__packed__))` to force non multiple of 4 byte alignment.  The main reason we are so specific about this is that the verilator access to imem is word based.  This means that higgs_helper needs to the unpacking/shifting/packing correctly.