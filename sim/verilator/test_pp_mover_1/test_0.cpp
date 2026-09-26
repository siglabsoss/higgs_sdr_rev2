int test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    preReset(top);
    t->reset(40);
    postReset(top);


    t->uarts["cs21"]->vmemTagToHexFile(0, "schedule_");
    t->uarts["cs21"]->vmemTagToHexFile(1, "moved_");

    // std::vector<uint32_t> packet;
    // packet = file_read_hex("../../data/mapmov_640_lin_qam16_1_mapped.hex");

    std::vector<uint32_t> expected;
    expected = file_read_hex("expected_hash.hex");


    unsigned int pull1, pull2;

    pull1 = rand() & 0x00ffffff;
    pull2 = 3 + (rand() % (57-3));

    cout << "pull1: 0x" << HEX32_STRING(pull1) << "\n";
    cout << "pull2: 0x" << HEX32_STRING(pull2) << "\n";


    int us = 240; // ORIGINAL RUNTIME was 200
    // this runtime of 200 was used to compare efficienty of the new ping-pong
    // approach to the old cs21 way.. NOW we are using a random valid ready
    // to add test to jenkins and thus this needs to be a bit longer

    // us = 1;

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


    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> hashes_plus;

    t->registerRawFbCb([&](const feedback_frame_t *v) {
        uint32_t type0 = v->type;
        uint32_t type1 = ((const feedback_frame_vector_t*)v)->vtype;
        uint32_t hash = _feedback_hash(v);
        // cout << "Hash: " << HEX32_STRING(hash) << "\n";
        cout << "Hash: " << HEX16_STRING(type0) << " " << HEX16_STRING(type1) << " " << HEX32_STRING(hash) << "\n";
        hashes_plus.push_back({type0, type1, hash});
    });

    t->enableDumpFbBus();


    for(unsigned int i = 0; i < us; i++) {

        if( i == pull2 ) {
            cout << "sending random seed at us " << pull2 << "\n";
            t->send_ring(RING_ADDR_CS22, EDGE_EDGE_IN | pull1 );
        }

        if( i == 15 ) {
            uint32_t timer = 0x3a00;
            t->send_ring(RING_ADDR_CS20, SELF_SYNC_CMD | (timer>>8) );
            t->send_ring(RING_ADDR_CS21, SELF_SYNC_CMD | (timer>>8) );
            t->send_ring(RING_ADDR_CS22, SELF_SYNC_CMD | (timer>>8) );
        }

        // Force them to be different by stamping output
        // if( i == 16) {
        //     t->send_ring(RING_ADDR_CS21, STAMP_STREAM_OUTPUT_CMD | 1);
        // }

        // if( i == 85 ) {
        //     t->send_ring(RING_ADDR_CS31, DUPLEX_SYNCHRONIZATION_CMD | 10 );
        // }

        t->tick(500);
    }

    t->print_ringbus_out();


    std::vector<uint32_t> hashes;
    cout << "Hashes:\n";
    for(const auto w: hashes_plus) {
        // cout << HEX32_STRING(w) << "\n";
        hashes.push_back(std::get<2>(w));
    }



  unsigned min_hash = std::min(hashes.size(), expected.size());

  cout << "Got      Hashes size: " << hashes.size() << "\n";
  cout << "Expected Hashes size: " << expected.size() << "\n";

  file_dump_vec(hashes, "got_hashes.hex", false);

  if( hashes.size() > expected.size() ) {
    cout << "Got " << hashes.size() - expected.size() << " extra hashes" << "\n";
    hashes.resize(expected.size());
  }

  bool match = hashes == expected;

  if( match ) {
    cout << "Hashes match\n\n";
  } else {
    cout << "Hashes DO NOT match!!!!!!!!!\n\n";
  }


    for(const auto w: hashes) {
        // cout << HEX32_STRING(w) << "\n";
        // hashes.push_back(std::get<0>(w));
    }





    t->allStreamDump(false);

    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) { tfp->close(); }

    assert(match);

    // Destroy model
    delete top; top = NULL;
    exit(0);
}
