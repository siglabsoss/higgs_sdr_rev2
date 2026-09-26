
#include "AirPacket.hpp"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

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


// returns error
bool considerFeedbackRingbus(const std::vector<uint32_t>& rb, uint32_t& final_time) {
// , const int32_t must_have_gap 
    final_time = 0;

    // std::vector<uint32_t> marked;
    std::vector<uint32_t> times;

    for(const auto w : rb) {
        if( (w & 0xff000000) == 0x4e000000 ) {
            // marked.push_back(w);

            times.push_back(w & 0xffff);
        }
        // cout << HEX32_STRING(w) << "\n";
    }

    if( times.size() == 0 ) {
        cout << "ERROR: Ringbus related to type 19 detected\n";
        return true;
    }

    final_time = times[times.size()-1];


    std::vector<int32_t> gaps;

    uint32_t p = 0;
    unsigned i = 0;
    for(const auto w : times) {
        cout << "   time " << w << "\n";

        int32_t gap = ((i)==0) ? 5 : (w-p);

        if( gap > 5 ) {
            cout << " gap at index " << i << " of " << gap << "\n";
            gaps.push_back(gap);
        }
        
        i++;
        p = w;
    }

    if( gaps.size() == 0 ) {
        cout << "ERROR: no gaps detected, did userdata work?\n";
        return true;
    }

    

    return false;
}


// std::vector<uint32_t> sdaf = {
// 0x4e191100,
// 0x4e191105,
// 0x4e19110a,
// 0x4e19110f,
// 0x4e191114,
// 0x4e191119,
// 0x4e19111e,
// 0x4600000c,
// 0x4e191173,
// 0x4e191178};


