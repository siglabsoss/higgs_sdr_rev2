# Purpose

Verify new mapmov verilog works

# Test 7
Does a QPSK mapmov after 80us.
Repurposing this to test eth mapmov "chunked init".  we do this by just checkout output of mapmov. we do not need to wait for cs11 to schedule and then transmit over the air.  In fact I belive test7 does not have the right epoc / whatever to be able to transmit over the air.

# Tets 8
Test that Generic op works

# Test 9
Check that "check bootload" command works

# Test 10
Using Generic op, do a qpsk mapmov transmit.  We set the lifetime_32 time with a random variance of 5.  This is such a minor thing we don't need to worry about randomizing this test.  The test just verifies that the "early frames" comes back correct.  Depending if `_printf` `ENABLE_TB_DEBUG` is enabled or not, the "early frames" may be up to 3 wrong, but we just check for a tolerance.  This was put in place before I changed timeslot to frame number.

# Test 11
* Smodem printed out a mapmov packet
* this was saved into mapmov_640_lin_1_mapped.hex
* mapmov.c was adjusted (guess and check)
* ringbus in test_tx_9/text_11 adjusted for new mode
* cs11_in.hex opened and all lines past mapmov header (first 16) copied to mapmov_640_lin_1_mapped.hex
* test_5.cpp in test_mover_9 runs first part with exit(0)
* output of this is then pasted into test_mover_9/override/fpgas/cs/cs22/c/src/debug_640_lin4.h as a VMEM_SECTION array
* cs22 is edited to dma_block_send this variable
* test_5.cpp edited to remove exit(0) and run. output is final output of sliced data



# Mapmov counter

```cpp
    std::vector<uint32_t> packet;

    // packet = file_read_hex("../../data/mapmov_512_lin_1.hex");
    packet = file_read_hex("../../data/mapmov_640_lin_1.hex");

    packet.resize((40*32)+16);

    for(unsigned i = 0; i < 40*10; i++ ) {
        packet.push_back(0xf0000000 + i);
    }

    feedback_update_length_field(packet);

    // file_dump_vec(packet, "../../data/mapmov_640_lin_3.hex");

    // then I messed up, wrote to disk, reread and ran this
     std::vector<uint32_t> packet2;

    unsigned ii = 0;
    for(auto w : packet) { 

        if( ii >= 16 ) {
            packet2.push_back(w);
        }

        ii++;
    }

    // for(auto w : packet2) { 
    //     cout << HEX32_STRING(w) << "\n";
    // }

             auto packet3 = 
                    feedback_vector_mapmov_scheduled_sized(
                        FEEDBACK_VEC_TX_USER_DATA, 
                        packet2, 
                        640, 
                        0,
                        FEEDBACK_DST_HIGGS,
                        0, // timeslot fillin, overwritten later
                        0,  // epoc fillin, overwritten later
                        FEEDBACK_MAPMOV_QPSK
                        );
                    
```