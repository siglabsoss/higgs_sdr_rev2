# CS20 fpga

## Modules included

* [core_top](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/common/modules) - accepts FPGA_clk as input and gives 125 MHz and 12.5 MHz clock as output
* [MIB Master](https://github.com/siglabsoss/ip-library-core/tree/master/mib_bus) - to send and receive commands form MIB Slave. 
* [UDP Command](https://github.com/siglabsoss/ip-library-core/tree/master/lattice_support/gbit_mac/modules/udp_cmd) - Receives MIB commands from ETH and translates them for MIB Master

### Valid commands over the MIB
```

* Write in scratch register -> 0x0, 0x0, 0x0000004, data
* Read from scratch register -> 0x0, 0x1, 0x0000004, 0x0
```