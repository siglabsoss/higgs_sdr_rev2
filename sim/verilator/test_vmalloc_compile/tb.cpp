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
top_t* top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
  return main_time; // Note does conversion to real, to match SystemC
}

std::string translate_test1(unsigned int x) {
  switch(x) {
    case 0:
      return "next->NULL";
      break;
    case 0x1:
      return "";
    case 23556:
      return "next->node 0";
      break;
    case 0x40000:
      return "node 0";
      break;
    case 0x5c0c:
      return "next->node 1";
      break;
    case 0x48000:
      return "node 1";
      break;
    case 0x5c14:
      return "next->node 2";
      break;
    case 0x50000:
      return "node 2";
      break;
    case 0x5c1c:
      return "next->node 3";
      break;
    case 0x58000:
      return "node 3";
      break;
    case 0x5c24:
      return "next->node 4";
      break;
    case 0x60000:
      return "node 4";
      break;
    case 0x5c2c:
      return "next->node 5";
      break;
    case 0x68000:
      return "node 5";
      break;
    case 0x5c34:
      return "next->node 6";
      break;
    case 0x70000:
      return "node 6";
      break;
    case 0x5c3c:
      return "next->node 7";
      break;
    case 0x78000:
      return "node 7";
      break;
    default:
      return std::to_string(x);
      break;
  }
}

int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 

  srand(1);

  preReset(top);

  t->reset(40);

  postReset(top);

  unsigned int us = 1000;

  // Loop for 300 us or until CS20 is done
  for(unsigned int i = 0; i < us; i++) {
    t->tick(500); // tick 1 us
    if( t->pc_exicted_main("cs20") ) {
        std::cout << "Exiting main early at us " << i << "\n";
      break;
    }
  }

  t->tick(500*6);
  t->print_ringbus_out();

  assert(t->
         outs["ringbusout"]->
         data[t->outs["ringbusout"]->data.size()-1] == 0x01 &&
         "Some tests did not pass");  

  std::cout << "All Tests Passed\n";

  // Final model cleanup
  top->final();

  // Close trace if opened
  if (tfp) { tfp->close(); }

  // Destroy model
  delete top; top = NULL;

  exit(0);
}
