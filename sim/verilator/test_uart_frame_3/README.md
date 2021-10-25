# Purpose

Verify tb_debug works.  We have various small memory and test:
* setup_debug
* dump_vmem_cpu
* dump_vmem_row
* dump_vmem_dma
* block_until_dump_done

We also test sending data at not multiple of 16.  This is probably never going to be used, but would allows for intuitive usage.

# Flow
We also test imem.  The problem with imem is that we can only read full words out of scalar memory, not bytes.  Because of this, we need to do some shuffling in order to read these correctly.