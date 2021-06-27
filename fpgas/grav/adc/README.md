# ADC fpga

## Modules included

* [core_top](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/common/modules) - accepts FPGA_clk as input and gives 125 MHZ and 12.5 MHz clock as output
* [MIB Slave](https://github.com/siglabsoss/ip-library-core/tree/master/mib_bus) - to send and receive commands form MIB Master
* [adc_data_rx](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/grav/adc/hdl) - accepts 16 bit data at 250MHz from adc module and gives 32 bit data at 125 MHz
* [variable gain attenuator adjustment](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/grav/adc/hdl) - configures the gain settings of VGA through reg write over the MIB
* digital step attenuator control - configures the gain settings of DSA through reg writes over the MIB

### Valid commands over the MIB
```

* Write in scratch register -> 0x0, 0x0, 0x1200004, data
* Read from scratch register -> 0x0, 0x1, 0x1200004, 0x0
* Change attenuation setting for DSA (First_amplifier) -> 0x0, 0x0, 0x1210000, gain
* Change attenuation setting for VGA (Second_amplifier) -> 0x0, 0x0, 0x1220000, gain
* Select channel A to receive -> 0x0,0x0,0x1230000,0x0
* Select channel B to receive -> 0x0,0x0,0x1230000,0x1
```

Note: value of attenuation can be from 0 to 31. attenuation = 0 means minimum attenuation