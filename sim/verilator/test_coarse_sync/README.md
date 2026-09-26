# Folder
This folder allows for bootloading onto real hardware.  The purpose is to demonstrate the fft in cs10 running continuously.

Pre calculated data is fed from cs20.  If you want to change which data is being sent, simply edit cs20

# Procedure
Bootload with official bitfiles

* `make fetch_master rev=82` at time of writing.
  * (flash these bitfiles)
* `make test_eth` from `higgs_sdr_rev2` folder until it passes
* `make config_dac`
* `make transmit tx=a`
* from this folder
  * `make vall bootload_cs10`
  * `make vall bootload_cs20`

At this point you will see transmission coming from channel A.  To change, simply edit `override/fpgas/cs/cs20/c/src/main.c` and then re bootload it.
