int32_t epoc_recent_reply_frames;
/*
void applyFillLevelToMember(uint32_t word)
{
    uint32_t dmode =      (word & 0x00ff0000) >> 16;
    int32_t frame_delta =  word & 0x0000ffff;

    // do this to sign extend
    frame_delta <<= 16;
    frame_delta >>= 16;

    int epoc_delta = (int8_t)dmode;

    // unwinds some values, I think this is just to save compute
    // in cs20
    while(0 < epoc_delta) {
        frame_delta += SCHEDULE_FRAMES;
        epoc_delta--;
    }

    // copy to class
    epoc_recent_reply_frames = frame_delta;
    // epoc_recent_reply_frames_valid = true;
}

// see handleEpocReply
void handleFillLevelReply(uint32_t word) {

    applyFillLevelToMember(word); // updates epoc_recent_reply_frames

    cout << "epoc_delta represented as frames " << epoc_recent_reply_frames << endl;

    int target = 512*3; // this could be put somewhere else

    if( abs(epoc_recent_reply_frames) > (SCHEDULE_FRAMES*2) ) {
        cout << "epoc delta is WAY out of estimate" << endl;
    }

    // int target_error = epoc_recent_reply_frames - target;
    int target_error = target - epoc_recent_reply_frames;

    // we want this to be negative. I think if this is too large, we go too late
    // actually if that's true, 0 really is ideal.
    // cout << "   target_error " << target_error << " (large is late)" << endl;
    
    // map_mov_acks_received++;

    // if( abs(target_error) < 100 ) { // was 200
    //     cout << "   Skipping doesn't need update" << endl;
    //     return;
    // }

}
*/


int test2(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    preReset(top);
    t->reset(40);
    postReset(top);



    std::vector<uint32_t> packet;
    packet = file_read_hex("../../data/mapmov_640_lin_qam16_1_mapped.hex");


    int us = 200;

    us = 200;
    us = 150;

    // us = 90;

    // us = 10;

    uint32_t asmState = 0;

    t->registerRb([&](const uint32_t word) {
        const uint32_t who   = (word & 0x00f00000)>>16;
        const uint32_t dmode = (word & 0x000f0000)>>16;
        const uint64_t data =  word & 0x0000ffff;

        switch(dmode) {
            case 0:
                asmState = data;
                break;
            case 1:
                asmState |= (data << 16);
                cout << " Timer: " << HEX32_STRING(asmState) << "\n";
                break;
            default:
                cout << "UNKNOWN rb callback\n";
                break;
        }
    }, TIMER_RESULT_PCCMD);

    t->registerRb([&](const uint32_t word) {
        auto msg = getErrorStringFeedbackBusParse(TX_USERDATA_ERROR|word);
        cout << msg;
    }, TX_USERDATA_ERROR);


    // t->registerRb([&](const uint32_t word) {
    //     handleFillLevelReply(word);
    // }, TX_FILL_LEVEL_PCCMD);


    t->registerRb([&](const uint32_t word) {
        int32_t delta = schedule_parse_delta_ringbus(word);
        // handleFillLevelReply(word);
        cout << "lifetime_delta represented as frames " << delta << endl;
    }, TX_UD_LATENCY_PCCMD);


    const uint32_t enabled_subcarriers = 320;
    const uint32_t modulation_schema = FEEDBACK_MAPMOV_QPSK;


    for(unsigned int i = 0; i < us; i++) {

        if( i == 42 ) {
            // OMG
            // inserting here is MUCH better than inserting at start
            // this is because cs31 will take so long to start
            // we actually drop samples at the start
            // this means any small changes in startup time will totally affect output FFT
            // sampling time
            // std::vector<uint32_t> zrs;
            // zrs.resize(802-1);
            // t->inStreamAppend("cs31in", zrs);
            t->inStreamAppend("cs31in", packet);
            t->inStreamAppend("cs31in", packet);
            t->inStreamAppend("cs31in", packet);
            t->inStreamAppend("cs31in", packet);
            t->inStreamAppend("cs31in", packet);
        }

        // if( i == 10 ) {
        //     t->send_ring(RING_ADDR_CS11, GET_TIMER_CMD);
        // }

        if( i == 15 ) {
            t->wakeupSelfSync(); // no arguemnt is ok if us == 15
        }

        if( i == 40 ) {
            t->setCookedDataMode(2);
        }

        if( i == 83 ) {
            auto setLifetime = [=](const uint32_t x) {
                std::vector<uint32_t> pack;
                pack = op("set", 0, x, GENERIC_OPERATOR_CMD);

                t->send_ring(RING_ADDR_TX_0, pack[0]);
                t->send_ring(RING_ADDR_TX_0, pack[1]);
            };

            setLifetime(42); // add frames for delta to go down
        }

        if( i == 44 ) {
            // kicks off long process in eth which may actually deny ringbus processing
            // seems like this finishes at about 118
            // FIXME push other stuff out
            t->send_ring(RING_ADDR_ETH, MAPMOV_MODE_CMD | MAPMOV_SUBCARRIER_320);
        }

        if( i == 108 ) {

            uint32_t timeslot = 0;
            uint32_t epoc = 0;

            uint32_t lifetime = 43; // subtract frames for delta to go down

            const std::vector<uint32_t> d = get_counter(0xff000000, 0xff000000+20);

            // for(const auto w : d) {
            //     cout << HEX32_STRING(w) << "\n";
            // }

            auto packet = 
                feedback_vector_mapmov_scheduled_sized(
                    FEEDBACK_VEC_TX_USER_DATA,
                    d,
                    enabled_subcarriers,
                    0,
                    FEEDBACK_DST_HIGGS,
                    timeslot, // timeslot
                    epoc,  // epoc
                    modulation_schema // aka constellation
                );

            set_mapmov_lifetime_32(packet, lifetime);

            t->inStreamAppend("cs11in", packet);


            // for(const auto w : packet) {
            //     cout << HEX32_STRING(w) << "\n";
            // }



            // or we can set timeslot/epoc to 0 and use set_mapmov_epoc_frames()
        }

        // if( i == 85 ) {
        //     t->send_ring(RING_ADDR_CS31, DUPLEX_SYNCHRONIZATION_CMD | 10 );
        // }

        t->tick(500);
    }

    t->print_ringbus_out();
    t->allStreamDump();

    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) { tfp->close(); }

    // Destroy model
    delete top; top = NULL;
    exit(0);
}
