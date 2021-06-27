## schedule_maker.py

The script generates schedule code for the mover function. To select the appropriate subcarriers, edit the ```subcarriers = ``` line in the script.
The 512 subcarriers to the right on the center frequency range from 1023 (center) to 512 (right) in decreasing order and the 512 sucarriers to the left of the center frequency range from 0 (center) to 511 (left) in the increasing order.
Another way to think about this that the sucarrier numbers are rotated right by 512 positions.

Since we can only 902Mhz to 928Mhz, do not use subcarriers between 512+/-102

The script generates both reverse and normal configuation of the mover. Currently, you need to delete the reverse configuration to make it work with test_tx_6.
