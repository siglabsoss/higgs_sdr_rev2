
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



int test2(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    preReset(top);
    t->reset(40);
    postReset(top);

    bool printExpected = false;
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
        if( t0 != FEEDBACK_TYPE_VECTOR || t1 != FEEDBACK_VEC_DEMOD_DATA ) {
            return;
        }
        const uint32_t lifetime_32 = header[5];
        gotSlicedData(header, body);
    });

    t->customParseFeedbackBusEndpoint("cs02out");


    // above is callbacks and other testbench overhead code
    ////////////////////////////////////////////////////////////////////////////////

    // how many ofdm frames of data will we use
    unsigned use_frames = 1 + (rand() % 130);

    // setup global air object
    setupAir();


    const unsigned header_length = tx.sliced_word_count * 32;


    bool print_settings = true;

    // 20 words per frame
    const unsigned word_length = use_frames*tx.sliced_word_count;

    const std::vector<uint32_t> d = get_counter(0xff000000, 0xff000000+word_length);

    // data with header
    uint8_t seq;
    std::vector<uint32_t> dataWHeader = tx.transform(d, seq, 0);

    if( print_settings) {
        cout << "transform length: " << dataWHeader.size() << "\n";
    }
    tx.padData(dataWHeader);
    if( print_settings) {
        cout << "final pad length: " << dataWHeader.size() << "\n\n";
    }

    auto sliced_words = tx.emulateHiggsToHiggs(dataWHeader)[1];

    sliced_words.erase(sliced_words.begin(), sliced_words.begin()+header_length);

    const auto expected_sliced = sliced_words;

    cout << "Expected Words (" << expected_sliced.size()  << ")\n";
    cout << "Expected Frames (" << use_frames  << ")\n";
    if( printExpected ) {
        for(const auto w: expected_sliced) {
            cout << HEX32_STRING(w) << "\n";
        }
    }

    cout << "\n\n";


    ////////////////////////////////////////////////////////////////////////////////

    // Estimated rate is 2.7 us per frame


    int us = 200;

    us = 200;

    us = 2000;
    // us = 150;

    // us = 500;

    // us = 90;

    // us = 10;

    int automatic_quit_early = use_frames + 4; // measured in frames past 43


    // used below for mapmov things
    const uint32_t enabled_subcarriers = tx.enabled_subcarriers;
    const uint32_t modulation_schema = tx.modulation_schema;

    int now_late = 0; // in counters
    


    for(unsigned int i = 0; i < us; i++) {

        if( automatic_quit_early != -1 && gotBeyond43 >= automatic_quit_early ) {
            cout << "Automatically quitting run after " << i << " us because we got " << gotBeyond43 << " interesting frames!!\n\n\n";
            break;
        }

        if( i == 15 ) {
            t->wakeupSelfSync(); // no arguemnt is ok if us == 15
        }

        if( i == 40 ) {
            t->setCookedDataMode(2);
        }

        if( i == 41 ) {
            auto setCS11Role = [=](const uint32_t x) {
                std::vector<uint32_t> pack;
                pack = op("set", 10, x, GENERIC_OPERATOR_CMD);

                t->send_ring(RING_ADDR_TX_0, pack[0]);
                t->send_ring(RING_ADDR_TX_0, pack[1]);
            };

            setCS11Role(DUPLEX_ROLE_TX_0);
        }

        if( i == 119 ) {
            auto setLifetime = [=](const uint32_t x) {
                std::vector<uint32_t> pack;
                pack = op("set", 0, x, GENERIC_OPERATOR_CMD);

                t->send_ring(RING_ADDR_TX_0, pack[0]);
                t->send_ring(RING_ADDR_TX_0, pack[1]);
            };

            setLifetime(41+now_late); // add frames for delta to go down
        }

        if( i == 44 ) {
            // kicks off long process in eth which may actually deny ringbus processing
            // seems like this finishes at about 118
            t->send_ring(RING_ADDR_ETH, MAPMOV_MODE_CMD | MAPMOV_SUBCARRIER_320);
        }

        if( i == 120 ) {

            uint32_t timeslot = 0;
            uint32_t epoc = 0;

            uint32_t lifetime = 43; // subtract frames for delta to go down



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
    

    // t->allStreamDump();

    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) { tfp->close(); }


    // testing time

    // trim output to only relevant words
    auto sliced_out_relevant = sliced_out;
    sliced_out_relevant.resize(expected_sliced.size());

    bool match = expected_sliced == sliced_out_relevant;

    cout << "Two vectors match? " << (match?"yes":"no") << "\n";
    cout << "Vector Length expected: " << expected_sliced.size() << "\n";
    cout << "Vector Length      out: " << sliced_out_relevant.size() << "\n";


    assert(match);


    // Destroy model
    delete top; top = NULL;
    exit(0);
}
