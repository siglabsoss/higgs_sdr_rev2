# DAC fpga

## Modules included

* [core_top](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/common/modules) - accepts FPGA_clk as input and gives 125 MHZ and 12.5 MHz clock as output
* [MIB Slave](https://github.com/siglabsoss/ip-library-core/tree/master/mib_bus) - to send and receive commands form MIB Master. It is encapsulated in `core_top`


### Valid commands over the MIB
```

* Write in scratch register -> 0x0, 0x0, 0x1100004, data
* Read from scratch register -> 0x0, 0x1, 0x1100004, 0x0
* Channel A enable -> 0x0,0x0,0x1110000,0x1
* Channel A disable -> 0x0,0x0,0x1110000,0x0
* Channel B enable -> 0x0,0x0,0x1110004,0x1
* Channel B disable -> 0x0,0x0,0x1110004,0x0
```
