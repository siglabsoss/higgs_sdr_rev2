
void test7(int argc, char** argv, char** env) {

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

    int us = 450;
    us = 100;



    auto go_1 = 60;


    for(unsigned int i = 0; i < us; i++) {

        if(i == go_1 ) {
            t->send_ring(RING_ADDR_CS11, 0xd0000000);
        }
        if(i == go_1 + 5) {
            t->send_ring(RING_ADDR_CS11, 0xd0000001);
        }
        if(i == go_1 + 10) {
            t->send_ring(RING_ADDR_CS11, 0xd0000002);
        }


        t->tick(500);
    }



    cout << "Ringbus got out" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
    }

    std::vector<uint32_t> needed = {
0xde000000,
0xde000001,
0xde000002,
0xde000003,
0xde000004,
0xde000005,
0xde000006,
0xde000007,
0xde000008,
0xde000009,
0xde00000a,
0xde00000b,
0xde00000c,
0xde00000d,
0xde00000e,
0xde00000f,
0xde000010,
0xde000011,
0xde000012,
0xde000013,
0xde000014,
0xde000015,
0xde000016,
0xde000017,
0xde000018,
0xde000019,
0xde00001a,
0xde00001b,
0xde00001c,
0xde00001d};
    
    for(const auto w : needed) {
        bool found = VECTOR_FIND(t->outs["ringbusout"]->data, w);
        assert(found);
    }



    t->allStreamDump();

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