int test3(int argc, char** argv, char** env) {



    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    preReset(top);
    t->reset(40);
    postReset(top);


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

    uint32_t tb_fb_parse_errors = 0;

    t->registerFbError([&](void) {
        tb_fb_parse_errors++;
    });

    uint32_t fb_parse_errors = 0;

    t->registerRb([&](const uint32_t word) {
        auto msg = getErrorStringFeedbackBusParse(TX_USERDATA_ERROR|word);
        cout << msg;
        fb_parse_errors++;
    }, TX_USERDATA_ERROR);

    t->registerRb([&](const uint32_t word) {
        cout << "cs20 lifetime ringbus " << HEX32_STRING(word) << "\n";
    }, 0);




    std::vector<std::tuple<uint32_t, uint32_t, std::vector<uint32_t>, std::vector<uint32_t> >> got;

    // catch all callbacks, filter
    t->registerFbCb([&](uint32_t t0, uint32_t t1, const std::vector<uint32_t>& header, const std::vector<uint32_t>& body) {
        const uint32_t t0t1 = (t0<<16) | t1;
        fb_times.emplace_back(t0t1,i);

        const uint32_t lifetime_32 = header[5];

        cout << "lifetime: " << HEX32_STRING(lifetime_32) << "  " << t0 << ", " << t1 <<  "\n";

        // for(auto w : header) {
        //     cout << HEX32_STRING(w) << "\n";
        // }
        // cout << "\n\n";

        // for(auto w : body) {
        //     cout << HEX32_STRING(w) << "\n";
        // }
        // cout << "-------\n\n";

        got.emplace_back(t1, lifetime_32, header, body);


    });


    t->customParseFeedbackBusEndpoint("ethout");

    ////////////////////////////////////////////////////////////////////////////////



    // how many ofdm frames of data will we use
    unsigned use_frames = 1 + (rand()%3);

    // setup global air object
    setupAir();


    const unsigned header_length = tx.sliced_word_count * 32;


    bool print_settings = true;

    // 20 words per frame
    const unsigned word_length = use_frames*tx.sliced_word_count;

    const std::vector<uint32_t> d = get_counter(0xcc000000, 0xcc000000+word_length);

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





















    ////////////////////////////////////////////////////////////////////////////////

    uint8_t plan[] = {1,2,0,0,0,0,0,1,2,0,0,1,2,0,2,1};

    const uint32_t plan_size = ARRAY_SIZE(plan);

    std::vector<uint32_t> plan2_expected;


    ////////////////////////////////////////////////////////////////////////////////
    ///
    ///
    ///  CS20 follows a plan, we check to see that we get the right messages according to this
    ///  
    ///  Each time tb sends a message (according to the 2nd plan alive_at) we get a rb out, tb checks that this is correct
    ///
    ///
    ///
    ///
    ///







    ////////////////////////////////////////////////////////////////////////////////

    // Estimated rate is 2.7 us per frame


    const uint32_t seed_0 = rand()%0xffffff;
    const uint32_t seed_1 = rand()%0xffffff;

    cout << "seed_0 " << HEX32_STRING(seed_0) << "\n";
    cout << "seed_1 " << HEX32_STRING(seed_1) << "\n";

    // was a problem seed before fixed #114
    // short seed works 1571048420

    int us = 200;

    us = 350;

    // used below for mapmov things
    const uint32_t enabled_subcarriers = tx.enabled_subcarriers;
    const uint32_t modulation_schema = tx.modulation_schema;

    const uint32_t max_rb = 3;

    const uint32_t max_kickoff = 153;
    const uint32_t last_alive = max_kickoff+80;

    std::vector<uint32_t> alive_at = {};

    if( alive_at.size() == 0 ) { // only randomize if not chosen above

        uint32_t rb_start = max_kickoff + (rand() % 120);

        while( (rb_start < (last_alive)) && (alive_at.size() < max_rb) ) {
            alive_at.push_back(rb_start);

            rb_start += 1 + (rand() % 35);
        }
    }


    // std::vector<uint32_t> mapmov_at = {20,44};
    std::vector<uint32_t> mapmov_at = {max_kickoff + (rand()%20) };
    // std::vector<uint32_t> mapmov_at = {};


    cout << "Check times: ";
    for(const auto w : alive_at ) {
        cout << w << ",";
    }
    cout << "\n";

    uint32_t dumb_tail = 0xdd000000;

    uint32_t check_sent = 0;

    for(i = 0; i < us; i++) {

        // was 60
        // if( false && (i == 8 || i == 36) ) { //. 26

        if( VECTOR_FIND(alive_at, i) ) {

            uint32_t check_body = 16 + (rand() % 237) ;

            // if( check_sent != 0 && ((rand() % 10) < 5) ) {
            //     check_body = 1 + rand() % 512;
            // }

            std::vector<uint32_t> dumb;
            for(unsigned i = 0; i < check_body; i++) {
                dumb.push_back(dumb_tail);
                dumb_tail++;
            }
            // dumb.resize(16);
            auto check_ok = feedback_vector_packet(
                FEEDBACK_VEC_STATUS_REPLY,
                dumb,
                0x00000001,
                0x88800000
                );

            cout << "Injecting check alive with length " << check_body << " at " << i << "\n";
            t->inStreamAppend("cs11in", check_ok);

            plan2_expected.push_back( TX_PARSE_GOT_VECTOR_PCCMD | 0x070000 );

            check_sent++;
        }


        if( VECTOR_FIND(mapmov_at, i) ) {

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


            // or we can set timeslot/epoc to 0 and use set_mapmov_epoc_frames()
        }

        if( i == 44 ) {
            // kicks off long process in eth which may actually deny ringbus processing
            // seems like this finishes at about 150
            t->send_ring(RING_ADDR_ETH, MAPMOV_MODE_CMD | MAPMOV_SUBCARRIER_320);
        }


        if( i == 5 ) {
            t->send_ring(RING_ADDR_CS11, SEED_RANDOM_CMD | seed_0);
            t->send_ring(RING_ADDR_CS20, SEED_RANDOM_CMD | seed_1);
        }

        if( i == (us-1) ) {
            t->customParseFeedbackPadZeros(1);
        }

        // cout << "uus " << i << "\n";

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
            case 0x00020006:
                nicename = "sliced data";
                break;
            case 0x00020019:
                nicename = "analog feedback";
                break;
        }

        cout << "t: " << uus << " " << HEX32_STRING(types) << "  " << nicename << "\n";

        final_time = uus;
    }

    // if( fb_times.size() == 0) {
    //     cout << "didn't get any fb bus at all" << "\n";
    //     exit(1);
    // }

    auto final_delta = us - final_time;
    cout << "final delta " << final_delta << "\n";

    // assert(final_delta < 60);


    cout << " --- Post Check --- " << "\n";
    const auto rb_copy = t->outs["ringbusout"]->data;


    uint32_t errors = 0;

    uint32_t plan_life = 0x1100;
    int erase;

    for(unsigned j = 0; j < plan_size; j++) {
        uint8_t run = plan[j];
        erase = -1;
        switch(run) {
            // check this via ringbus now
            // previously were sniffing at cs20 output
            case 1: {
                uint32_t expected = TX_PARSE_GOT_VECTOR_PCCMD | 0x190000 | plan_life;
                cout << "expected " << HEX32_STRING(expected) << "\n";
                bool found = VECTOR_FIND(rb_copy, expected);
                if( !found ) {
                    cout << "Plan " << j << " did not make it to cs11\n";
                    errors++;
                }
                break;
            }
            break;


            case 2: {
                bool found = false;
                for(unsigned k = 0; k < got.size(); k++) {
                    auto g = got[k];
                    uint32_t type;
                    uint32_t life;
                    std::vector<uint32_t> head;
                    std::vector<uint32_t> body;
                    std::tie(type,life,head,body) = g;

                    if( life != plan_life ) {
                        errors++;
                        cout << "lifetime error in " << j << "\n";
                    }

                    if( head[4] != 0x1 ) {
                        errors++;
                        cout << "type error in " << j << "\n";
                    }

                    erase = k;

                    found = true;
                    break;
                }

                if( erase >= 0 ) {
                    got.erase(got.begin()+erase);
                    cout << "Erasing one\n";
                }

                if( !found ) {
                    cout << "Did not find plan " << j << " item\n";
                    errors++;
                }

            }
            break;


        }

        plan_life++;
    }


    uint32_t rb_found = 0;

    for(unsigned j = 0; j < rb_copy.size(); j++) {
        const auto rb = t->outs["ringbusout"]->data[j];
        cout << HEX32_STRING(rb) << "   xxxx\n";

        if( rb == (TX_PARSE_GOT_VECTOR_PCCMD | 0x070000) ) {
            rb_found++;
        }
    }

    if( rb_found != plan2_expected.size() ) {
        cout << "Did not find corret number of plan2 ringbus "
        << rb_found << ", " << plan2_expected.size() << "\n";


        errors++;
    }

    if( fb_parse_errors ) {
        cout << "Got " << fb_parse_errors << " feedback parse errors " << "\n";

        errors += fb_parse_errors;
    }

    if( tb_fb_parse_errors ) {
        cout << "Higgs Helper Got " << tb_fb_parse_errors << " feedback parse errors " << "\n";

        errors += tb_fb_parse_errors;
    }

    uint32_t total_parse_errors = tb_fb_parse_errors + fb_parse_errors;


    cout << "errors " << errors << "\n";

    cout << "Parse errors " << total_parse_errors << "\n";

    assert(total_parse_errors == 0);



    // final lifetime of feedback frame (type 19) that cs11 got
    uint32_t finalLifetime = 0;
    bool consdierError = considerFeedbackRingbus(rb_copy, finalLifetime);

    assert(consdierError == false);

    
    assert( finalLifetime > 0x1105 );




    // Destroy model
    delete top; top = NULL;
    exit(0);
}
