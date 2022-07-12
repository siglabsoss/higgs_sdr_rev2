# Purpose
Trying to add zero recovery to this feedback bus.  We jam the feedback bus with a SABOTAGE_CMD ringbus, which causes feedback bus to do an extra long dma transaction which causes corruption.  Or we corrupt by sending extra bytes from the outside.

# FPGA

# Test Flow
Goes like this:
* do 3 random feedback bus commands, non user data
* send a "feedback bus alive?" packet
* randomly choose to do outside, or inside, or both corruption
* send zeros
* send our special unjam packet
  * at this point we expect the bus to be unjammed
* do a short map/mov.
* send a "feedback bus alive?" packet.  This follows the previous mapmov and checks that both that and this command works.  It is possible for the map/mov to become desynched with cs20, so this sequence should validate correctness
* check that we get two replies for the "feedback bus alive?" packets.

