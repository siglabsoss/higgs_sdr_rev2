

void test2(int argc, char** argv, char** env) {

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
    qam16_input = gain_ishort_vector(0.001, qam16_input, saturations);

    if( saturations != 0 ) {
        cout << "gain procedure caused " << saturations << " samples to saturate!!\n";
    }

    std::vector<uint32_t> vec_input;

    std::vector<uint32_t> first;

    for(unsigned int i = 0; i < 1024; i++) {
        first.push_back(0x00010001);
    }

    // first[0] = 0x00010001;

    VEC_APPEND(vec_input, first);
    VEC_APPEND(vec_input, qam16_input);


    // auto sadfqam16_input = file_read_hex("ben.hex");

    // for( int i = 0; i < 32; i++ ) {
    //     cout << HEX_STRING(qam16_input[i]) << "\n";
    // }
    // exit(0);


    // std::vector<uint32_t> zrs;
    // zrs.resize(802-1);
    // t->inStreamAppend("cs31in", zrs);



    int us = 200;

    // us = 140;
    us = 190;
    // us = 40;
    // us = 10;

   
    for(unsigned int i = 0; i < us; i++) {

        if( i == 10 ) {
            t->inStreamAppend("cs31in", vec_input);
        }
        if( i == 15 ) {
            t->send_ring(RING_ADDR_CS22, TRIGGER_EXFIL_CMD | 0);
        }

        // if(i == go_1 ) {
        //     t->send_ring(RING_ADDR_CS11, CHECK_BOOTLOAD_CMD | 1);
        // }

        t->tick(500);
    }





    cout << "Ringbus got out" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
    }



    t->allStreamDump();

    // cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
    // file_dump_T<cs20_node_t>(cs20_node,"foo.out");


    auto node = top->tb_higgs_top->cs31_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
    file_dump_T(node,"cs31_mem.out");

    // auto memory = readVmemUnode(top->tb_higgs_top->cs31_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME, 0x6c70, 5);

    const unsigned base = 0x6c70;

    auto memorya = t->readVmem("cs31", base, 1024);
    auto memoryb = t->readVmem("cs31", base+1024, 5);

    #define CCCV(x) (((x)-0x40000)/4)

    // values copied from cs31_top_symbols.txt
auto output0_raw = CCCV(0x00048800); // g     O .vmem_constant_section 00000800 output_0
auto output1_raw = CCCV(0x00041080); // g     O .vmem_constant_section 00000200 output_1
auto output2_raw = CCCV(0x00041280); // g     O .vmem_constant_section 00000080 output_2
auto output3_raw = CCCV(0x00041300); // g     O .vmem_constant_section 00000040 output_3

    auto output0 = t->readVmem("cs31", output0_raw, 512);
    auto output1 = t->readVmem("cs31", output1_raw, 128);
    auto output2 = t->readVmem("cs31", output2_raw, 32);
    auto output3 = t->readVmem("cs31", output3_raw, 16);

    file_dump_vec(output0, "output0.hex");
    file_dump_vec(output1, "output1.hex");
    file_dump_vec(output2, "output2.hex");
    file_dump_vec(output3, "output3.hex");

    // cout << "Mem A: \n";
    // for(const auto w : memorya) {
    //     cout << HEX32_STRING(w) << "\n";
    // }

    // cout << "Mem B: \n";
    // for(const auto w : memoryb) {
    //     cout << HEX32_STRING(w) << "\n";
    // }


    file_dump_vec(memorya, "mem_a.hex");

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
