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

  // These will change if crt_standard.S changes
  unsigned int pc_done[3] = {0xE4, 0xE8, 0xEC};

  unsigned int us = 1000;

  // Loop for 300 us or until cs20 is done
  for(unsigned int i = 0; i < us; i++) {
    t->tick(500); // tick 1 us
    unsigned int cs20_pc = top->\
                           tb_higgs_top->\
                           cs20_top->\
                           vex_machine_top_inst->\
                           q_engine_inst->get_iBus_cmd_payload_pc();
    if(cs20_pc == pc_done[0] ||
       cs20_pc == pc_done[1] ||
       cs20_pc == pc_done[2]) {
      std::cout << "Breaking at " << main_time << "(" << i << " us) PC was "
                << cs20_pc << "\n";
      break;
    }
  }

  // Flush ringbusout
  t->tick(500*6);
  t->print_ringbus_out();

  assert(t->\
         outs["ringbusout"]->\
         data[t->outs["ringbusout"]->data.size()-1] == 0xff &&
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

