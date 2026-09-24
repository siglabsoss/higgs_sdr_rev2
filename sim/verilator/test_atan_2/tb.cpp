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


int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  // This helper is what I built to make this function easy
  // this handles reset.  You can register an arbitrary number of inputs
  // and outputs.
  // calling things like `inStreamAppend()` allows user to easily specify queue
  // input data which will be ticked over when tick is called
  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);


  srand(320948+12319);

  preReset(top);

  t->reset(40);

  postReset(top);

  int us = 500; 


  bool report_flag = true;

  for(unsigned int i = 0; i < us; i++) {

    if( i == 3 ) {
        // t->enable_adc_counter(false);
        // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, SFO_PERIODIC_ADJ_CMD |  0x3 ) ); // amount
    }

    // if( i == 25 ) {
    //   t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, SYNCHRONIZATION_CMD | 1 ) ); // Ask for a on-demand sync
    // }

    t->tick(500);

  }


  cout << "Ringbus got out" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }

    t->allStreamDump();


  // Final model cleanup
  top->final();

  // Close trace if opened

  if (tfp) { tfp->close(); }

  // Destroy model
  delete top; top = NULL;
  //print_vector(output_vector);
  // Fin
  exit(0);
}
