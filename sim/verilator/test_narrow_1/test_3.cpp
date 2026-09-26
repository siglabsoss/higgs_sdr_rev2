#include "verilate_multiple.hpp"
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




//////////////////////////////////////////////////////////////////////
// global for now

    std::vector<std::pair<uint32_t,uint32_t>> fb_times;

    unsigned int global_us = 0;

    // bool printExpected = false;
    bool printGot = false;
    uint32_t asmState = 0;



    // output from feedback bus activity
    unsigned got43Count = 0;
    bool captureEnabled = false;
    std::vector<uint32_t> sliced_out;
    unsigned gotBeyond43 = 0;


    std::vector<std::pair<uint32_t,uint32_t>> inserts;


    // Zhen, this is the lifetime_32 value we set, this can be changed to change what value we are on
    // as the simulation runs
    uint32_t lifetime_value = 115;

    // uint32_t bump_lims = 1;

    uint32_t insert_progress = 0;

    uint32_t enabled_subcarriers;
    uint32_t modulation_schema;

    std::vector<uint32_t> eq_zero;
    std::vector<uint32_t> eq_one;

    std::vector<uint32_t> fb_packet_eq_zero;
    std::vector<uint32_t> fb_packet_eq_one;

    // uint32_t next_pull = 0xffffffff;

//////////////////////////////////////////////////////////////////////


///
/// Custom function to make this test easier
/// 
void BoardBeginShared(HiggsHelper<top_t>* const t, const uint32_t higgs_id) {

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
        fb_times.emplace_back(t0t1,global_us);

        if( t0 != FEEDBACK_TYPE_VECTOR || t1 != FEEDBACK_VEC_DEMOD_DATA ) {
            return;
        }
        const uint32_t lifetime_32 = header[5];
        gotSlicedData(header, body);
    });





    cout << "lifetime_value: " << lifetime_value << "\n";
    // cout << "bump limit: " << bump_lims << "\n";





    for(uint32_t k = 0; k < 1; k++) {
        inserts.emplace_back(RING_ADDR_RX_0, COOKED_DATA_TYPE_CMD | 2);
        inserts.emplace_back(RING_ADDR_RX_1, COOKED_DATA_TYPE_CMD | 2);
        inserts.emplace_back(RING_ADDR_RX_2, COOKED_DATA_TYPE_CMD | 2);
        inserts.emplace_back(RING_ADDR_RX_4, COOKED_DATA_TYPE_CMD | 2);
        inserts.emplace_back(RING_ADDR_TX_0, COOKED_DATA_TYPE_CMD | 2);
        inserts.emplace_back(RING_ADDR_TX_1, COOKED_DATA_TYPE_CMD | 2);
    }



    enabled_subcarriers = tx.enabled_subcarriers;
    modulation_schema = tx.modulation_schema;


    // eq_zero
    // eq_one
    // fb_packet_eq_zero
    // fb_packet_eq_one
    for(unsigned i = 0; i < 1024; i++) {
        uint32_t v = 0;
        if(i == 1022 || (i>=16 && i < 80) ) {
            v = 0x7fff;
        }
        eq_zero.push_back(v);
    }
    for(unsigned i = 0; i < 1024; i++) {
        uint32_t v = 0;
        if( (i >= (1024-(32*4))) && (i%4==2) ) {
            v = 0x7fff;
        }
        eq_one.push_back(v);
    }


    fb_packet_eq_zero = feedback_vector_packet(
        FEEDBACK_VEC_TX_EQ,
        eq_zero,
        1,
        FEEDBACK_DST_HIGGS);
    fb_packet_eq_one = feedback_vector_packet(
        FEEDBACK_VEC_TX_EQ_1,
        eq_one,
        1,
        FEEDBACK_DST_HIGGS);


}


///
/// Required with verilate multiple higgs
/// 
void Board0Begin(HiggsHelper<top_t>* const t) {
    BoardBeginShared(t,0);
}

///
/// Required with verilate multiple higgs
/// 
void Board1Begin(HiggsHelper<top_t>* const t) {
    BoardBeginShared(t,1);
}



