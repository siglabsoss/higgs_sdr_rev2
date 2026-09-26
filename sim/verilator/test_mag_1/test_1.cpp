

void test1(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



    preReset(top);

    t->reset(40);

    postReset(top);


    // auto qam16_input = file_read_hex("../../data/cs10_out_qam16_rotated.hex");

    // uint32_t sample_of_last_full_frame = qam16_input.size() - (qam16_input.size()%(1280));

    // qam16_input.resize(sample_of_last_full_frame);

    // cout << "read from file: " << qam16_input.size() << endl;

    // unsigned saturations;
    // qam16_input = gain_ishort_vector(0.001, qam16_input, saturations);

    // if( saturations != 0 ) {
    //     cout << "gain procedure caused " << saturations << " samples to saturate!!\n";
    // }

    // std::vector<uint32_t> vec_input;

    // std::vector<uint32_t> first;

    // for(unsigned int i = 0; i < 1024; i++) {
    //     first.push_back(0x00010001);
    // }

    // // first[0] = 0x00010001;

    // VEC_APPEND(vec_input, first);
    // VEC_APPEND(vec_input, qam16_input);


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
    // us = 500;
    // us = 40;
    // us = 10;
    // us = 2000;

   
    for(unsigned int i = 0; i < us; i++) {

        // if( i == 10 ) {
        //     t->inStreamAppend("cs31in", vec_input);
        // }
        // if( i == 15 ) {
        //     t->send_ring(RING_ADDR_CS22, TRIGGER_EXFIL_CMD | 0);
        // }

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


    // const unsigned base = 0x6c70;

    // auto memorya = t->readVmem("cs31", base, 1024);
    // auto memoryb = t->readVmem("cs31", base+1024, 5);

    // #define CCCV(x) (((x)-0x40000)/4)

    // auto output0 = t->readVmem("cs31", output0_raw, 512);
    // auto output1 = t->readVmem("cs31", output1_raw, 128);
    // auto output2 = t->readVmem("cs31", output2_raw, 32);
    // auto output3 = t->readVmem("cs31", output3_raw, 16);

    // file_dump_vec(output0, "output0.hex");
    // file_dump_vec(output1, "output1.hex");
    // file_dump_vec(output2, "output2.hex");
    // file_dump_vec(output3, "output3.hex");

    // file_dump_vec(memorya, "mem_a.hex");

    // cout << "All Tests Passed" << endl;

    // Final model cleanup
    top->final();

    if (tfp) { tfp->close(); }


    // Destroy model
    delete top; top = NULL;
    exit(0);
}
