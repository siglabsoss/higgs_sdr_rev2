# Purpose
Kept the same tx chain
Moved cs21 to cs20
Put custom code in cs21 to send out cooked ofdm frames

# Knobs
* Can adjust rate of OFDM frames, however we've tuned it to be approx the same as normal
* Can adjust rate of ringbus sent from cs21 to pc

# Crash
Seems it crashes with maximum ringbus at the same time
s-modem does not need to be sending any ringbus
Crashes after about 4 mins


# Notes

I can make it crash with:


sjs.higgsRunThread = false

sjs.sendZerosToHiggs(1000)


sjs.dsp.op(hc.RING_ADDR_CS21, "set", 3, 1000)




rerror = (x)=>{sjs.dsp.op(hc.RING_ADDR_ETH, "set", 3, x); sjs.dsp.op(hc.RING_ADDR_ETH, "get", 4, 0)}


sjs.attached.updatePartnerEq(true, true); sjs.sendZerosToHiggs(5000)


txTimer = setInterval( ()=>{sjs.attached.updatePartnerEq(true, true); sjs.sendZerosToHiggs(5000)}  , 500);



sjs.dsp.op(hc.RING_ADDR_ETH, "set", 5, 2) // led



# Saturday Overall notes


I ran the tx side, with sjs.safe().  just bootloaded and then ran test_eth every 30 seconds.  This ran for 2 hours 20 mins.


Things I noticed, cs30 buffer does not seem to have pushback



# Sunday notes

Will run tx side with bootload only and test_ring_forever. no program