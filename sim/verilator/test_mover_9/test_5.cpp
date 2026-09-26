#include "handle_power.h"

// void add_power_entry(std::vector<uint32_t>& allRb, const uint64_t thresh, const std::vector<uint32_t>& rbs) {

//     std::vector<uint32_t> pack;

//     const uint32_t thresh_high = (thresh >> 32) & 0xffffffff;
//     const uint32_t thresh_low  = thresh & 0xffffffff;

//     pack = op("add", 0, thresh_high, GENERIC_OPERATOR_CMD);
//     allRb.push_back(pack[0]);
//     allRb.push_back(pack[1]);

//     pack = op("add", 0, thresh_low, GENERIC_OPERATOR_CMD);
//     allRb.push_back(pack[0]);
//     allRb.push_back(pack[1]);

//     for(unsigned i = 0; i < rbs.size(); i++ ) {
//         const uint32_t sel = (i==rbs.size()-1)?1:0;
//         const uint32_t word = rbs[i];

//         pack = op("add", sel, word, GENERIC_OPERATOR_CMD);
//         allRb.push_back(pack[0]);
//         allRb.push_back(pack[1]);
//     }
// }

int test5(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    srand(320948);
    preReset(top);
    t->reset(40);
    postReset(top);


    std::vector<uint32_t> pack;
    std::vector<uint32_t> allRb;



    DispatchGeneric dispatch;

    int dispatch_calls = 0;

    uint32_t call_p = 0;

    dispatch.setCallback([&dispatch_calls, &call_p](const uint32_t _sel, const uint32_t _val) {
        cout << "Call: " << dispatch_calls << " Variable index #" << _sel << " has value " << HEX32_STRING(_val) << "\n";

        if( dispatch_calls % 2 == 0) {
            call_p = _val;
        }

        if( dispatch_calls % 2 == 1) {

            uint64_t built = ((uint64_t)call_p)<<32 | _val;

            double asdb = POW_DB_FROM_MAG2(built);
            cout << "   " << asdb << "\n";

            // call_p = _val;
        }

        // assert(_sel == 15);

        // switch(calls) {
        //     case 0:
        //         assert(_val == 0x42);
        //         break;
        //     case 1:
        //         assert(_val == 0xdeadcccc);
        //         break;
        //     case 2:
        //         assert(_val == 0xdead0000);
        //         break;
        //     case 3:
        //         assert(_val == 0xdeadf00f);
        //         break;
        // }

        dispatch_calls++;
    }, GENERIC_READBACK_PCCMD);




    int calls_2 = 0;
    int calls_3 = 0;


     t->uarts["cs21"]->registerVmemImemTagCb([&](const uint32_t addr, const std::vector<uint32_t>& data) {
        
        cout << "                                  callback 2: " << calls_2 << "\n";

        for(const auto w : data) {
            cout << HEX32_STRING(w) << "\n";
        }

        cout << "\n\n";

        calls_2++;
       
    }, 2);


     t->uarts["cs21"]->registerVmemImemTagCb([&](const uint32_t addr, const std::vector<uint32_t>& data) {
        
        cout << "                                  callback 3: " << calls_3 << "\n";

        for(const auto w : data) {
            cout << HEX32_STRING(w) << "\n";
        }

        cout << "\n\n";

        calls_3++;
       
    }, 3);





    std::vector<uint32_t> packet;

    // packet = file_read_hex("../../data/mapmov_640_lin_1_mapped.hex");
    // packet = file_read_hex("../../data/mapmov_512_lin_1_mapped.hex");
    // packet = file_read_hex("../../data/mapmov_640_lin_3_mapped.hex");
    packet = file_read_hex("../../data/mapmov_640_lin_qam16_1_mapped.hex");

    // packet.resize(1024*2);

    // auto counter = get_counter(0,1024*2);
    // packet = get_counter(0,1024*2);

    // auto v = fftAirSubcarrierTransform(packet);
    auto v = fftAirSubcarrierTransform(packet);

    auto y = vectorInsertTrunk(v, TRUNK_FRAME_COUNTER, 1);


    // for(const auto w : y) {
    //     cout << HEX32_STRING(w) << "\n";
    //     // cout << w << "\n";
    // }

    // exit(0);


    int us = 250;
    us = 700;
    // us = 40;

    // us = 700;

    // us = 10;

    t->monitor_adc_in = true;
    t->printPowerEstimates(false);

    bool report_flag = true;

    auto injector = meteredRingbusSendUni<top_t>(t, allRb, 200, RING_ADDR_CS31, 11);

    for(unsigned int i = 0; i < us; i++) {

        if( i == 12 ) {
            // OMG
            // inserting here is MUCH better than inserting at start
            // this is because cs31 will take so long to start
            // we actually drop samples at the start
            // this means any small changes in startup time will totally affect output FFT
            // sampling time
            std::vector<uint32_t> zrs;
            // zrs.resize(802-1);
            // t->inStreamAppend("cs31in", zrs);
            // t->inStreamAppend("cs31in", packet);
        }

        // this wont work unless makefile has options added
        // if( i == 15 ) {
        //      t->send_ring(RING_ADDR_CS31, POWER_ESTIMATION_CMD | 0);
        // }

        // if( i == 115 || i == 620 ) {
        //      t->send_ring(RING_ADDR_CS31, POWER_ESTIMATION_CMD | 0);
        // }

        // injector(i);

        t->tick(500);
    }
    int jamming = -1;

    int error_count = 0;
    std::vector<uint32_t> stream_out;
    std::vector<uint32_t> sliced_out;

    auto &data = t->outs["cs21out"]->data;
    uint32_t j = 0;

    for(uint32_t i = 0; i < data.size(); /*empty*/ ) {
        uint32_t word = data[i];

        feedback_frame_t *v;
        feedback_frame_vector_t *vec;

        // Use uint32_t pointer arithmatic to and then do final cast before
        // assigning
        v = (feedback_frame_t*) (((uint32_t*)data.data())+i);
        vec = (feedback_frame_vector_t*)v;

        uint32_t* vs = (uint32_t*) v;

        bool error = false;
        
        if(word != 0) {

            if((i+1)+16 > data.size()) {
                std::cout << "Breaking loop at word #" << i
                          << " because header goes beyond received words\n";
                break;
            }

            error = false;
            uint32_t this_len = feedback_word_length((feedback_frame_t*)v, &error);
            if( !error ) {
                unsigned num_words = v->length - FEEDBACK_HEADER_WORDS;
                unsigned int* fbdata = ((unsigned int *)v)+FEEDBACK_HEADER_WORDS;

                if((i+1)+16+num_words > data.size()) {
                    std::cout << "Breaking loop at word #" << i
                              << " because body goes beyond received words\n";
                    break;
                }

                if( vec->type == FEEDBACK_TYPE_STREAM && vec->vtype == 1 ) {
                    for(size_t k = 0; k < num_words; k++) {
                        stream_out.push_back(fbdata[k]);
                    }
                }
                if( vec->type == FEEDBACK_TYPE_VECTOR && vec->vtype == FEEDBACK_VEC_DEMOD_DATA ) {
                    for(size_t k = 0; k < num_words; k++) {
                        const auto w = fbdata[k];
                        sliced_out.push_back(w);
                        cout << HEX32_STRING(w) << "\n";
                    }
                }


            }

            j++;
            if(jamming != -1) {
                std::cout << "Was Jamming was for " << jamming << endl;
                jamming = -1;
            }
        } else {
            if(jamming == -1) {
                jamming = 1;
            } else {
                jamming++;
            }
        }

        error = false;
        uint32_t advance = feedback_word_length((feedback_frame_t*)v, &error);

        if( error ) {
            std::cout << "Hard fail when parsing word #" << i << endl;
            advance = 1;
            error_count++;
        }

        if( advance != 1 ) {
            // if zero we want to print because that's wrong
            // if 1 we don't want to spam during a flushing section
            // if larger we want to print because they are few
            // std::cout << "Advance " << advance << endl;
        }

        i += advance;
    }

    cout << "\n";



    dumpVector32("fb_stream.hex", stream_out);



    for( int i = 0; i < stream_out.size(); i++ ) {
        std::cout << HEX32_STRING(stream_out[i]) << ",";
        if( i%64==63 ) {
            std::cout << endl;
        }
    }


    
    std::cout << "Hashes of stream frames:" << endl;


    size_t hash = 0;
    for( int i = 0; (i+64) < stream_out.size(); i+=64 ) {
        std::size_t sent_a = i;
        std::size_t sent_b = i + 64;

        std::vector<uint32_t> split_a(stream_out.begin() + sent_a, stream_out.begin() + sent_b);

        std::cout << "0x" << HEX_STRING(hashVector32(split_a)) << endl;
    }

    // Hash of stream_out 0xbe6ebe7cfe40dcbb
    // Hash of stream_out 0x8f53701aa8bb1d5d  (after removing tail)
    std::cout << "\nHash of total: 0x" << HEX_STRING(hashVector32(stream_out))
              << "\n";
    t->print_ringbus_out();
    t->allStreamDump();

    for(const auto w : t->outs["ringbusout"]->data ) {
        dispatch.gotWord(w);
    }


    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) { tfp->close(); }

    // Destroy model
    delete top; top = NULL;
    exit(0);
}
