# Purpose

Trying to track down why subcarrier #2 gets set to zero during mapmov operations


# Adjust
From within the tb, see:
* `test =` - send 120 or 320 subcarriers
* `send_mod_eq` - send an eq which can mask subcarriers
* `trim_low` - if `send_mod_eq` is set, this sets how many subcarriers to zero out
* `update_bs` - true to update barrel shift values over ringbus
* `bs[5]`  - which barrel shift values to send to each stage in ascending order (`bs[0]` is stage 0)


# Plotting
Has python to plot stuff.  Run
* `python plot_output.py` - you will need `python-osi` checked out in the folder next to `higgs_sdr_rev_2`