///
/// Custom function to make this test easier
/// 
int BoardLoopShared(HiggsHelper<top_t>* const t, const uint32_t us, const uint32_t higgs_id) {
    global_us = us;



    if( us == 15 ) {
        t->wakeupSelfSync(); // no arguemnt is ok if us == 15
    }


    if( us == 40 ) {
        // make a lambda so we can set the lifetime32
        auto setLifetime = [=](const uint32_t x) {

            // this is the same as sjs.dsp.op()
        if(higgs_id == 1){

            std::vector<uint32_t> pack_0 = op("set", 0, x, GENERIC_OPERATOR_CMD);

            t->send_ring(RING_ADDR_TX_0, pack_0[0]);
            t->send_ring(RING_ADDR_TX_0, pack_0[1]);

            std::vector<uint32_t> pack_1 = op("set", 0, x+1, GENERIC_OPERATOR_CMD);

            t->send_ring(RING_ADDR_RX_0, pack_1[0]);
            t->send_ring(RING_ADDR_RX_0, pack_1[1]);

        }

        if(higgs_id == 0){

            std::vector<uint32_t> pack_0 = op("set", 0, x, GENERIC_OPERATOR_CMD);

            t->send_ring(RING_ADDR_TX_0, pack_0[0]);
            t->send_ring(RING_ADDR_TX_0, pack_0[1]);

            std::vector<uint32_t> pack_1 = op("set", 0, x+1, GENERIC_OPERATOR_CMD);

            t->send_ring(RING_ADDR_RX_0, pack_1[0]);
            t->send_ring(RING_ADDR_RX_0, pack_1[1]);

        }
            

           
        };

        // call our lambda, with the lifetime counter we would like to set
        setLifetime(lifetime_value); // add frames for delta to go down
    }

    auto ff =  DUPLEX_ROLE_RX;

    // Zhen:
    // This should be changed to set the role as we want
    // you also need to set the role of each fpga
    // Change this code (it was copied from s-modem EventDspFsmTx)
                //
                // uint32_t role_enum;
                //
                // role_enum = DUPLEX_ROLE_RX;
                // role_enum = DUPLEX_ROLE_TX_0;
                //
                // op(RING_ADDR_TX_PARSE    , "set", 10, role_enum);
                // op(RING_ADDR_TX_FFT      , "set", 10, role_enum);

                // op(RING_ADDR_RX_FFT      , "set", 10, role_enum);
                // op(RING_ADDR_RX_FINE_SYNC, "set", 10, role_enum);
                // op(RING_ADDR_RX_EQ,        "set", 10, role_enum);
                // op(RING_ADDR_RX_MOVER,     "set", 10, role_enum);
    //
    // this is disabled so you can fix it
    if( true && (us == 45) ) {

        auto setHiggsRole = [=](const uint32_t x) {
            // this is the same as sjs.dsp.op()
            std::vector<uint32_t> pack = op("set", 10, x, GENERIC_OPERATOR_CMD);

            t->send_ring(RING_ADDR_TX_0, pack[0]);
            t->send_ring(RING_ADDR_TX_0, pack[1]);

            t->send_ring(RING_ADDR_TX_1, pack[0]);
            t->send_ring(RING_ADDR_TX_1, pack[1]);

            t->send_ring(RING_ADDR_RX_FFT, pack[0]);
            t->send_ring(RING_ADDR_RX_FFT, pack[1]);

            t->send_ring(RING_ADDR_RX_FINE_SYNC, pack[0]);
            t->send_ring(RING_ADDR_RX_FINE_SYNC, pack[1]);

            t->send_ring(RING_ADDR_RX_EQ, pack[0]);
            t->send_ring(RING_ADDR_RX_EQ, pack[1]);

            t->send_ring(RING_ADDR_RX_MOVER, pack[0]);
            t->send_ring(RING_ADDR_RX_MOVER, pack[1]);
        };

        // make a lambda to set the role of cs11
        if(higgs_id == 0){

            setHiggsRole(DUPLEX_ROLE_RX);

        }else if(higgs_id == 1){

            setHiggsRole(DUPLEX_ROLE_TX_0);

        }
    }

    if( us == 80 ) {
         t->inStreamAppend("cs11in", fb_packet_eq_zero);
    }

    if( us == 85 ) {
         t->inStreamAppend("cs11in", fb_packet_eq_one);
    }



    // we enter cooked data mode 2 here
    ////////////////////////////////// should uncomment later//////////////////////????????????????
    if( us == 90 ) {
        t->setCookedDataMode(2);
    }


    if(us ==92){
        t->send_ring(RING_ADDR_CS32, EQ_DATA_RX_CMD | 2 );
    }

    // if( us == 90 ) {
    //     uint32_t bump = 1+(rand() % bump_lims);
    //     next_pull = i + bump;
    // }

    // if( i == next_pull ) {

    //     if( insert_progress < inserts.size() ) {
    //         auto xxx = inserts[insert_progress];
    //         uint32_t ttl, rb;
    //         std::tie(ttl,rb) = xxx;

    //         cout << "sending mode " << HEX32_STRING(rb) << " to " << ttl << " at " << i << "\n";
    //         t->send_ring(ttl, rb);

    //         insert_progress++;
    //     }

    //     uint32_t bump = 1+(rand() % bump_lims);
    //     next_pull = i + bump;
    // }


    return 0;
}


///
/// Required with verilate multiple higgs
/// 
int Board0Loop(HiggsHelper<top_t>* const t, const uint32_t us) {
    // cout << "Board 0: us " << us << "\n";
    return BoardLoopShared(t, us, 0);
}

///
/// Required with verilate multiple higgs
/// 
int Board1Loop(HiggsHelper<top_t>* const t, const uint32_t us) {
    // cout << "Board 1: us " << us << "\n";
    return BoardLoopShared(t, us, 1);
}


///
/// Required with verilate multiple higgs
///
/// Called once time after all us have been ticked
/// Return non 0 to fail
int Board0Final(HiggsHelper<top_t>* const t, const uint32_t us) {
    t->print_ringbus_out();
    

    // Zhen, as of now allStreamDump() can only be called from one Board
    // the file names would conflict if you call it twice
    t->allStreamDump();
    return 0;
}

///
/// Required with verilate multiple higgs
/// 
int Board1Final(HiggsHelper<top_t>* const t, const uint32_t us) {
    t->print_ringbus_out();
    
    // Zhen, as of now allStreamDump() can only be called from one Board
    // the file names would conflict if you call it twice
    t->allStreamDump();




    // use assert2 not assert


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

    // Zhen all of this can be changed
    if( fb_times.size() == 0) {
        cout << "didn't get any fb bus at all" << "\n";
        assert2(0);
    }

    auto final_delta = us - final_time;
    cout << "final delta " << final_delta << "\n";

    // assert2(final_delta < 60);

    return 0;
}




int test3(const int argc, char** const argv, char** const env) {



    ////////////////////////////////////////////////////////////////////////////////

    // Estimated rate is 2.7 us per frame


    uint32_t runtime_us =600;


    return VerilateMultiple(
        argc,
        argv,
        env,
        runtime_us
    );
}
