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

  bool dump_hex = false;
  int us = 100;
  for(unsigned int i = 0; i < us; i++) {
    t->tick(500);
  }

  auto &data = t->outs["cs20out"]->data;

  cout << "CS20 gave us " << data.size() << " samples\n";

  for(unsigned int i = 0; i < 1024; i++) {
      unsigned int j = i % 1024;
      uint32_t expected = 0xff000000 + j;
      assert( data[i] == expected );
  }

  assert(data.size() > 2048);

  std::cout << "All Tests Passed\n";

  if (dump_hex) t->allStreamDump();

  // Final model cleanup
  top->final();

  // Close trace if opened

  if (tfp) { tfp->close(); }

  // Destroy model
  delete top; top = NULL;

  exit(0);
}
