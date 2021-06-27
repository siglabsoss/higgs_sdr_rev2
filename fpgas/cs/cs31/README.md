
# CS31 fpga

## Modules included

* [core_top](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/common/modules) - accepts FPGA_clk as input and gives 125 MHz and 12.5 MHz clock as output
* [MIB Slave](https://github.com/siglabsoss/ip-library-core/tree/master/mib_bus) - to send and receive commands form MIB Master. 
* [Upconverter](https://github.com/siglabsoss/ip-library-core/tree/master/upconverter) - Upconverts the incoming data from 31.25 MHz to 125 MHz. (*** Verify this***)
* [TX turnstile](https://github.com/siglabsoss/ip-library-core/tree/master/tx_turnstile) - Appends zeroes to complete a frame
* [VershaRepeat](https://github.com/siglabsoss/ip-library-core/tree/master/VershaRepeat_turnstile_32_bit) - Receives data from software via MIB reg writes. Sends this data to Tx turnstile whenever required.

### Valid commands over the MIB
```

* Write in scratch register -> 0x0, 0x0, 0x1d00004, data
* Read from scratch register -> 0x0, 0x1, 0x1d00004, 0x0

* Set number of samples on fpga30 -> 0x0,0x0,0x1d20000,number_samples
* Start reading from fifo -> 0x0,0x0,0x1d20004,0x1
* Set loopback -> 0x0,0x0,0x1d20008,0x1 // enable
* Set new data set -> 0x0,0x0,0x1d2000c,0x1
					  0x0,0x0,0x1d2000c,0x0
* Read done signal -> 0x0,0x1,0x1d20010,0x0
```