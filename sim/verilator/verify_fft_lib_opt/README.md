# Test
On board verification of Optimized fft

# Instructions

* In a new window, run `scripts/dump_ringbus.py`
* make vall
* make bootload_cs20

# Passing output:
`0xf0000003` is the passing ringbus message

```
0xf0000001
0x228d
0xf0000003
```

# Failing output:
`0xf0000004` is the failing ringbus message

```
0xf0000001
0x228d
0xf0000004
```

