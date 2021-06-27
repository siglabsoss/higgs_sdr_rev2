#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <bitset>
#include <assert.h>
#include <verilated.h>
#include <sys/stat.h>
#include <fstream>
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"
#include "cpp_utils.hpp"
#include "feedback_bus_tb.hpp"
#include <verilated_vcd_c.h>
#include "higgs_helper.hpp"
#include "piston_c_types.h"
#include "vmem_types.h"

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
    return main_time; // Note does conversion to real, to match SystemC
}

int main(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);

    srand(320948);

    preReset(top);

    t->reset(40);

    postReset(top);

    std::vector<uint32_t> long_counter = get_counter(0,1024*2);

    bool dump_hex = false;
    int us = 280; // estimate for 64*3*1280 to go through  1962


    for(unsigned int i = 0; i < us; i++) {
        if(i == 80) {
            t->inStreamAppend("cs22in", long_counter);
        }
        t->tick(500);
    }

    t->print_ringbus_out();

    for(unsigned int i = 0; i < 0x50; i++) {
        assert( VECTOR_FIND(t->outs["ringbusout"]->data, i) );
    }

    if (dump_hex) t->allStreamDump();

    std::cout << "All Tests Passed\n";

    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) { tfp->close(); }

    // Destroy model
    delete top; top = NULL;

    exit(0);
}
