# Test RX Chain

The receive chain in Copper Suicide currently consist of nine FPGAs in the following order `ADC`, `CS00`, `CS01`, `CS11`, `CS21`, `CS31`, and `CS30`. This folder test the health of all RX connections by sending counter values through each FPGA and receiving from a computer. The counter value must be a multiple of 367 because the ethernet FPGA pack and ships data in sizes of 368 words. The sequence number is the first value of every packet. Because the vector memory size of `CS00` is 64K or 65536 words of data, the maximum counter value to check is 65326. There are two main test. The first test checks the connection between the `ADC` FPGA and `CS00` by sending counter values from `ADC` to `CS00` and receiving it from a computer. The second test checks the connection between `CS00` and the computer by sending counter values to `CS00` and receiving it from a computer. It is advised to first check the connection from `CS00` to computer and then `ADC` FPGA to `CS00`. If both test pass, then the receive chain is properly connected.

# Get Started

## Dependencies

Install the following libraries:

On Ubuntu:
```bash
sudo apt-get install python-pip
sudo pip2 install matplotlib
sudo apt-get install python-tk
sudo pip2 install bitstring
```

On openSUSE:
```bash
sudo zypper install python-pip
sudo pip2 install matplotlib
sudo zypper install python-tk
sudo pip2 install bitstring
```

Checkout the following branch:
```bash
cd higgs_sdr_rev2
git checkout deng
git submodule update --init --recursive
```

## Flashing

Flash Higgs with any bit files where testing the RX chain is desired. As a start, use the following bit files as a reference before testing other bit files.

```bash
cd higgs_sdr_rev2
make fetch_dev rev=925 # This revision passes RX Chain Test
cd official_builds/higgs_sdr_rev2_dev
make programhiggs # From Windows VM
make test_eth # Ensure Ethernet FPGA is running properly
```

## Testing

### RX Single Test
Begin a single RX chain test using the following commands:

```bash
cd higgs_sdr_rev2/sim/verilator/test_rx_chain
make bootload_test_rx
cd ../../../
make test_ring # Ensure ringbus is working properly
cd higgs_sdr_rev2/sim/verilator/test_rx_chain
make test_rx_chain counter=65326
```

If test is successful, the following message will appear:
```bash
2018-05-01 21:31:26 [INFO] (receive_data:66) RX Chain PASSED. A counter of 65326 values were received
```
If test is unsuccessful, either message will appear:
```bash
2018-05-01 21:31:26 [INFO] (receive_data:66) RX Chain FAILED. Incorrect counter values received

OR

2018-05-01 21:31:26 [INFO] (receive_data:66) RX Chain FAILED. No counter values received
```

### RX Torture Test
To execute multiple RX chain test continuously use the following commands:

```bash
cd higgs_sdr_rev2/sim/verilator/test_rx_chain
make bootload_test_rx # Optional if executed above
cd ../../../
make test_ring # Ensure ringbus is working properly
cd higgs_sdr_rev2/sim/verilator/test_rx_chain
make torture_rx_chain iteration=100
```

The above command will execute the RX chain test 100 times. The test will terminate at the first failure and output an error message.

### ADC Single Test
Test the connection between the `ADC` FPGA and `CS00` via the following commands:

```bash
cd higgs_sdr_rev2/sim/verilator/test_rx_chain
make bootload_test_rx # Optional if performed above
cd ../../../
make test_ring # Ensure ringbus is working properly
cd higgs_sdr_rev2/sim/verilator/test_rx_chain
make test_adc_chain counter=65326
cd ../../../
make disable_adc_counter
```

If test is successful, the following message will appear:
```bash
2018-05-07 16:36:18 [INFO] (receive_data:88) Enabling ADC counter
2018-05-07 16:36:18 [INFO] (higgs:116) Packet sent: 0x3, 0xd380000
2018-05-07 16:36:18 [INFO] (receive_data:94) Resetting ADC counter
2018-05-07 16:36:18 [INFO] (higgs:116) Packet sent: 0x3, 0xc380000
2018-05-07 16:36:18 [INFO] (receive_data:117) Enabling input DMA of CS00
2018-05-07 16:36:18 [INFO] (higgs:116) Packet sent: 0x3, 0x200ff2e
2018-05-07 16:36:18 [INFO] (receive_data:122) Enabling data pass through in 367 chunks
2018-05-07 16:36:18 [INFO] (higgs:116) Packet sent: 0x3, 0xa00ff2e
2018-05-07 16:36:18 [INFO] (receive_data:77) RX Chain PASSED. A counter of 65321 values were received
```

If test is unsuccessful, either message will appear:
```bash
2018-05-07 16:36:18 [INFO] (receive_data:88) Enabling ADC counter
2018-05-07 16:36:18 [INFO] (higgs:116) Packet sent: 0x3, 0xd380000
2018-05-07 16:36:18 [INFO] (receive_data:94) Resetting ADC counter
2018-05-07 16:36:18 [INFO] (higgs:116) Packet sent: 0x3, 0xc380000
2018-05-07 16:36:18 [INFO] (receive_data:117) Enabling input DMA of CS00
2018-05-07 16:36:18 [INFO] (higgs:116) Packet sent: 0x3, 0x200ff2e
2018-05-07 16:36:18 [INFO] (receive_data:122) Enabling data pass through in 367 chunks
2018-05-07 16:36:18 [INFO] (higgs:116) Packet sent: 0x3, 0xa00ff2e
2018-05-07 16:36:18 [INFO] (receive_data:77) RX Chain FAILED. Incorrect counter values received

OR

2018-05-07 16:36:18 [INFO] (receive_data:88) Enabling ADC counter
2018-05-07 16:36:18 [INFO] (higgs:116) Packet sent: 0x3, 0xd380000
2018-05-07 16:36:18 [INFO] (receive_data:94) Resetting ADC counter
2018-05-07 16:36:18 [INFO] (higgs:116) Packet sent: 0x3, 0xc380000
2018-05-07 16:36:18 [INFO] (receive_data:117) Enabling input DMA of CS00
2018-05-07 16:36:18 [INFO] (higgs:116) Packet sent: 0x3, 0x200ff2e
2018-05-07 16:36:18 [INFO] (receive_data:122) Enabling data pass through in 367 chunks
2018-05-07 16:36:18 [INFO] (higgs:116) Packet sent: 0x3, 0xa00ff2e
2018-05-07 16:36:18 [INFO] (receive_data:77) RX Chain FAILED. No counter values received
```

### ADC Torture Test
To execute multiple ADC chain test continuously use the following commands:

```bash
cd higgs_sdr_rev2/sim/verilator/test_rx_chain
make bootload_test_rx # Optional if executed above
cd ../../../
make test_ring # Ensure ringbus is working properly
cd higgs_sdr_rev2/sim/verilator/test_rx_chain
make torture_adc_chain iteration=100
cd ../../../
make disable_adc_counter
```

The above command will test the ADC connection 100 times. The test will terminate at the first failure and output an error message.

### Capture Data
To capture data from Higgs:

```bash
cd higgs_sdr_rev2
make disable_adc_counter
make get_adc_samples samples=65326 capture_samples packet_count=178
```

Received data will be plotted and saved to a CSV file called `receive_data.csv`
# Contributing

As the requirement of the receive chain changes, update this README and test script to check if receive is working properly. One possible test that may be necessary is checking a continuous stream of data. At this moment, this test only verifies `n` packets.

## Point of Contact
* Janson Fang
* Ben Morse
