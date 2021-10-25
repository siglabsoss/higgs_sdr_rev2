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


    t->uarts["cs11"]->print = true;
    // t->uarts["cs11"]->print_chars = true;

    int simulation_time_us = 40;



    // t->uarts["cs11"]->vmemTagToHexFile(0, "counter_");
    // t->uarts["cs11"]->vmemTagToHexFile(1, "small_");

    unsigned fail = 0;
    // unsigned calls_1 = 0;
    constexpr unsigned expected_calls = 6;

  

    unsigned calls_imem = 0;

    t->uarts["cs11"]->registerImemBytesTagCb([&](const uint32_t addr, const std::vector<uint8_t>& data) {
        
        cout << "                            imem  callback " << calls_imem << "\n";

        std::vector<uint8_t> ideal;
        // a = {2,3,4};

        switch(calls_imem) {
            default:
            case 0:
                ideal = {20};
                break;
            case 1:
                ideal = {21,22};
                break;
            case 2:
                ideal = {23,24,25};
                break;
            case 3:
                ideal = {26,27,28,29};
                break;
            case 4:
                ideal = {0x30,0x31,0x32,0x33,0x34};
                break;
            case 5:
                ideal = {0x40,0x41,0x42};
                break;
            case 6:
                ideal = {0x50,0x51,0x52,0x53,0x54,0x55,0x56};
                break;
        }

        if( data != ideal ) {
            cout << "ERROR: imem value wrong in call " << calls_imem << "\n";
            fail++;
        }


        calls_imem++;
       
    }, 2);


    for(unsigned int i = 0; i < simulation_time_us; i++) {
        t->tick(500);
    }




    t->print_ringbus_out();

    top->final();

    if (tfp) { tfp->close(); }

    assert(fail == 0);
    assert(calls_imem == (expected_calls+1) && "did not get correct number of calls");

    delete top; top = NULL;
} 
