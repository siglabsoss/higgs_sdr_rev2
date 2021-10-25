int test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    preReset(top);
    t->reset(40);
    postReset(top);

    int us = 40;

    bool report_flag = true;

    for(unsigned int i = 0; i < us; i++) {
        // if( t->ins["cs31in"]->data.size() == 0 ) {
        //     if(report_flag) {
        //         std::cout << "Adc data ran out at us " << i << endl;
        //         report_flag = false;
        //     }
        // } else if ( t->ins["cs31in"]->data.size() < 1024 ) {
        //     std::cout << "Adc data running low ("
        //               << t->ins["cs31in"]->data.size() << ") at us "
        //               << i << "\n";
        // }
        t->tick(500);
    }


    t->print_ringbus_out();
    // t->allStreamDump();


    bool found1 = false;

    for( const auto w : t->outs["ringbusout"]->data ) {
        // cout << "w " << HEX32_STRING(w) << "\n";
        if( w == (EDGE_EDGE_OUT | 1)  ) {
            found1 = true;
        }
    }



    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) { tfp->close(); }

    assert(found1);

    cout << "All Tests Passed\n";

    // Destroy model
    delete top; top = NULL;
    exit(0);
}
