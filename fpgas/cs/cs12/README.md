# CS12 fpga

## Modules included

* [core_top](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/common/modules) - accepts FPGA_clk as input and gives 125 MHz and 12.5 MHz clock as output
* [MIB Slave](https://github.com/siglabsoss/ip-library-core/tree/master/mib_bus) - to send and receive commands form MIB Master. It is encapsulated in `core_top`

### Valid commands over the MIB
```

* Write in scratch register -> 0x0, 0x0, 0x1600004, data
* Read from scratch register -> 0x0, 0x1, 0x1600004, 0x0
```