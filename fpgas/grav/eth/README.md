# ETH fpga

## Modules included

* [MIB Master](https://github.com/siglabsoss/ip-library-core/tree/master/mib_bus) - to send and receive commands form MIB Slave.
* [cmd_cdc](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/common/modules) - to transfer MIB signals in 125MHz clock domain
* [rf_rx_eth_frame_rx](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/grav/eth/hdl) - transfers data from CS20 to eth_mega_wrapper in correct way
* [cs_udp_cmd_rx_tx](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/grav/eth/hdl) - To send cmd data being received from CS20 to eth_mega_wrapper
* [eth_mega_wrapper](https://github.com/siglabsoss/higgs_sdr_rev2/tree/master/fpgas/grav/eth/hdl) - Contains different  modules like mac_arbiter, arp_reply, etc to communicate with the ethernet 

### Valid commands over MIB
```

*cmd to read mac address: 0x0,0x1,0x0000000,0x1 // lsw
						  0x0,0x1,0x0000004,0x1 //msw
```