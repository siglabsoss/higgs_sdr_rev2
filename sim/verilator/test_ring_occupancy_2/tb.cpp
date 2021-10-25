// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2017 by Wilson Snyder.
//======================================================================

#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
// Include common routines

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"







// // If "verilator --trace" is used, include the tracing class
#include <verilated_vcd_c.h>
// #include "dbus.hpp"

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))


#define RESET MIB_MASTER_RESET

#include "higgs_helper.hpp"


using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;



VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
  return main_time; // Note does conversion to real, to match SystemC
}






int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 


  srand(1);

  preReset(top);

  t->reset(40);

  postReset(top);


    unsigned got_asm = 0;

    uint32_t correct = 0;


    uint32_t asmState;
    
    t->registerRb([&](const uint32_t word) {
        const uint32_t dmode = (word & 0x00ff0000)>>16;
        const uint64_t data =  word & 0x0000ffff;

        const uint32_t flip =  (~word) & 0x00ffffff;

        cout << HEX32_STRING(flip) << "\n";

        const uint32_t expected = correct+1;

        if( expected == flip ) {
            correct++;
        }

    }, EDGE_EDGE_IN);



    unsigned us = 60;


    
    for( int i = 0; i < us; i++) {

        if( i == 2 ) {
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x01);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x02);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x03);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x04);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x05);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x06);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x07);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x08);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x09);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x0a);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x0b);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x0c);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x0d);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x0e);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x0f);
            t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x10);
        }

        t->tick(500);
    }

    cout << "correct: " << correct << "\n";


    t->print_ringbus_out();
    t->allStreamDump();


    // Final model cleanup
    top->final();

    // Close trace if opened

    if (tfp) { tfp->close(); }

    // Destroy model
    delete top; top = NULL;


    assert(correct == 16);

    cout << "All Tests passed\n";

    //print_vector(output_vector);
    // Fin
    exit(0);
}
