#ifdef DISABLED_TEST

void test9(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



    preReset(top);

    t->reset(40);

    postReset(top);

    // tb inputs starts here
    // user can tick the clock for a period
    // append data to input streams, and look at output streams
    // modify negClock() and posClock() above
    // you can also insert for check streams from those functins()

    // delay between sending inputs

    int us = 105;

   
    auto check_initial_settings = 70;

    auto go_1 = 90;

    // how long to inject the 2nd check
    int inject_2nd_ok_check = 295; // can be 0 to 16 probably

    auto inject_mapmov_at = 84;

    auto inject_corruption = 270; // 165

    int adjust_offset = -10; // negative numbers means the packet will be sent later

    bool start_with_check = true;

    bool inject_recover_zeros = 275;

    bool do_recover = true;
    bool do_corrupt = false;

    // int enabled_subcarriers = 128;

    bool mm_found_0, mm_found_1, mm_found_2;
    mm_found_0 = mm_found_1 = mm_found_2 = false;
    
    uint32_t mm_enabled, mm_start, mm_end;
    mm_enabled = mm_start = mm_end = 0;

    for(unsigned int i = 0; i < us; i++) {

        if(i == go_1 ) {
            t->send_ring(RING_ADDR_CS11, CHECK_BOOTLOAD_CMD | 1);
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
    cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
    file_dump_T<cs20_node_t>(cs20_node,"cs20_vmem.out");

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
#endif