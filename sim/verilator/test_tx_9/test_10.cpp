void test10(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);



    preReset(top);

    t->reset(40);

    postReset(top);





    t->uarts["cs11"]->print = true;
    t->uarts["cs11"]->print = false;


    // t->uarts["cs11"]->registerVmemImemCb([](uint8_t tag, uint32_t addr, std::vector<uint32_t> *data)
    //                    {
    //                    std::cout << "TAG: " << int(tag) << std::endl;
    //                    for(auto& it : *data) {
    //                        std::cout << std::hex << it << std::endl;
    //                    }
    //                    });

    // tb inputs starts here
    // user can tick the clock for a period
    // append data to input streams, and look at output streams
    // modify negClock() and posClock() above
    // you can also insert for check streams from those functins()

    // delay between sending inputs

    int us = 400;
    // us = 60;

    // us = 220; // (was) enough for test

    us = 350+25;

    uint32_t amount = 5;
    uint32_t direction = 1;

    // std::vector<uint32_t> vin = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

    bool found1 = false;
    bool found2 = false;

    const auto check_awake = get_check_awake_packet();

    auto check_initial_settings = 70;

    const auto go_1 = 80;
    const auto go_2 = go_1 + 5;


    // int enabled_subcarriers = 128;

    bool mm_found_0, mm_found_1, mm_found_2;
    mm_found_0 = mm_found_1 = mm_found_2 = false;
    
    uint32_t mm_enabled, mm_start, mm_end;
    mm_enabled = mm_start = mm_end = 0;


    t->uarts["cs11"]->vmemTagToHexFile(0, "frame_", false);
    t->uarts["cs11"]->vmemTagToHexFile(1, "header_", false);
    t->uarts["cs11"]->vmemTagToHexFile(2, "userdata_", false);


    uint32_t pull1 = rand() % 5;
    cout << "pull 1 0x" << HEX32_STRING(pull1) << "\n";
    
    const unsigned extra_early = 0 + pull1;

    const unsigned expected = 1 + extra_early;

    const auto set_lifetime = op("set", 0, 512 - 4 - 4 - 2 - extra_early, GENERIC_OPERATOR_CMD);

    unsigned calls = 0;
    unsigned fails = 0;
    unsigned alive = 0;

    // expected was calculated with _printf on, however things go faster
    // with _printf off, so we use a tol
    constexpr int tol = 4;


    t->registerRb([&](const uint32_t word){
        handleFillLevelReply(word);
        calls++;
        cout << "Expected " << expected << "\n";
        cout << "Got      " << epoc_recent_reply_frames << "\n";
        int delta = abs((int)expected - (int)epoc_recent_reply_frames);
        cout << "Delta    " << delta << "\n";
        if( delta > tol )  {
            fails++;
        }
    }, CS20_FILL_LEVEL_PCCMD);

    t->registerRb([&](const uint32_t word){
        cout << "Got  alive! " << "\n";
        alive++;
    }, FEEDBACK_ALIVE);

    for(unsigned int i = 0; i < us; i++) {

        // risky to send rings before 80, but if we space them eth should be ok
        if( i == 15 ) {
            t->send_ring(RING_ADDR_TX_0, set_lifetime[0]);
        }

        if( i == 40 ) {
            cout << "sent ring\n";
            t->send_ring(RING_ADDR_TX_0, set_lifetime[1]);
        }

        if(i == go_1 ) {
            std::vector<uint32_t> data;

            // cout << "data: " << endl;
            // for(auto it = data.begin(); it != data.end(); it++) {
            //     cout << "0x" << HEX_STRING(*it) << endl;
            // }
            // cout << endl << endl;;

            // data.resize(8);

            const double enabled_subcarriers = 128;


            new_subcarrier_data_sync2(&data, 0xbeefbabe, enabled_subcarriers);
            // new_subcarrier_data_sync2(&data, 32, enabled_subcarriers);


            uint32_t val;

            for(int i = 0; i < 8; i++) {
                data.push_back(i);
            }

            const int header = 16;

            cout << "About to send " << data.size() << " words " << endl;
            cout << "or " << data.size()*32 << " bits" << endl;
            cout << "or " << data.size()*16 << " subcarriers worth " << endl;
            cout << "or " << (double)data.size()*16.0/(double)enabled_subcarriers << " frames worth " << endl;


            const uint32_t epoc = 1;     // seq2
            const uint32_t timeslot = 1; // seq

            const uint32_t frame = 512;

            // int custom_size;
            // assumes that 0 and 1023 are the boundaries
            // custom_size = header + ceil((data.size()*16)/enabled_subcarriers)*1024;
            // cout << "custom size " << custom_size << endl;


            // auto packet = 
            //     feedback_vector_mapmov_scheduled(
            //         FEEDBACK_VEC_TX_USER_DATA, 
            //         data, 
            //         custom_size, 
            //         FEEDBACK_PEER_8, 
            //         FEEDBACK_DST_HIGGS,
            //         timeslot,
            //         epoc
            //         );

            auto packet = 
                feedback_vector_mapmov_scheduled_sized_frames(
                    FEEDBACK_VEC_TX_USER_DATA, 
                    data, 
                    enabled_subcarriers, 
                    FEEDBACK_PEER_8, 
                    FEEDBACK_DST_HIGGS,
                    frame,
                    epoc
                    );

            // cout << "-----------\n";
            // for( auto w : packet ) {
            //     cout << HEX32_STRING(w) << "\n";
            // }

            // cout << "\n";
            // cout << "-----------\n";
            // for( auto w : packet2 ) {
            //     cout << HEX32_STRING(w) << "\n";
            // }



            t->inStreamAppend("cs11in", packet);
        }

        if( i == go_2 ) {
            cout << "sending check alive at " << i << "\n";
            // std::vector<uint32_t> zeros;
            // zeros.resize(20);
            // t->inStreamAppend("cs11in", zeros);
            t->inStreamAppend("cs11in", check_awake);
        }


        t->tick(500);
    }


    cout << "Ringbus got out" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
        uint32_t cmd = (*it & 0xff000000);

        switch(cmd) {
            case CS20_USERDATA_ERROR: {
                auto msg = getErrorStringFeedbackBusParse(*it);
                cout << msg;
            }
            break;
            default:
                break;
        }

    }


    t->allStreamDump();
    top->final();

    // Close trace if opened

    if (tfp) { tfp->close(); }


    assert(calls > 0);
    assert(fails == 0);
    assert(alive == 1);
    cout << "All Tests Passed\n";

    // Destroy model
    delete top; top = NULL;
    //print_vector(output_vector);
    // Fin
    exit(0);
}
