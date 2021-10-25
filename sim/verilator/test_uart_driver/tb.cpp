#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <assert.h>
#include <verilated.h>
#include <sys/stat.h>
#include <fstream>
#include <math.h>
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
    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 

    preReset(top);

    t->reset(40);

    postReset(top);

    int simulation_time_us = 20;

    for(unsigned int i = 0; i < simulation_time_us; i++) {
        t->tick(500);
    }

    t->print_ringbus_out();

    top->final();

    if (tfp) { tfp->close(); }

    delete top; top = NULL;
} 