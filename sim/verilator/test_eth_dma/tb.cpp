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
#include <verilated_vcd_c.h>
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

  srand(1);

  preReset(top);

  t->reset(40);

  postReset(top);


  // Boot the processors
  t->tick(1000);

  assert( t->outs["ringbusout"]->data.size() != 0 && "didn't get any output");

  t->print_ringbus_out();

  assert(t->outs["ringbusout"]->data.size() == 4 &&
         "got wrong number of dma items");

  for(auto i =0; i < 4; i++) {
    assert(t->outs["ringbusout"]->data[i] == i+1 && "got wrong value in dma");  
  }

  std::cout << "All Tests Passed\n";

  // Final model cleanup
  top->final();

  // Close trace if opened

  if (tfp) { tfp->close(); }

  // Destroy model
  delete top; top = NULL;

  exit(0);
}

