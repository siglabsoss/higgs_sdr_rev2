# Test ook 1
Test of `ook_modem.h` including checksum

# Flow
* cs21 outputs ook words from the ook modulator.  note we don't have a 1024 frame here or anything, each word is the next ook symbol.
  * A counter is output
  * The second output is corrupted by modifying the data as it is being generated (never do this in real setup)
* cs20 runs the ook demodulator.
  * Since each ook message is 48 bits we send 2 ringbus bash to the tb for each message we get
* testbench asserts that correct messages get through and corrupted one does not
