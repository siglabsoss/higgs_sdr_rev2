
// void register_cb(ring_cb_t cb, uint32_t w, std::vector<ring_pair_cb_t>& table) {
//     table.push_back(ring_pair_cb_t(w,cb));
// }

// void dispatch_single(const std::vector<ring_pair_cb_t>& table, const uint32_t found) {

//     const unsigned int type = found & 0xff000000;
//     const unsigned int data = found & 0x00ffffff;

//     for(const auto row : table) {
//         uint32_t a;
//         ring_cb_t cb;
//         std::tie(a,cb) = row;

//         if( a == type ) {
//             cb(data);
//         }

//     }
// }

// void dispatch_ringbus(std::vector<uint32_t>& rb, std::vector<uint32_t>& now, const std::vector<ring_pair_cb_t>& table) {

//     if( rb == now ) {
//         return;
//     }
//     if( rb.size() > now.size() ) {
//         cout << "Warning dispatch_ringbus saw 'now' shrink\n";
//         return;
//     }

//     unsigned foundnew = now.size() - rb.size();

//     if( foundnew == 0 ) {
//         cout << "Warning dispatch_ringbus got same size different values, aborting\n";
//         return;
//     }

//     cout << "\n\n";
//     for(int i = rb.size(); i < now.size(); i++ ) {
//         const auto w = now[i];
//         cout << "new rb: " << HEX32_STRING(w) << "\n";
//         dispatch_single(table, w);
//     }

//     rb = now;


//     // for(const auto w : now ) {
//     //     cout << HEX32_STRING(w) << "\n";
//     // }
// }

uint64_t check_ideal(const std::vector<uint32_t> v, const unsigned shift) {
    uint64_t out;

    out = riscv_mag2_ideal(v, 0);

    out >>= shift;

    cout << "Ideal was: " << HEX64_STRING(out)  << "  ("<< out << ")" << "\n";

    return out;
}


void test1(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



    preReset(top);

    t->reset(40);

    postReset(top);

    t->monitor_adc_in = true;

    // tb inputs starts here
    // user can tick the clock for a period
    // append data to input streams, and look at output streams
    // modify negClock() and posClock() above
    // you can also insert for check streams from those functins()

    // delay between sending inputs

    // put a counter at the beginning of the ADC Data
    // since the FPGA's take a bit to boot, some of this will be dropped
    // std::vector<uint32_t> initial_counter = get_counter(0,1024*1);
    // t->inStreamAppend(2, initial_counter ); // give data for adc

    // read in some generated data we made. this will come after the counter
    // there are 117 ofdm frames in here
    // auto ideal_input = file_read_hex("sc_16_144_880_1008_bpsk.hex");


    // std::vector<uint32_t> long_counter = get_counter(0,1024*1024);
    // t->inStreamAppend("cs00in", long_counter);

    auto qam16_input = file_read_hex("../../data/cs10_out_qam16_rotated.hex");

    uint32_t sample_of_last_full_frame = qam16_input.size() - (qam16_input.size()%(1280));

    qam16_input.resize(sample_of_last_full_frame);

    cout << "read from file: " << qam16_input.size() << endl;

    unsigned saturations;
    qam16_input = gain_ishort_vector(1.0, qam16_input, saturations);

    if( saturations != 0 ) {
        cout << "gain procedure caused " << saturations << " samples to saturate!!\n";
    }

    std::vector<uint32_t> vec_input;

    std::vector<uint32_t> first;

    for(unsigned int i = 0; i < 1024; i++) {
        first.push_back(0x00010001);
    }

    int pull0 = rand() % 1024;


    int pull1 = pull0 + (rand() % 128);

    pull0 = 495;
    pull1 = 532;


    // cout << "pull0 " << pull0<< "\n";
    // cout << "pull1 " << pull1<< "\n";

    const int input_style = 1;

    if( input_style == 0 ) {

        for(int i = pull0; (i < 1024) && (i < pull1); i++) {

            // cout << "overwriting location " << i << "\n";
            if( i % 2 == 0 ) {
                first[i] = 0x00320002;
            } else {
                first[i] = 0x00020032;
            }
        }
    }

    if( input_style == 1) {
        uint32_t hilo;
        hilo = 0x0044 + 6; // ok for shift 4-5
        // hilo = 0x0060;

        for(int i = 32; i < 128; i++) {
            first[i] = (hilo&0xffff) << 16 | (hilo&0xffff);
        }
    }

    if( input_style == 2) {
        uint32_t hilo;
        hilo = 0x0144 + 6; // ok for shift 4-5
        // hilo = 0x0060;

        for(int i = 0; i < 1024; i++) {
            first[i] = (hilo&0xffff) << 16 | (hilo&0xffff);
        }
    }

    if( input_style == 3) {
        uint32_t hilo;
        // hilo = 0x7ff0;
        hilo = 0x1200;
        // hilo = 0x0060;
        const unsigned stride = 21;

        for(int i = 0; i < 1024; i++) {

            const double gain = 0.19 + ((i%(stride-3)) / (double)stride);

            const uint32_t original = (hilo&0xffff) << 16 | (hilo&0xffff);
            uint32_t postgain;
            bool sat;
            gain_ishort(gain, original, postgain, sat);

            // cout << "gain " << gain << "\n";
            if (false) {
                cout << HEX32_STRING(postgain);
                if( sat ) {
                    cout << " SAT ";
                }
                cout << "\n";
            }


            first[i] = postgain;
        }
    }

    if( input_style == 4) {
        uint32_t hilo;
        hilo = 0x7ffd; // absolute maximum
        // hilo = 0x3ef0; // less

        for(int i = 0; i < 1024; i++) {

            const uint32_t original = (hilo&0xffff) << 16 | (hilo&0xffff);
           
            first[i] = original;
        }
    }

    if( input_style == 5) {
        for(unsigned int i = 0; i < 1024; i++) {
            first[i] = qam16_input[i];
        }
    }

    // cout << "\n";
    unsigned shift = 18;
    auto res = check_ideal(first, shift);
    // cout << (double) (res << shift) << "    or   ";
    // cout << (double) res / 1E6 << "M " << "\n";
    // cout << "\n";



    // append and run test

    VEC_APPEND(vec_input, first);
    VEC_APPEND(vec_input, qam16_input);



    int us = 200;
    // us = 140;
    // us = 100;
    us = 80;
    // us = 10;



    t->registerRb([&](const uint32_t word) {
        cout << "ran for " << word << "\n";
    }, DEBUG_0_PCCMD);

    t->registerRb([&](const uint32_t word) {
        cout << "stalled for " << word << "\n";
    }, DEBUG_1_PCCMD);


    uint32_t mgr_base = 0;
    uint32_t output0_raw = 0;
    uint32_t output1_raw = 0;
    uint32_t output2_raw = 0;
    uint32_t output3_raw = 0;
    uint32_t mema_raw = 0;
    uint32_t memb_raw = 0;
    uint32_t memc_raw = 0;

    uint32_t variable_cfg_stage_0 = 0;
    uint32_t power_data = 0;


    t->registerRb([&](const uint32_t word) {
        static unsigned ptrs = 0;
        switch(ptrs) {
            case 0:
                mema_raw = word;
                break;
            case 1:
                memb_raw = word;
                break;
            case 2:
                memc_raw = word;
                break;
            default:
                cout << "!!! pointer type got unknown index " << word << "\n";
                return;
                break;
        }
        cout << "got pointer ["<< ptrs << "] " << word << "\n";
        ptrs++;
    }, DEBUG_2_PCCMD);


   
    for(unsigned int i = 0; i < us; i++) {

        // if( i == 10 ) {
        //     t->inStreamAppend("cs31in", vec_input);
        // }

        // // this wont work unless makefile has options added
        // if( i == 85 ) {
        //      t->send_ring(RING_ADDR_CS31, POWER_ESTIMATION_CMD | 0);
        // }

        // if(i == go_1 ) {
        //     t->send_ring(RING_ADDR_CS11, CHECK_BOOTLOAD_CMD | 1);
        // dispatch_ringbus(ringbus_prev, t->outs["ringbusout"]->data, ctable);

        t->tick(500);
    }





    cout << "\n\nRingbus got out" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
    }



    cout << "\n\n";
    t->allStreamDump();

    // cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
    // file_dump_T<cs20_node_t>(cs20_node,"foo.out");


    auto node = top->tb_higgs_top->cs31_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
    file_dump_T(node,"cs31_mem.out");


    if( false
        || mema_raw == 0
        || memb_raw == 0
        || memc_raw == 0
        ) {
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        cout << "\n";
        cout << " A pointer was set to 0, dumps will not be correct";
    }

    // auto memory = readVmemUnode(top->tb_higgs_top->cs31_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME, 0x6c70, 5);

    // unsigned mgr_base = 0x6c70;

    // auto memorya = t->readVmem("cs31", mgr_base, 1024);
    // auto memoryb = t->readVmem("cs31", mgr_base+1024, 5);

    // file_dump_vec(memorya, "mem_a.hex");

    // #define CCCV(x) (((x)-0x40000)/4)

    // values copied from cs31_top_symbols.txt
