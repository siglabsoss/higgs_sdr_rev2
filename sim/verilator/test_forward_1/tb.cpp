#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
// Include common routines

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

#include <fstream>
#include <math.h>

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"
#include <verilated_vcd_c.h>
#include "higgs_helper.hpp"


using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

#include "piston_c_types.h"
#include "vmem_types.h"
#include "schedule.h"
// #include "feedback_bus.h"
// #include "feedback_bus_types.h"

// using namespace siglabs::rb;



VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
    return main_time; // Note does conversion to real, to match SystemC
}



HiggsHelper<top_t>* t;

bool found_pass_fail() {
    for(const auto w : t->outs["ringbusout"]->data ) {
        if( (w & 0xffff0000) == (DMA_TORTURE_RESULTS | (RING_ENUM_CS02<<16)) ) {
            return true;
        }
    }
    return false;
}


void test8(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    t = new HiggsHelper<top_t>(top,&main_time,tfp); 



    preReset(top);

    t->reset(40);

    postReset(top);

    int us = 900;

    // us = 80;

    // std::vector<uint32_t> allRb = {
    //     // ETH_GET_STATUS_CMD
    // };
    // auto injector = meteredRingbusSendUni<top_t>(t, allRb, 90, RING_ADDR_CS20, 11);
    // injectReport(i);


    uint32_t pull1 = rand() & 0xffffff;
    
    uint32_t pull2 = (rand() % 15);  // if this chooses zero, it will not do random delays
    uint32_t pull3 = rand() % 400;
    uint32_t pull4 = rand() & 0xffffff;

    cout << "pull 1 0x" << HEX32_STRING(pull1) << "\n";
    cout << "pull 2 0x" << HEX32_STRING(pull2) << "\n";
    cout << "pull 3 0x" << HEX32_STRING(pull3) << "\n";
    cout << "pull 4 0x" << HEX32_STRING(pull4) << "\n";
    
    const int eth_start = 80;

    // bool b1 = false;

    for(unsigned int i = 0; i < us; i++) {

        if( i == eth_start + 5 ) {
            t->send_ring(RING_ADDR_TX_2, SEED_RANDOM_CMD | pull2);
        }

        if( i == eth_start + 15 ) {
            t->send_ring(RING_ADDR_TX_2, SEED_RANDOM_CMD | pull3);
        }

        if( i == eth_start + 25 ) {
            t->send_ring(RING_ADDR_TX_2, SEED_RANDOM_CMD | pull4);
        }

        if( i == eth_start + 35 ) {
            t->send_ring(RING_ADDR_TX_0, SEED_RANDOM_CMD | pull1);
        }

        if( found_pass_fail() ) {
            cout << "CS02 reported result at " << i << "\n";
            break;
        }

        // injector(i);

        t->tick(500);
    }

    int pass0 = 0;
    int pass1 = 0;
    bool pass2 = false;
    bool found2 = false;
    int first_result = 0;

    cout << "Ringbus got out" << endl;
    for(const auto w : t->outs["ringbusout"]->data ) {

        if( (w & 0xffff0000) == (DMA_TORTURE_RESULTS | (RING_ENUM_CS02<<16)) ) {
            int result = w & 0xffff;

            cout << "Got result " << result << "\n";

            pass2 = (result == 0);

            if( !found2 ) {
                first_result = result;
            }

            // if( w & )
            found2 = true;
        }

        // cout << "         mask 0x" << HEX_STRING( (w & 0xffff0000) ) << endl;
        cout << "0x" << HEX_STRING(w) << endl;
    }

    cout << "\n\n";



    // t->allStreamDump();

    top->final();

    // Close trace if opened
    if (tfp) { tfp->close(); }

    // Destroy model
    delete top; top = NULL;


    // assert here

    assert(found2 && "Didn't get any result, maybe test was too short");

    if( first_result != 0 ) {
        cout << "Got an error at the " << first_result << "th sample\n";
    }

    assert(pass2 && "Got incorrect result");


    exit(0);
}






int main(int argc, char** argv, char** env) {

    // if(const char* env_p = std::getenv("TEST_SELECT")) {
    //     unsigned int env_test_select = atoi(env_p);
    //     cout << "environment variable TEST_SELECT was set to: " << env_test_select << endl;
    //     test_select = env_test_select;
    // }


    unsigned int fixed_seed = 0; // set to non zero to use

    // fixed_seed = 1559862371;
    // fixed_seed = 1559862472;  // fails on old forward
    // fixed_seed = 1559862590;  // fails on new forward
    // fixed_seed = 1559895042;  // fails on new forward


    setup_random(fixed_seed);

    test8(argc, argv, env);

}