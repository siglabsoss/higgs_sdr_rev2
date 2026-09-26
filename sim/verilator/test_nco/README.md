# Test Numerically Controlled Oscillator (NCO)

A numerically controlled oscillator (NCO) is a digital signal generator which usually creates a discrete time sinusoidal waveform. Being able to generate a sinusoidal waveform within the framework of RISCV allows quick execution of frequency offset in the transmit and receive chain. A signal of the form ![eqn](http://latex.codecogs.com/gif.latex?$f({\omega}t)=e^{j({\omega}t+\theta)}$) is generated in RISCV. The frequency and angle can be set by editing `NCO_START_ANGLE` and `NCO_DELTA` where frequency and delta is related as follow ![eqn](http://latex.codecogs.com/gif.latex?$\delta=131.072f$). The decision to create a NCO within RISCV is to allow frequency offsets to be corrected quickly. This test verifies whether a correct oscillator is created in hardware given different frequencies and angle. 

# Get Started

## Dependencies

Checkout the following branch:
```bash
cd higgs_sdr_rev2
git checkout deng
# Alternatively, master branch works equally well
git checkout master
git submodule update --init --recursive
```

## Testing

## Test NCO

An oscillator with pre-set frequency and angle is loaded in CS30. Executing the following test will generate an oscillator and send the values to PC to be compared with verilated values.

```bash
cd higgs_sdr_rev2
make test_eth
make test_ring
cd sim/verilator/test_nco
make compilehex
make
make bootload_cs30
make test_nco nco_length=32768
```

If test is successful, the following message will appear:
```bash
2018-06-15 18:20:55 [INFO] (higgs:132) Packet sent: 0x8, 0x27008000
2018-06-15 18:20:56 [INFO] (test_nco:65) NCO TEST PASSED. A NCO of size 32768 matches with verilated results
```

If test is unsuccessful, the following message will appear:
```bash
2018-06-15 18:20:05 [INFO] (higgs:132) Packet sent: 0x8, 0x27008000
2018-06-15 18:20:06 [INFO] (test_nco:71) NCO TEST FAILED. Received and verilated NCO has different shapes: (33030,) (32768,)


OR

2018-06-15 18:22:51 [INFO] (higgs:132) Packet sent: 0x8, 0x27008000
2018-06-15 18:22:52 [INFO] (test_nco:76) NCO Test FAILED. Received and verilated values are different
```

# Contributing

Currently this test compares hardware results with verilated results. Future improvements to this test include verifying hardware results with analytical methods. For example, plot received data and see if values are within tolerance of an ideal sinusoid. In addition, this test only verifies a single angle and frequency. For more comprehensive test, it is ideal to test a range of frequencies and angles.

## Point of Contact
* Janson Fang
* Ben Morse
