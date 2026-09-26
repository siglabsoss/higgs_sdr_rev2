# Test
Edge to edge test, meant to be bootloaded onto the board

# procedure
There are some make targets to run the tests.

* `make edge_test`
  * All-in-one single command to load and run the test
* `make edge_bootload`
  * This will bootload all the CS fpgas with the edge to edge test code.
* `make edge_edge_test`
  * This will send ringbus command to each fpga to conduct the edge to edge test.
* `make edge1`
* `make edge2`
* ...
* `make edge7`
  * These are independent commands to check only one edge at a time. edge1 tests the connection between CS00 and CS01, and so forth.
