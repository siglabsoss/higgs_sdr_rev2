
#include "AirPacket.hpp"

AirPacketOutbound tx;

void setupAir() {
    tx.print_settings_did_change = true;
    tx.print5 = false;
    // tx.setTestDefaults();
    // code_bad0 = tx.set_code_type(AIRPACKET_CODE_REED_SOLOMON, code_length, fec_length);
//     tx.set_modulation_schema(FEEDBACK_MAPMOV_QPSK);
//     tx.set_subcarrier_allocation(MAPMOV_SUBCARRIER_320);

//     tx.set_rs(0);

    tx.set_modulation_schema(FEEDBACK_MAPMOV_QPSK);
    tx.set_subcarrier_allocation(MAPMOV_SUBCARRIER_320);
    tx.set_interleave(0);

}



int test3(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    preReset(top);
    t->reset(40);
    postReset(top);


    std::vector<uint32_t> packet;
    packet = file_read_hex("../../data/mapmov_640_lin_qam16_1_mapped.hex");


    std::vector<std::pair<uint32_t,uint32_t>> fb_times;

    unsigned int i = 0;

    // bool printExpected = false;
    bool printGot = false;


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



    // output from feedback bus activity
    unsigned got43Count = 0;
    bool captureEnabled = false;
    std::vector<uint32_t> sliced_out;
    unsigned gotBeyond43 = 0;

    // below is a series of "filter" lambdas that pick out the data we want
    // could be moved to inside higgs_helper

    // only gets lifetime32 and body
    auto gotSlicedCore = [&](uint32_t lifetime_32, const std::vector<uint32_t>& body) {
        if( lifetime_32 == 43 ) {
            got43Count++;
            captureEnabled = true;
        }

        if( !captureEnabled ) {
            return;
        }

        gotBeyond43++;

        for(const auto w : body) {
            sliced_out.push_back(w);
        }


        if( printGot ) {

            cout << "lifetime_32 " << lifetime_32 << "\n";


            // for(auto w : header) {
            //     cout << HEX32_STRING(w) << "\n";
            // }
            // cout << "\n";
            // cout << "\n";

            for(auto w : body) {
                cout << HEX32_STRING(w) << "\n";
            }
            cout << "\n";
            cout << "\n";
            cout << "\n";
        }
    };


    // result of filter, only gets sliced data
    auto gotSlicedData = [&](const std::vector<uint32_t>& header, const std::vector<uint32_t>& body) {
        const uint32_t lifetime_32 = header[5];
        gotSlicedCore(lifetime_32, body);
    };



    // catch all callbacks, filter
    t->registerFbCb([&](uint32_t t0, uint32_t t1, const std::vector<uint32_t>& header, const std::vector<uint32_t>& body) {
        const uint32_t t0t1 = (t0<<16) | t1;
        fb_times.emplace_back(t0t1,i);

        if( t0 != FEEDBACK_TYPE_VECTOR || t1 != FEEDBACK_VEC_DEMOD_DATA ) {
            return;
        }
        const uint32_t lifetime_32 = header[5];
        gotSlicedData(header, body);
    });

    // t->customParseFeedbackBusEndpoint("cs02out");


    // above is callbacks and other testbench overhead code
    ////////////////////////////////////////////////////////////////////////////////

    // how many ofdm frames of data will we use
    // unsigned use_frames = 1 + (rand() % 130);

    // setup global air object
    // setupAir();


    // const unsigned header_length = tx.sliced_word_count * 32;


    // bool print_settings = true;

    // // 20 words per frame
    // const unsigned word_length = use_frames*tx.sliced_word_count;

    // const std::vector<uint32_t> d = get_counter(0xff000000, 0xff000000+word_length);

    // // data with header
    // uint8_t seq;
    // std::vector<uint32_t> dataWHeader = tx.transform(d, seq, 0);

    // if( print_settings) {
    //     cout << "transform length: " << dataWHeader.size() << "\n";
    // }
    // tx.padData(dataWHeader);
    // if( print_settings) {
    //     cout << "final pad length: " << dataWHeader.size() << "\n\n";
    // }

    // auto sliced_words = tx.emulateHiggsToHiggs(dataWHeader)[1];

    // sliced_words.erase(sliced_words.begin(), sliced_words.begin()+header_length);

    // const auto expected_sliced = sliced_words;

    // cout << "Expected Words (" << expected_sliced.size()  << ")\n";
    // cout << "Expected Frames (" << use_frames  << ")\n";
    // if( printExpected ) {
    //     for(const auto w: expected_sliced) {
    //         cout << HEX32_STRING(w) << "\n";
    //     }
    // }

    // cout << "\n\n";



    uint32_t lifetime_value = rand() % 4096;

    uint32_t bump_lims = 2 + (rand() % 20);

    cout << "lifetime_value: " << lifetime_value << "\n";
    cout << "bump limit: " << bump_lims << "\n";



    std::vector<std::pair<uint32_t,uint32_t>> inserts;

    for(uint32_t k = 0; k < 1; k++) {
        inserts.emplace_back(RING_ADDR_RX_0, COOKED_DATA_TYPE_CMD | 2);
        inserts.emplace_back(RING_ADDR_RX_1, COOKED_DATA_TYPE_CMD | 2);
        inserts.emplace_back(RING_ADDR_RX_2, COOKED_DATA_TYPE_CMD | 2);
        inserts.emplace_back(RING_ADDR_RX_4, COOKED_DATA_TYPE_CMD | 2);
        inserts.emplace_back(RING_ADDR_TX_0, COOKED_DATA_TYPE_CMD | 2);
        inserts.emplace_back(RING_ADDR_TX_1, COOKED_DATA_TYPE_CMD | 2);
    }

    uint32_t insert_progress = 0;




    ////////////////////////////////////////////////////////////////////////////////

    // Estimated rate is 2.7 us per frame


    int us = 200;

    us = 200;

    us = 1300;
    // us = 110;

    // us = 500;

    // us = 90;

    // us = 10;

    // int automatic_quit_early = use_frames + 4; // measured in frames past 43




    // used below for mapmov things
    const uint32_t enabled_subcarriers = tx.enabled_subcarriers;
    const uint32_t modulation_schema = tx.modulation_schema;

    int now_late = 0; // in counters

    uint32_t next_pull = 0xffffffff;
    


    for(i = 0; i < us; i++) {

        // if( automatic_quit_early != -1 && gotBeyond43 >= automatic_quit_early ) {
        //     cout << "Automatically quitting run after " << i << " us because we got " << gotBeyond43 << " interesting frames!!\n\n\n";
        //     break;
        // }

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
            t->inStreamAppend("cs31in", packet);
            t->inStreamAppend("cs31in", packet);
            t->inStreamAppend("cs31in", packet);
        }



        if( i == 15 ) {
            t->wakeupSelfSync(); // no arguemnt is ok if us == 15
        }


        if( i == 40 ) {
            auto setLifetime = [=](const uint32_t x) {
                std::vector<uint32_t> pack;
                pack = op("set", 0, x, GENERIC_OPERATOR_CMD);

                t->send_ring(RING_ADDR_TX_0, pack[0]);
                t->send_ring(RING_ADDR_TX_0, pack[1]);

                t->send_ring(RING_ADDR_RX_0, pack[0]);
                t->send_ring(RING_ADDR_RX_0, pack[1]);
            };

            setLifetime(lifetime_value); // add frames for delta to go down
        }

        // if( i == 90 ) {
        //     t->setCookedDataMode(2);
        // }

        if( i == 90 ) {
            uint32_t bump = 1+(rand() % bump_lims);
            next_pull = i + bump;
        }

        if( i == next_pull ) {

            if( insert_progress < inserts.size() ) {
                auto xxx = inserts[insert_progress];
                uint32_t ttl, rb;
                std::tie(ttl,rb) = xxx;

                cout << "sending mode " << HEX32_STRING(rb) << " to " << ttl << " at " << i << "\n";
                t->send_ring(ttl, rb);

                insert_progress++;
            }

            uint32_t bump = 1+(rand() % bump_lims);
            next_pull = i + bump;
        }

        // if( i == 41 ) {
        //     auto setCS11Role = [=](const uint32_t x) {
        //         std::vector<uint32_t> pack;
        //         pack = op("set", 10, x, GENERIC_OPERATOR_CMD);

        //         t->send_ring(RING_ADDR_TX_0, pack[0]);
        //         t->send_ring(RING_ADDR_TX_0, pack[1]);
        //     };

        //     setCS11Role(DUPLEX_ROLE_TX_0);
        // }

        

        // if( i == 44 ) {
        //     // kicks off long process in eth which may actually deny ringbus processing
        //     // seems like this finishes at about 118
        //     t->send_ring(RING_ADDR_ETH, MAPMOV_MODE_CMD | MAPMOV_SUBCARRIER_320);
        // }


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




    uint32_t final_time = 0;
    for(const auto pp : fb_times ) {
        uint32_t types,uus; // type and us (time)
        std::tie(types, uus) = pp;

        std::string nicename = "??";
        switch(types) {
            case 0x00010001:
                nicename = "gnuradio";
                break;
            case 0x00020001:
                nicename = "fine sync";
                break;
        }

        cout << "t: " << uus << " " << HEX32_STRING(types) << "  " << nicename << "\n";

        final_time = uus;
    }

    if( fb_times.size() == 0) {
        cout << "didn't get any fb bus at all" << "\n";
        exit(1);
    }

    auto final_delta = us - final_time;
    cout << "final delta " << final_delta << "\n";

    assert(final_delta < 60);




    // const uint32_t t0t1 = (t0<<8) | t1;
    //     fb_times.empalce_back(t0t1);



    // testing time

    // trim output to only relevant words
    // auto sliced_out_relevant = sliced_out;
    // sliced_out_relevant.resize(expected_sliced.size());

    // bool match = expected_sliced == sliced_out_relevant;

    // cout << "Two vectors match? " << (match?"yes":"no") << "\n";
    // cout << "Vector Length expected: " << expected_sliced.size() << "\n";
    // cout << "Vector Length      out: " << sliced_out_relevant.size() << "\n";


    // assert(match);


    // Destroy model
    delete top; top = NULL;
    exit(0);
}