// auto output0_raw = CCCV(0x00048800); // g     O .vmem_constant_section 00000800 output_0
// auto output1_raw = CCCV(0x00041080); // g     O .vmem_constant_section 00000200 output_1
// auto output2_raw = CCCV(0x00041280); // g     O .vmem_constant_section 00000080 output_2
// auto output3_raw = CCCV(0x00041300); // g     O .vmem_constant_section 00000040 output_3

    auto output0 = t->readVmem("cs31", mema_raw, 16);
    auto output1 = t->readVmem("cs31", memb_raw, 16);
    auto output2 = t->readVmem("cs31", memc_raw, 1024);
    // auto output3 = t->readVmem("cs31", output3_raw, 16);

    file_dump_vec(output0, "mema.hex");
    file_dump_vec(output1, "memb.hex");
    file_dump_vec(output2, "memc.hex");
    // file_dump_vec(output3, "output3.hex");

    // auto varstage0 = t->readVmem("cs31", variable_cfg_stage_0, 16);
    // file_dump_vec(varstage0, "cfg0.hex");

    // auto pdata = t->readVmem("cs31", power_data, 1024);
    // file_dump_vec(pdata, "pdata.hex");



    // cout << "Mem A: \n";
    // for(const auto w : memorya) {
    //     cout << HEX32_STRING(w) << "\n";
    // }

    // cout << "Mem B: \n";
    // for(const auto w : memoryb) {
    //     cout << HEX32_STRING(w) << "\n";
    // }



    // cout << "auto mode: \n";
 
    // auto gott = t->foo<4>();

    // cout << gott[0] << "\n\n";


    // cout << "All Tests Passed" << endl;

    // Final model cleanup
    top->final();

    if (tfp) { tfp->close(); }


    // assert(found1);
    // assert(found2);


    // Close trace if opened

    // Destroy model
    delete top; top = NULL;
    //print_vector(output_vector);
    // Fin
    exit(0);
}
