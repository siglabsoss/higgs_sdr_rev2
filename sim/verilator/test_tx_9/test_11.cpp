void test11(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);



    preReset(top);

    t->reset(40);

    postReset(top);





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

    int us = 750;
    // us = 50;

    uint32_t amount = 5;
    uint32_t direction = 1;
    int custom_size;

    // std::vector<uint32_t> vin = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

    bool found1 = false;
    bool found2 = false;

    // dump is here because we require size 1 due to a bug
    std::vector<uint32_t> dumb;
    dumb.resize(1);
    auto check_awake = feedback_vector_packet(
                FEEDBACK_VEC_STATUS_REPLY,
                dumb,
                FEEDBACK_PEER_8, FEEDBACK_DST_HIGGS);





    int calls_1 = 0;
    int calls_2 = 0;

     t->uarts["cs11"]->registerVmemImemTagCb([&](const uint32_t addr, const std::vector<uint32_t>& data) {
        
        cout << "                                  callback " << calls_1 << "\n";

        // for(const auto w : data) {
        //     cout << HEX32_STRING(w) << "\n";
        // }

        calls_1++;
       
    }, 1);

     t->uarts["cs11"]->registerVmemImemTagCb([&](const uint32_t addr, const std::vector<uint32_t>& data) {
        
        cout << "                                  callback " << calls_2 << "\n";

        // for(const auto w : data) {
        //     cout << HEX32_STRING(w) << "\n";
        // }

        cout << "\n\n";

        calls_2++;
       
    }, 2);



    std::vector<uint32_t> packet;

    // packet = file_read_hex("../../data/mapmov_512_lin_1.hex");
    // packet = file_read_hex("../../data/mapmov_640_lin_1.hex");
    // packet = file_read_hex("../../data/mapmov_640_lin_3.hex");
    // /mnt/overflow/work/software_parent_repo/higgs_sdr_rev2/sim/data/mapmov_640_lin_qam16_1.hex
    packet = file_read_hex("../../data/mapmov_640_lin_qam16_1.hex");

    // std::vector<uint32_t> packet2;

    // unsigned ii = 0;
    // for(auto w : packet) { 

    //     if( ii >= 16 ) {
    //         packet2.push_back(w);
    //     }

    //     ii++;
    // }

    // for(auto w : packet) { 
    //     cout << HEX32_STRING(w) << "\n";
    // }

    //          auto packet3 = 
    //                 feedback_vector_mapmov_scheduled_sized(
    //                     FEEDBACK_VEC_TX_USER_DATA, 
    //                     packet2, 
    //                     640, 
    //                     0,
    //                     FEEDBACK_DST_HIGGS,
    //                     0, // timeslot fillin, overwritten later
    //                     0,  // epoc fillin, overwritten later
    //                     FEEDBACK_MAPMOV_QPSK
    //                     );

    // for(auto w : packet3) { 
    //     cout << HEX32_STRING(w) << "\n";
    // }


    // exit(0);




    auto check_initial_settings = 70;

    auto go_0 = 80;
    auto go_1 = 165;


    // int enabled_subcarriers = 128;

    bool mm_found_0, mm_found_1, mm_found_2;
    mm_found_0 = mm_found_1 = mm_found_2 = false;
    
    uint32_t mm_enabled, mm_start, mm_end;
    mm_enabled = mm_start = mm_end = 0;

    for(unsigned int i = 0; i < us; i++) {

        // if(calls_2) {
        //     break;
        // }

        if(i == go_0 ) {
            // t->send_ring(RING_ADDR_ETH, MAPMOV_MODE_CMD | MAPMOV_SUBCARRIER_512_LIN);
            t->send_ring(RING_ADDR_ETH, MAPMOV_MODE_CMD | MAPMOV_SUBCARRIER_640_LIN);

        }

        if(i == go_1 ) {

            // cout << "data: " << endl;
            // for(auto it = data.begin(); it != data.end(); it++) {
            //     cout << "0x" << HEX_STRING(*it) << endl;
            // }
            // cout << endl << endl;;

            // data.resize(8);

            // const double enabled_subcarriers = 640;


            uint32_t epoc = 1;     // seq2
            uint32_t timeslot = 16+6; // seq

            set_mapmov_epoc_frames(packet, epoc, timeslot);


            t->inStreamAppend("cs11in", packet);
        }


        t->tick(500);
    }

    // if(!found1) {
    //     cout << "found1 was NOT set, something is wrong" << endl;
    // } else {
    //     cout << "found1 was set" << endl;
    // }

    // for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    //     if( *it == FEEDBACK_ALIVE ) {
    //         found2 = true; // look to see if fbbus is alive
    //     }
    // }

    // if(!found2) {
    //     cout << "found2 was NOT set, something is wrong" << endl;
    // } else {
    //     cout << "found2 was set" << endl;
    // }

    cout << "Ringbus got out" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
    }

    // cout << "CS10 sent to dac:" << endl;
    // for(auto it = t->outs[1].data.begin(); it != t->outs[1].data.end(); it++) {
    //   cout << "0x" << HEX_STRING(*it) << endl;
    // }

    t->allStreamDump();
    // cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
    // file_dump_T<cs20_node_t>(cs20_node,"cs20_vmem.out");

    // cout << "All Tests Passed" << endl;

    // assert(found1);
    // assert(found2);

    // Final model cleanup
    top->final();

    // Close trace if opened

    if (tfp) { tfp->close(); }

    // Destroy model
    delete top; top = NULL;
    //print_vector(output_vector);
    // Fin
    exit(0);
}
