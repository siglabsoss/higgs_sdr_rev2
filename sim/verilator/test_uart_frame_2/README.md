# Purpose

Verify higgs_helper uart parser works.  This does not care about frame structure or types.

# Flow
* Testbench sends two seed ringbus packets to CS11, CS01
* Both fpgas:
* pre-calculate a random list of delays
  * This is done ahead of time so that calculating the random number does not add delay itself
* loop through 0 -> 512 and send the uart character
  * note this results a counter of 0,255 twice
* after each character we call stall2 with a random argument that was pre calculated
* test bench checks to make sure correct uart counter data is received
