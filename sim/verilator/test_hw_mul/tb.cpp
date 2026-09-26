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

  srand(1);

  preReset(top);

  t->reset(40);

  postReset(top);

  unsigned int simulation_time_us = 90+40;
  t->tick(simulation_time_us*500);

  t->print_ringbus_out();

  assert(t->outs["ringbusout"]->data.size() != 0x0 &&
         "Testbench didn't get any ringbus");

  assert(t->outs["ringbusout"]->data[0] == 0xdead &&
         "Test did not begin");

  assert(t->outs["ringbusout"]->data.size() >= 5 &&
         "Test did not finish, or output too few items");

  unsigned int div_time = t->outs["ringbusout"]->data[1];
  unsigned int mul_time = t->outs["ringbusout"]->data[3];

  std::cout << std::endl;

  // 202, 84 for soft
  // 45,  12 for hardware
  std::cout << "Divide took: " << div_time << " clock cycles" << std::endl;
  std::cout << "Multiply took: " << mul_time << " clock cycles" << std::endl;
  std::cout << std::endl;

  assert( div_time < 60 );
  assert( mul_time < 40 );

  std::cout << "All Tests Passed (Hardware Multiplier is present)" << std::endl;


  // Final model cleanup
  top->final();

  // Close trace if opened

  if (tfp) { tfp->close(); }

  // Destroy model
  delete top; top = NULL;

  exit(0);
}
