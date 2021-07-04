# cs20_out_qam16.hex
QAM 16 output of cs20 (meaning no fft no cp). un rotated

# cs10_out_qam16_rotated.hex
Output of QAM16 with the default rotations applied given by zhen

# mapmov_640_lin_1.hex
# mapmov_640_lin_1_mapped.hex
# mapmov_640_lin_1_sliced.hex
testing 640 linear subcarriers.  see ChagneModulation.md


# mapmov_640_lin_qam16_
Used this on tx to generate:
```js
sjs.txSchedule.doWarmup = true
sjs.txSchedule.print1 = true
sjs.txSchedule.air.set_rs(0)
sjs.txSchedule.debug(400)
```

# mapmov_320_qpsk_1.hex
`attachHeader() using: 00000259`

