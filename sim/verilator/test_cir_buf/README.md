# Test
Multi and very through test of circular buffer

# Tests: 
* `test0()` - put a in 4 (fills it), check full and empty flags along the way.  insert 4 things, remove directly, test flags along the way, and values as they come out
* `test1()` - test add 3, remove 2, add 2, remove 3
* `test2()` - teste peek/put
* `test3()` - torture test circular_buf_occupancy, many adds and removes are performed in a tripple loop over:
  * how many were in there
  * how many to add
  * how many to subtract
* `test4()` - test I added when I realized filling a size 4 buf returns occupancy 3;. checks occupancy

# Notes:
This library/test needs to be refactored to force power of 2 and remove `%`
