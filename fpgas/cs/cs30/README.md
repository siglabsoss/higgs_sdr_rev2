
# CS30 fpga

## Modules included

* [core_top](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/common/modules) - accepts FPGA_clk as input and gives 125 MHz and 12.5 MHz clock as output
* [MIB Slave](https://github.com/siglabsoss/ip-library-core/tree/master/mib_bus) - to send and receive commands form MIB Master. 
* [IFFT framer]() - append zero to complete the frame
* [IFFT]() - Calculate IFFT of incoming samples
* [Synthesis Polyphase Filters](https://github.com/siglabsoss/ip-library-core/tree/master/synthesis_filter_bank) - *** Update ***
* [Packet buffer using DDR fifo](https://github.com/siglabsoss/ip-library-core/tree/master/DRAM_FIFO/ddr_fifo_32_bit_read) - Works as a normal fifo. You can read 32 bit of data at every fourth clcok cycle of 125 MHz.

### Valid commands over the MIB
```

* Write in scratch register -> 0x0, 0x0, 0x1c00004, data
* Read from scratch register -> 0x0, 0x1, 0x1c00004, 0x0
```