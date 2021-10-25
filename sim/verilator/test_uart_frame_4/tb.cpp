#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <assert.h>
#include <verilated.h>
#include <sys/stat.h>
#include <fstream>
#include <math.h>
#include <cstdint>
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"
#include "cpp_utils.hpp"
#include <verilated_vcd_c.h>
#include "higgs_helper.hpp"
#include "piston_c_types.h"
#include "vmem_types.h"
#include "schedule.h"
#include "feedback_bus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"

typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
// Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
top_t* top = new top_t; 
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
    // Note does conversion to real, to match SystemC
    return main_time;
}

int main(int argc, char** argv, char** env) {
    unsigned int fixed_seed = 0; // set to non zero to use

    setup_random(fixed_seed);

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);

    preReset(top);
    t->reset(40);
    postReset(top);


    t->uarts["cs11"]->print = false;
    // t->uarts["cs11"]->debug_printf = true;

    int simulation_time_us = 40;



    unsigned fail = 0;
    unsigned calls_1 = 0;

    t->uarts["cs11"]->registerVmemImemTagCb([&](const uint32_t addr, const std::vector<uint32_t>& data) {
        
        cout << "                                  callback " << calls_1 << "\n";

        // unsigned start = 0;
        // switch(calls_1) {
        //     default:
        //     case 0:
        //         start = 0;
        //         break;
        //     case 1:
        //         start = 200;
        //         break;
        //     case 2:
        //         start = 0xff0000;
        //         break;
        //     case 3:
        //         start = 300;
        //         break;
        //     case 4:
        //         start = 777;
        //         break;
        //     case 5:
        //         start = 223440;
        //         break;
        //     case 6:
        //         start = 0xab9439;
        //         break;
        //     case 7:
        //         start = 0xccccc;
        //         break;
        // }

        // unsigned end = start+(1024*8);

        // auto ideal = get_counter(start,end);

        // if( ideal != data ) {
        //     cout << "ERROR: call " << calls_1 << " had wrong sequence" << "\n";
        //     fail++;
        // }

        calls_1++;
       
    }, 0);


// t->uarts["cs11"]->registerVmemImemTagCb([&](const uint32_t addr, const std::vector<uint32_t>& data) {
        
//         cout << "                                  callback tag 3 " << "\n";

//         for( const auto w : data) {
//             cout << HEX32_STRING(w) << "\n";
//         }

//     }, 3);

    t->uarts["cs11"]->registerImemBytesTagCb([&](const uint32_t addr, const std::vector<uint8_t>& data) {
        
        cout << "                            imem  callback " << 0 << "\n";

        for( const auto c : data ) {
            cout << c;
        }
        cout << "\n";
       
    }, 3);



    for(unsigned int i = 0; i < simulation_time_us; i++) {
        t->tick(500);
    }


    cout << "\n\n-------------\n\n";

    for(const auto w : t->uarts["cs11"]->print_history) {
        cout << w;
    }


    t->print_ringbus_out();

    top->final();

    if (tfp) { tfp->close(); }

    assert(fail == 0);

    delete top; top = NULL;
} 
