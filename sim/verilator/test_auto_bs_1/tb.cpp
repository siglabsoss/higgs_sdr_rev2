#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <assert.h>
#include <verilated.h>
#include <sys/stat.h>
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"
# include <verilated_vcd_c.h>
#include "higgs_helper.hpp"

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

    unsigned int fixed_seed = 0;
    setup_random(fixed_seed);

    preReset(top);
    t->reset(40);
    postReset(top);

    const unsigned int simulation_time_us = 40;

    // const uint32_t ideal0 = rand()&0xffffffff;
    // const uint32_t ideal1 = rand()&0xffffffff;


    // pass fail 0/1 addresses
    // const uint32_t a0 = 0x7ff8/4;
    // const uint32_t a1 = 0x7ffc/4;

    // auto write_fn = t->writeImemFunction("cs11");
    // write_fn(a0, ideal0);
    // write_fn(a1, ideal1);

    // t->writeImemWord("cs11", a0, ideal0);
    // t->writeImemWord("cs11", a1, ideal1);


    t->tick(simulation_time_us*500);

    // const uint32_t pf0 = t->readImemWords("cs11", a0, 1)[0];
    // const uint32_t pf1 = t->readImemWords("cs11", a1, 1)[0];

    // cout << "\n";
    // cout << "pf0 " << HEX32_STRING(pf0) << "\n";
    // cout << "pf1 " << HEX32_STRING(pf1) << "\n";

    t->print_ringbus_out();


    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) { tfp->close(); }

    // Destroy model
    delete top; top = NULL;

    // assert(ideal0 == pf0 && "higgs helper write read not working");
    // assert(ideal1 == pf1 && "higgs helper write read not working");

    // assert(t->outs["ringbusout"]->data.size() == 2 && "not correct # of rb");

    // const uint32_t rb0 = t->outs["ringbusout"]->data[0];
    // const uint32_t rb1 = t->outs["ringbusout"]->data[1];


    // assert(ideal0 == rb0 && "riscv is not able to read memory");
    // assert(ideal1 == rb1 && "riscv is not able to read memory");

    return 0;
}
