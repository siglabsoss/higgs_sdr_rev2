# Purpose

Test input/output at the same time

# Setup
* cs21 streams data to cs31, who accepts but does not use the data
* cs31 streams data to cs30 in chunks of 1024*32
* cs30 consumes and checks data

# Notes
This was copied from the test_fill_level and then continued to be modified

# Test
DOES NOT SELF CHECK
