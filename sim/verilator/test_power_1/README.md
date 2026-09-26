# Purpose


Copied from `test_mover_9`.  Trying to upgrade power detection in cs31 to work.  Only adding the bare minimum fpgas.


# Flow
As I got going, at some point I realized that copying memory on the first DMA actually causes input ready to go low.  But it's really nice to see the power calculations based on the first frame.
So enable FORCE_POWER_FIRST_FRAME to use the first.