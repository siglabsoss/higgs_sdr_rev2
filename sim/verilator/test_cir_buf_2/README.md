# Test
Rewriting cirbut for power of 2 only.  This test removes usage of % and is mega faster than `circular_buffer.h`

# Details
A new type `circular_buf_pow2_t` can only have power of 2 elements.

# Usage:

```c
#include "circular_buffer_pow2.h"

circular_buf_pow2_t mybuffer = CIRBUF_POW2_STATIC_CONSTRUCTOR(mybuffer, 16);

int main() {
	CIRBUF_POW2_RUNTIME_INITIALIZE(mybuffer);
	circular_buf2_put(&mybuffer,0xa);
	circular_buf2_put(&mybuffer,0xb);
	circular_buf2_put(&mybuffer,0xc);
	unsigned int get_back;
	int error;
	error = circular_buf2_get(&nybuffer,&get_back); // error is 0 for success
}

```

# Notes
The runtime initilizer WILL BLOCK FOREVER (never exit) if incorrect arguments are passed

# Tests: 
* `test0()` - put a in 4 (fills it), check full and empty flags along the way.  doulbe get at the bottom.  insert 4 things, remove directly, test flags along the way, and values as they come out.  After, refill quickly, and double fill at top. verify that double fill didn't get in
* `test1()` - test add 3, remove 2, add 2, remove 3
* `test2()` - teste peek/put
* `test3()` - torture test circular_buf_occupancy, many adds and removes are performed in a tripple loop over:
  * how many were in there
  * how many to add
  * how many to subtract
* `test4()` - test I added when I realized filling a size 4 buf returns occupancy 3;. checks occupancy
* `test5()` - test subfunction circular_buf2_get_pow2()
