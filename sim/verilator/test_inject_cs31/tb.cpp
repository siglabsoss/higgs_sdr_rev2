#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <bitset>
// Include common routines

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

#include <fstream>

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"

// also gets feedback_bus.h from riscv
#include "feedback_bus_tb.hpp"



#include <verilated_vcd_c.h>


#define RESET MIB_MASTER_RESET


#include "higgs_helper.hpp"

using namespace std;

typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

#include "piston_c_types.h"
#include "vmem_types.h"


VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
  return main_time; // Note does conversion to real, to match SystemC
}




int test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    // This helper is what I built to make this function easy
    // this handles reset.  You can register an arbitrary number of inputs
    // and outputs.
    // calling things like `inStreamAppend()` allows user to easily specify queue
    // input data which will be ticked over when tick is called
    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);


    // srand(320948);

    preReset(top);

    t->reset(40);

    postReset(top);

    t->monitor_adc_in = true;


    std::vector<uint32_t> long_counter = get_counter(0,1024*2);

    // std::vector<uint32_t> zrs;
    // zrs.resize(100);


    int us = 190; // estimate for 64*3*1280 to go through  1962


    bool pass0 = false;

    // bool report_flag = true;

    for(unsigned int i = 0; i < us; i++) {

        if( i == 65 ) {
            t->inStreamAppend("cs31in", long_counter);
        }

        // if( t->ins["cs31in"]->data.size() == 0 ) {
        //     if(report_flag) {
        //         cout << "Adc data ran out at us " << i << endl;
        //         report_flag = false;
        //     }
        // } else if ( t->ins["cs31in"]->data.size() < 1024 ) {
        //     cout << "Adc data running low (" << t->ins["cs31in"]->data.size() << ") at us " << i << endl;
        // }


        t->tick(500);
    }


    cout << "Ringbus got out" << endl;
    for(const auto w : t->outs["ringbusout"]->data) {

        if( (w & 0xff000000) == DEBUG_3_PCCMD ) {
            // cout << "found " << HEX_STRING(w) << "\n";
            pass0 = (w == DEBUG_3_PCCMD);
        }

        cout << "0x" << HEX_STRING(w) << endl;
    }


    t->allStreamDump();

      // Final model cleanup
    top->final();

      // Close trace if opened

    if (tfp) { tfp->close(); }

    assert(pass0 && "test bench was not able to provide input samples to cs31");

      // Destroy model
    delete top; top = NULL;
      //print_vector(output_vector);
      // Fin
    exit(0);
}

int main(int argc, char** argv, char** env) {
    uint32_t test_select = 0;

    if(const char* env_p = std::getenv("TEST_SELECT")) {
        unsigned int env_test_select = atoi(env_p);
        cout << "environment variable TEST_SELECT was set to: " << env_test_select << endl;
        test_select = env_test_select;
    }


    unsigned int fixed_seed = 0; // set to non zero to use
    setup_random(fixed_seed);

    switch(test_select) {
        case 0:
            test0(argc, argv, env);
            break;
        default:
            cout << "Invalid test selected" << endl;
            exit(1);
            break;
    }

}