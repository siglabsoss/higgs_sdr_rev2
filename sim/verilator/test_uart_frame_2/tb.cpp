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

    int simulation_time_us = 160;

    uint32_t pull1 = rand() & 0xffffff;
    cout << "pull 1 0x" << HEX32_STRING(pull1) << "\n";

    uint32_t pull2 = rand() & 0xffffff;
    cout << "pull 2 0x" << HEX32_STRING(pull2) << "\n";

    for(unsigned int i = 0; i < simulation_time_us; i++) {
        if(i == 15) {
            t->send_ring(RING_ADDR_CS11, SEED_RANDOM_CMD | pull1);
        }
        if(i == 20) {
            t->send_ring(RING_ADDR_CS01, SEED_RANDOM_CMD | pull2);
        }
        t->tick(500);
    }


    unsigned fails11 = 0;
    uint8_t expected = 0;
    for(const auto c : t->uarts["cs11"]->data ) {
        if( c != expected ) {
            fails11++;
        }
        // cout << (int)c << "\n";
        expected++;
    }

    unsigned fails01 = 0;
    expected = 0;
    for(const auto c : t->uarts["cs01"]->data ) {
        if( c != expected ) {
            fails01++;
        }
        // cout << (int)c << "\n";
        expected++;
    }

    cout << "Got " << fails11 << " failures\n";
    cout << "Got " << fails01 << " failures\n";

    // t->print_ringbus_out();

    top->final();

    if (tfp) { tfp->close(); }

    assert(t->uarts["cs11"]->data.size() == 512);
    assert(t->uarts["cs01"]->data.size() == 512);

    assert( fails11 == 0);
    assert( fails01 == 0);

    delete top; top = NULL;
} 
