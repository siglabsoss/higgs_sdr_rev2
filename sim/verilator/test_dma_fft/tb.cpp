#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <assert.h>
#include <verilated.h>
#include <sys/stat.h>
#include <fstream>
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"
#include "cpp_utils.hpp"
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
  const char* const flag = Verilated::commandArgsPlusMatch("trace");

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);

  unsigned int seed_start = std::time(0);

  unsigned int fixed_seed = 0; // Set to non zero to use

  // fixed_seed = 1525241634; // After 1900

  if(fixed_seed != 0) {
    seed_start = fixed_seed;
    std::cout << "Starting with hard-coded seed " << seed_start << "\n";
  } else {
    std::cout << "Starting with random seed " << seed_start << "\n";
  }

  srand(seed_start);

  preReset(top);

  t->reset(40);

  postReset(top);

  // seed finds it at 1900
  int us = 2100;

  std::vector<uint32_t> vin = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,
                               19,20,21,22,23,24,25,26,27,28,29,30,31};

  unsigned int injection = 0;

  bool test_failed = false;

  bool trace_started = false;

  unsigned int pick = rand(); // Get first random delay

  for(unsigned int i = 0; i < (us*500); i++) {
    if( (i % (20 + (pick%128)) ) == 0 ) {
      if(i > 500*100) {

        pick = rand(); // Get a new random delay, used above

        t->inStreamAppend("cs11in", vin );
        injection += (i + 3) % 7777;
        injection = injection % 2700;
      }
    }

    if(!trace_started && (i/500) == (1800) ) {
      if (flag && 0==strcmp(flag, "+trace")) {
        Verilated::traceEverOn(true);  // Verilator must compute traced signals
        std::cout << "Enabling waves into wave_dump.vcd...\n\n";
        tfp = new VerilatedVcdC;
        top->trace(tfp, 99);  // Trace 99 levels of hierarchy
        tfp->open("wave_dump.vcd");  // Open the dump file
        trace_started = true;

        // Tell higgs_helper we want to dump now
        t->tfp = tfp;
      }
    }

    if( (i % 1000) == 0 ) {
        unsigned int willbreak = 0;
        for(auto it = t->outs["ringbusout"]->data.begin();
            it != t->outs["ringbusout"]->data.end(); it++) {
          if( *it == 0xc000001 ) {
            willbreak = 1;
          }
        }
        if(willbreak) {
          cout << "Breaking early CS01 reports FAILURE at " << main_time
               << " (" << i << ")\n";
          test_failed = true;
          break;
        }

    }

    t->tick(1);
  }
  cs11_node_t* cs11_node = top->
                           tb_higgs_top->
                           cs11_top->
                           vex_machine_top_inst->
                           q_engine_inst->piston_inst->UNODE_NAME;
  cs01_node_t* cs01_node = top->
                           tb_higgs_top->
                           cs01_top->
                           vex_machine_top_inst->
                           q_engine_inst->piston_inst->UNODE_NAME;

  file_dump_T<cs11_node_t>(cs11_node, "cs11.out");
  file_dump_T<cs01_node_t>(cs01_node, "cs01.out");
  t->print_ringbus_out();
  t->outStreamDump("cs01out");

  file_dump_vec(t->outs["cs11out"]->data, "cs11_out.hex");

  assert(test_failed == false && "Test Failed, output dma glitch");

  std::cout << "All Tests Passed\n";
  std::cout << "Test ran for N us and did not glitch output DMA. Test Passed\n";

  // Final model cleanup
  top->final();

  // Close trace if opened
  if (tfp) {tfp->close();}

  // Destroy model
  delete top; top = NULL;

  exit(0);
}
