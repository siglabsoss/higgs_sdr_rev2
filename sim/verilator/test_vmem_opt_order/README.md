# Test
This test makes sure that VMEM_SECTION will always align to a 16 word boundary.

# Notes
* This was copied from some old fft version and hacked up.  The twiddle factors are used
* We set `tw3f` to be length `15` instead of `16`. With the old version for VMEM_SECTION this causes everything here to be off by 1

# Jenkins
* Under Jenkins Test
