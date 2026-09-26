# Purpose

There was another bug with issue 114 which caused the loop to lockup and not work if it was jamming when the pc finished.


# Flow

* cs20 is rigged to send 1,2,2,2,2
  * 1 feedback (loop)
  * 2 finesync (goes to pc)
* There is no random in this pattern
* cs11 (search for // hacked, )
  * this is the old epoc/frame oldschool timeslot version.
    * it should userdata on any frame??
  * this is rigged to always accept mapmov
  * During the parsing of the mampov we incur delays 64 word buffer cs20_top.sv
  * Then feedback (loop) starts to jam
  * In the condition that (pc) is done before loop is finished jamming, we had a bug.  (where loop would never send again).
  * tries to test for this



