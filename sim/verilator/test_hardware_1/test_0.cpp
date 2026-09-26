int test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    preReset(top);
    t->reset(40);
    postReset(top);



    std::vector<uint32_t> packet;
    packet = file_read_hex("../../data/mapmov_640_lin_qam16_1_mapped.hex");


    int us = 200;

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


    for(unsigned int i = 0; i < us; i++) {

        if( i == 12 ) {
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
            uint32_t timer = 0x4e00;
            t->send_ring(RING_ADDR_CS01, SELF_SYNC_CMD | (timer>>8) );
            t->send_ring(RING_ADDR_CS02, SELF_SYNC_CMD | (timer>>8) );
            t->send_ring(RING_ADDR_CS11, SELF_SYNC_CMD | (timer>>8) );
            t->send_ring(RING_ADDR_CS12, SELF_SYNC_CMD | (timer>>8) );
            t->send_ring(RING_ADDR_CS20, SELF_SYNC_CMD | (timer>>8) );
            t->send_ring(RING_ADDR_CS21, SELF_SYNC_CMD | (timer>>8) );
            t->send_ring(RING_ADDR_CS22, SELF_SYNC_CMD | (timer>>8) );
            t->send_ring(RING_ADDR_CS31, SELF_SYNC_CMD | (timer>>8) );
            t->send_ring(RING_ADDR_CS32, SELF_SYNC_CMD | (timer>>8) );
        }

        if( i == 85 ) {
            t->send_ring(RING_ADDR_CS31, DUPLEX_SYNCHRONIZATION_CMD | 10 );
        }

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
