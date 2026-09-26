#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <assert.h>
#include <verilated.h>
#include <sys/stat.h>  // mkdir
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"
#include "cpp_utils.hpp"
#include <verilated_vcd_c.h>
#include "higgs_helper.hpp"

typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;
typedef Vtb_higgs_top_q_engine__pi3 q_engine_t;

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

  t->tick(1000*140);

  t->print_ringbus_out();

  assert(t->outs["ringbusout"]->data.size() == 4 &&
         "Got wrong number of DMA items");  
  assert(t->outs["ringbusout"]->data[0] == 0xdeadbeef &&
         "Test did not start");  
  assert(t->outs["ringbusout"]->data[3] == 0xF &&
         "Some tests failed, expected 0xF");  

  std::cout << "All Tests Passed\n";

  // Final model cleanup
  top->final();

  // Close trace if opened
  if (tfp) { tfp->close(); }

  // Destroy model
  delete top; top = NULL;

  exit(0);
}

