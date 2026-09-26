
int test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    preReset(top);
    t->reset(40);
    postReset(top);


    int us = 110;


    for(unsigned int i = 0; i < us; i++) {
        t->tick(500);
    }

    unsigned found = 0;
    // unsigned illegal = 0;

    for(auto w : t->outs["ringbusout"]->data ) {

        switch(w) {
            case DEBUG_0_PCCMD:
                found++;
                break;
            default:
                break;
        }
        // std::cout << "0x" << HEX_STRING(w) << std::endl;
    }


    t->print_ringbus_out();
    t->allStreamDump();

    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) { tfp->close(); }

    assert( found == 1 && "Didn't find correct number of ook messages");
    // assert( illegal == 0 && "Should not find message with corrupted bits");

    cout << "All Tests Passed" << endl;

    // Destroy model
    delete top; top = NULL;
    exit(0);
}
