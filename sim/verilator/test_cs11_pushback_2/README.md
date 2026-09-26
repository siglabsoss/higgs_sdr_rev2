# Purpose

Found a HIDDEN BUG IN MAPMOV.  This has been here forever and may have been causing Ben to think it was "off chip errors".

# Background
When mapmov is given backpressure, it wasn't behaving correctly.

# Flow
Testbench sends a pattern of words that will be "valid" feedback bus words, but will never be a mapmov.  The valid words will also be low in value, so the lenght field does not go crazy

* cs11 runs test_dma_torture code, but we are actually giving it the wrong vector
  * doesn't matter as we are only using this code for it's random dma in behavior
* testbench checks it's ideal memory against the output of cs11_out.hex
* a diff output can also be displayed



# Seed extraction

Download console text into directory.  Then run:
```bash
cat *consoleText.txt | grep -a6 pushback_2/obj_dir | grep "random seed"
```

