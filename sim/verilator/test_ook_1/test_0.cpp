// const int32_t baseline_noise_tol = 20*0x000001e0/2;
// const int32_t baseline_noise_tol = 4*0x000001e0;
// const int32_t baseline_noise_tol = 40;
// const int32_t baseline_noise_tol = 80;
uint32_t add_noise(const uint32_t in, int32_t baseline_noise_tol) {
    int32_t noise_re = (rand() % (baseline_noise_tol)) - (baseline_noise_tol/2) ;
    int32_t noise_im = (rand() % (baseline_noise_tol)) - (baseline_noise_tol/2) ;

    int32_t re,im;
    re =  (int16_t)(in & 0xffff);
    im =  (int16_t)((in>>16) & 0xffff);

    re += noise_re;
    im += noise_im;

    // const uint32_t assembled = ((im << 16) & 0xffff) | (re&0xffff); // real only noise im is not used
    const uint32_t assembled = ((im << 16) & 0xffff0000) | (re&0xffff);

    return assembled;
}

void mm(uint32_t* v, uint32_t mmin, uint32_t mmax) {
    *v = (rand() % (mmax-mmin)) + mmin;
}

void pick2(int32_t* baseline_noise_tol, uint32_t* baseline_noise, uint32_t* pilot) {

    mm((uint32_t*)baseline_noise_tol, 1920, 0x7fff);
    *baseline_noise = 0;
    mm(pilot, 1000, 0x7fff);

    // int32_t delta = *pilot - (*baseline_noise + *baseline_noise_tol);
    // cout << "delta: " << delta << "\n";

    // float f = 1.42;
    // f = 1.21;


    // if( 
    //     ( (((int32_t)*baseline_noise_tol)*f) > ((int32_t)*pilot))
    //   )
    //  // 
    // {
    //     // re-roll
    //     pick(baseline_noise_tol, baseline_noise, pilot);
    // }
}

void pick(int32_t* baseline_noise_tol, uint32_t* baseline_noise, uint32_t* pilot) {
    float f;
    f = 1.21; // ok 
    // f = 1.15; // fail
    do {
        pick2(baseline_noise_tol, baseline_noise, pilot);
    }while(( (((int32_t)*baseline_noise_tol)*f) > ((int32_t)*pilot)));
}

void ppick(int32_t* baseline_noise_tol, uint32_t* baseline_noise, uint32_t* pilot) {
    cout << "bnt  : " << *baseline_noise_tol << "\n";
    cout << "noise: " << *baseline_noise << "\n";
    cout << "pilot: " << *pilot << "\n";
    cout << "\n";
}


uint32_t cmag(const uint32_t in) {
    int32_t re,im;
    re =  (int16_t)(in & 0xffff);
    im =  (int16_t)((in>>16) & 0xffff);

    return sqrt((re*re)+(im*im));
}

void explore_random_values(void) {

    int32_t baseline_noise_tol = 4*0x000001e0;

    uint32_t baseline_noise = 74;

    uint32_t pilot = 0x2000;



    std::vector<uint32_t> e_noise;
    std::vector<uint32_t> e_high;
    std::vector<uint32_t> e_low;

    std::vector<uint32_t> fails;
    for(unsigned j = 0; j < 50000; j++) {

        e_low.resize(0);
        e_high.resize(0);

        pick(&baseline_noise_tol, &baseline_noise, &pilot);
        ppick(&baseline_noise_tol, &baseline_noise, &pilot);

        const unsigned samples = 2000;

        for(unsigned i = 0; i < samples; i++) {
            uint32_t a;

            if( i < samples/2 ) {
                a = add_noise(baseline_noise, baseline_noise_tol);
                e_low.push_back(cmag(a));
            } else {
                a = add_noise(pilot, baseline_noise_tol);
                e_high.push_back(cmag(a));
            }

            e_noise.push_back(a);
        }

        uint32_t low_max  = *max_element(e_low.begin(), e_low.end());
        uint32_t high_min = *min_element(e_high.begin(), e_high.end());

        cout << "low  max: " << low_max << "\n";
        cout << "high max: " << high_min << "\n";

        if( low_max + 4 >= high_min ) {
            fails.push_back(j);
        }
    }

    // file_dump_vec(e_noise, "e_noise.hex", true);

    cout << "\n";

    for(auto w : fails) {
        cout << "Fail: " << w << "\n";
    }

    exit(0);


}


int test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    preReset(top);
    t->reset(40);
    postReset(top);


    int us = 190;

    // us = 300;
    us = 390;




    // explore_random_values();





    // us = 60;

    // t->registerRb([&](const uint32_t word) {
    //     const uint32_t who   = (word & 0x00f00000)>>16;
    //     const uint32_t dmode = (word & 0x000f0000)>>16;
    //     const uint64_t data =  word & 0x0000ffff;

    //     switch(dmode) {
    //         case 0:
    //             asmState = data;
    //             break;
    //         case 1:
    //             asmState |= (data << 16);
    //             cout << " Timer: " << HEX32_STRING(asmState) << "\n";
    //             break;
    //         default:
    //             cout << "UNKNOWN rb callback\n";
    //             break;
    //     }
    // }, TIMER_RESULT_PCCMD);


    for(unsigned int i = 0; i < us; i++) {

        if( i == 12 ) {
        }

        // if( i == 10 ) {
        //     t->send_ring(RING_ADDR_CS11, GET_TIMER_CMD);
        // }

        // if( i == 15 ) {
        //     uint32_t timer = 0x4e00;
        //     t->send_ring(RING_ADDR_CS01, SELF_SYNC_CMD | (timer>>8) );
        //     t->send_ring(RING_ADDR_CS02, SELF_SYNC_CMD | (timer>>8) );
        //     t->send_ring(RING_ADDR_CS11, SELF_SYNC_CMD | (timer>>8) );
        //     t->send_ring(RING_ADDR_CS12, SELF_SYNC_CMD | (timer>>8) );
        //     t->send_ring(RING_ADDR_CS20, SELF_SYNC_CMD | (timer>>8) );
        //     t->send_ring(RING_ADDR_CS21, SELF_SYNC_CMD | (timer>>8) );
        //     t->send_ring(RING_ADDR_CS22, SELF_SYNC_CMD | (timer>>8) );
        //     t->send_ring(RING_ADDR_CS31, SELF_SYNC_CMD | (timer>>8) );
        //     t->send_ring(RING_ADDR_CS32, SELF_SYNC_CMD | (timer>>8) );
        // }

        // if( i == 85 ) {
        //     t->send_ring(RING_ADDR_CS31, DUPLEX_SYNCHRONIZATION_CMD | 10 );
        // }

        t->tick(500);
    }

    unsigned found = 0;
    unsigned illegal = 0;

    for(auto w : t->outs["ringbusout"]->data ) {
        switch(w) {
            case 0x00005000:
            case 0xf500000f:
                found++;
                break;
            case 0x00005111:
            case 0xf5000010:
                illegal++;
                break;
            case 0x00005222:
            case 0xf5000011:
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

    assert( found == 4 && "Didn't find correct number of ook messages");
    assert( illegal == 0 && "Should not find message with corrupted bits");

    cout << "All Tests Passed" << endl;

    // Destroy model
    delete top; top = NULL;
    exit(0);
}
