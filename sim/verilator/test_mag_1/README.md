# Purpose
Cooked data in RX1, feeds input to RX2. this data was captured on real radios.
The goal is to improve the 1/x algo. want to add averaging.

Note that cooked data does not contain the trunk when captured, so we will add it?



# Python
* check_capture.py - Reads capture_1.hex (raw ringbus from exfil library)
  * Raw ringbus are checked for correctness, and then assembled into original uin
  * outputs capture_2.hex which is 12 frames of 1024 values which were captured in hardware.  these were captured
  * at the input of cs22 without the trunk
* plot_capture.py - Reads capture_2.hex, randomy picks from the 12 to build 63 frames
  * adds noise to sc 38, 39, 42.  Note these are same index scheme as input of cs22 (RX2)
  * writes cooked_1.h which is included by the cs32 (RX1) and streamed to RX2
  * writes longframes_1.hex which is same as cooked_1 just in .hex form
* plot_capture2.py - Reads longframes_1.hex and plots
* plot_capture3.py - Reads output of testbench.  This can be plotted to see how the filter performs in verilator

# IIR Filter
This one tap IIR was copied from @drom:
```javascript
let irr = (a, b, g) => {
  a[0] = g * (b[0] - a[0]) + a[0];
  a[1] = g * (b[1] - a[1]) + a[1];
};
```



# Second Approach
I put perfect data on the qpsk channel, and then put noisy data on pilot. This allows me to see how far off from ideal
our mag adjustments were



# FPGAs
* RX0 - power detection, symbol sync, hand-tuned symbol advance, Sfo/cfo adjustments
* RX1 - Sends 1024 + 16 extra words.  These are scheduled as a signle DMA
* RX2 - CS01 will send 1024 + 16 extra words.  these 16 words contain sync data.  This is a single dma schedule.
* RX3 - reverse mover, tagges data, every N frames we send every subcarrier
* RX4 - Forwarding only

# Naming
* RX0 - CS31
* RX1 - CS32
* RX2 - CS22
* RX3 - CS21
* RX4 - CS20


