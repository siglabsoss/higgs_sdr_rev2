int test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    srand(320948);
    preReset(top);
    t->reset(40);
    postReset(top);

    // put a counter at the beginning of the ADC Data
    // since the FPGA's take a bit to boot, some of this will be dropped
    std::vector<uint32_t> initial_counter = get_counter(0,1024*1);
    // read in some generated data we made. this will come after the counter
    // there are 117 ofdm frames in here
    // std::vector<uint32_t> ideal_input = file_read_hex("sc_16_144_880_1008_bpsk.hex");
    // std::vector<uint32_t> long_counter = get_counter(0,1024*1024);
    std::vector<uint32_t> qam16_input = file_read_hex("../../data/cs10_out_qam16_rotated.hex");

    uint32_t sample_of_last_full_frame = qam16_input.size() - (qam16_input.size()%(1280));

    qam16_input.resize(sample_of_last_full_frame);

    std::cout << qam16_input.size() << "\n";


    std::vector<uint32_t> zrs;
    zrs.resize(802-1);
    t->inStreamAppend("cs31in", zrs);
    t->inStreamAppend("cs31in", qam16_input);

    int us = 2500;

    bool report_flag = true;

    for(unsigned int i = 0; i < us; i++) {
        if( t->ins["cs31in"]->data.size() == 0 ) {
            if(report_flag) {
                std::cout << "Adc data ran out at us " << i << endl;
                report_flag = false;
            }
        } else if ( t->ins["cs31in"]->data.size() < 1024 ) {
            std::cout << "Adc data running low ("
                      << t->ins["cs31in"]->data.size() << ") at us "
                      << i << "\n";
        }
        t->tick(500);
    }
    int jamming = -1;

    int error_count = 0;
    std::vector<uint32_t> stream_out;

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

    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) { tfp->close(); }

    // Destroy model
    delete top; top = NULL;
    exit(0);
}
