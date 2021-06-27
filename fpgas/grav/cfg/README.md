# CFG fpga

## Modules included

* [core_top](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/common/modules) - accepts FPGA_clk as input and gives 125 MHZ and 12.5 MHz clock as output
* [MIB Slave](https://github.com/siglabsoss/ip-library-core/tree/master/mib_bus) - to send and receive commands form MIB Master. It is encapsulated in `core_top`
* [n25q_qspi_reader](https://github.com/siglabsoss/ip-library-core/tree/master/n25q_qspi_reader) - to read data flom flash memory
* [ecp5_slave_serial_programmer](https://github.com/siglabsoss/ip-library-core/tree/master/ecp5_slave_serial_programmer) - To program remaining fpgas on graviton serially
* [graviton_ti_cfg](https://github.com/siglabsoss/ip-library-core/tree/master/graviton_ti_cfg) - To program ADC and DAC modules. Also, to change settings of these modules using MIB bus


### Valid commands over the MIB
```

* Write in scratch register -> 0x0, 0x0, 0x1000004, data
* Read from scratch register -> 0x0, 0x1, 0x1000004, 0x0
* Change DAC current -> 0x0,0x0,0x1020000, 0x83f001
```

Note - user can replace `f` with any other hex value for dac current. All other bits should remain unchanged