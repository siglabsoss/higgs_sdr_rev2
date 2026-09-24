# Purpose
Copied from test_mover_8 and hacked up to try and port cs00 to nodejs

# Files
`sc_16_144_880_1008_bpsk_mod.hex` was created from `sc_16_144_880_1008_bpsk.hex` but for every value a repeating sequence from 0->33 was added to make sure each ofdm frame in the file is unique.

# Coarse samples
offsets into the input buffer (`sc_16_144_880_1008_bpsk_mod.hex`) which the coarse sync uses


res0 11050
res1 12074
res2 12074
res3 13098
res4 13098
res5 14122
res6 14122
res7 15146
res8 15146
res9 16170
res10 16170
res11 17194
res12 17194
res13 18218
res14 18218
res15 19242
res16 19242
res17 20266
res18 20266
res19 21290
res20 21290
res21 22314
res22 22314
res23 23338
res24 23338
res25 24362
res26 24362
res27 25386
res28 25386
res29 26410
res30 26410
res31 27434
res32 27434
res33 28458
res34 28458
res35 29482
res36 29482
res37 30506
res38 30506
res39 31530



# Exponential sin wave
offsets into the `exp_data_1280_ext_15` variable which coarse sync uses

0x0000500
0x0000400
0x0000300
0x0000200
0x0000100
0x0000500
0x0000400
0x0000300
0x0000200
0x0000100
0x0000500
0x0000400


# temp_complex
results of `temp_complex`

0x728f15d
0xe96e251
0xe30e2e3
0x160ed425
0x1d60c56a
0x2486b6c3
0x2bf1a7b7
0x2b8aa848
0x3c059e59
0x3cd490e3
0x3b058000
0x42e68000
0x440f8000
0x4edc8000
0x59bf8000
0x7acf12d
0xf1ae21e
0xeace2ac
0x1689d3e9
0x1ddfc532
