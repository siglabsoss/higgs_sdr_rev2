# CS00 fpga

## Modules included

* [core_top](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/common/modules) - accepts FPGA_clk as input and gives 125 MHz and 12.5 MHz clock as output
* [MIB Slave](https://github.com/siglabsoss/ip-library-core/tree/master/mib_bus) - to send and receive commands form MIB Master. It is encapsulated in `core_top`
* [VershaCapture](https://github.com/siglabsoss/ip-library-core/tree/master/VershaCapture_CS) - to store data samples coming from ADC fpga and send them to ETH fpga
* [adc_dsp](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/cs/cs00/hdl) -  to perform dsp operations on the raw data being received from ADC fpga
* [UDP Packetizer](https://github.com/siglabsoss/ip-library-core/tree/master/lattice_support/gbit_mac/modules/udp_packetizer) - converts 32 bit data into 8 bit serial data and send them to ETH fpga as a form of udp packet.


### Valid commands over the MIB
```

* Write in scratch register -> 0x0, 0x0, 0x1000004, data
* Read from scratch register -> 0x0, 0x1, 0x1000004, 0x0
* Choose channel on VC -> 0x0,0x0,0x1030000,0xf // for counter
* cmd for force trigger -> 0x0,0x0,0x1030004,0x0
* cmd to set read delay	-> 0x0,0x0,0x1030008,0x100 //256
* cmd to set number of samples -> 0x0,0x0,0x103000c,0xf4240 //1000000
* cmd to start reading -> 0x0,0x0,0x1030010,0x1
* cmd to check for reading -> 0x0,0x1,0x1030014,0x0
* cmd to set the mac address -> 0x0,0x0,0x1020000,0x800 //msb 
								0x0,0x0,0x1020004,0x2792845E //lsb 
```