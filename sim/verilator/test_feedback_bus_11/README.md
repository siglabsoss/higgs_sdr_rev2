# Purpose

Finally figured out why fb bus was crashing.  If a short vector length comes in, the code was doing nothing.  This test verifies by:

* sending a short header
* sending zeros
* sending 3 "check awake" fb messages

The test passes if 1 or more "check awake" make it through.  Note the test is randomized so it's actually this

* ? send zeros of ? length
* send a ? long/short header
* send 3 "check awake"

