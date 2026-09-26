# Test
Test if some of the vmem operators work correctly.


# Eth Override
We abuse verilator vs bitfiles using eth.  In bitfiles eth has no datapath, but in verilator it does

# Eth Mem Bug
Looks like eth has has 4096 rows of memory ALL ALONG. see https://github.com/siglabs/higgs_sdr_rev2/issues/109

