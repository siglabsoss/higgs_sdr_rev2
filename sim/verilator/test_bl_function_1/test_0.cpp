int test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 
    preReset(top);
    t->reset(40);
    postReset(top);

    int us = 100;

    // us = 600;


    uint32_t pull1 = rand() & 0xffffff;
    // uint32_t pull2 = rand() & 0xf;
    cout << "pull 1 0x" << HEX32_STRING(pull1) << "\n";
    // cout << "pull 2 0x" << HEX32_STRING(pull2) << "\n";



    uint32_t enabled_tests;

    uint32_t results = 0;
    bool got_results = false;

    t->registerRb([&](const uint32_t word) {
        enabled_tests = word & 0x0fffffff;
        cout << "Found " << enabled_tests << " Tests\n";
    }, 0x60000000);


    t->registerRb([&](const uint32_t word) {
        results = word & 0x0fffffff;
        got_results = true;
        // cout << "Found " << enabled_tests << " Tests\n";
    }, 0x80000000);

    std::vector<uint32_t> run_times;

    t->registerRb([&](const uint32_t word) {
        uint32_t x = word & 0x0fffffff;
        run_times.push_back(x);
    }, 0x90000000);

    for(unsigned int i = 0; i < us; i++) {

        if( i == 1 ) {
            t->send_ring(RING_ADDR_ETH, SEED_RANDOM_CMD | pull1);

        }

        t->tick(500);
    }

    cout << "\nRingbus got out:\n";
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
    }
    cout << "\n";

    assert( t->outs["ringbusout"]->data[0] == 0xdeadbeef && "Test did not start");
    assert( got_results && "Test did report failure / success");


    // for(uint32_t i = 0; i < enabled_tests; i++) {
    //     bool pass = (results >> i) & 0x1;
    //     cout << "\n\n  Test " << i << ":\n";
    //     cout << "     ";
    //     cout << (pass ? "pass" : "FAILED");
    //     cout << "\n\n";
    // }

    uint32_t fails = 0;

    for(uint32_t i = 0; i < enabled_tests; i++) {
        bool pass = (results >> i) & 0x1;

        if( i < 10 ) {
            cout << " ";
        }
        cout << i+1 << "/" << enabled_tests << " Test ";

        if( i < 10 ) {
            cout << " ";
        }
        cout << "#" << i+1 << ": ...................................   " ;

         // "1/17 Test  #1: run_test_fec .....................   Passed"
        cout << (pass ? "Passed" : "FAILED");


        cout << "   ";


        if( i < run_times.size() ) {
            auto ck = run_times[i];

            if( ck < 10 ) {
                cout << " ";
            }
            if( ck < 100 ) {
                cout << " ";
            }
            if( ck < 1000 ) {
                cout << " ";
            }
            if( ck < 10000 ) {
                cout << " ";
            }
            if( ck < 100000 ) {
                cout << " ";
            }

            cout << ck << " clock cycles";
        }



        cout << "\n\n";

        if(!pass) {
            fails++;
        }
    }

    cout << fails << " tests failed out of " << enabled_tests << "\n";//0 tests failed out of 17
    cout << "\n";

    assert(fails == 0);


    cout << "All Tests Passed" << endl;


    if( false ) {
        auto mem = t->readVmem("eth", 0, 16*13);
        for(const auto w : mem) {
            cout << HEX_STRING(w) << "\n";
        }
    }




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
