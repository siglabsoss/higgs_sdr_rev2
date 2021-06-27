# CS01 fpga

## Modules included

* [core_top](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/common/modules) - accepts FPGA_clk as input and gives 125 MHz and 12.5 MHz clock as output
* [MIB Slave](https://github.com/siglabsoss/ip-library-core/tree/master/mib_bus) - to send and receive commands form MIB Master. It is encapsulated in `core_top`
* [apc_argmax_wrapper](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/cs/cs01/hdl) - contains [arg_max](https://github.com/siglabsoss/ip-library-core/tree/master/argmax) and [apc](https://github.com/siglabsoss/ip-library-core/tree/master/apc) modules 
* [ddr3_fifo_apc](https://github.com/siglabsoss/ip-library-core/tree/master/DRAM_FIFO/ddr3_fifo_apc) - acts as a FIFO for moving average filter (Daryl is working on it) which uses DDR3 memory to store data samples.


### Valid commands over the MIB
```

* Write in scratch register -> 0x0, 0x0, 0x1100004, data
* Read from scratch register -> 0x0, 0x1, 0x1100004, 0x0
```