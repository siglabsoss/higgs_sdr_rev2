# Purpose

First test where I discovered that mac_tx_arbiter was overflowing.

It overflows before 450us.  However in-order to get this to happen you need to edit 

eth_mega_wrapper.sv:

```verilog
udp_packetizer_VC:

            .i_udp_payload_bytes      (8              ), 
```


# Code
We send a few rb/fb-bus packets and then stop


# Bug

Seems like fillcount does not return to zero despite the arbiter thinking it should be empty.
See https://github.com/siglabs/higgs_sdr_rev2/issues/104




# Notes

Ringbus packets are 50 bytes long


