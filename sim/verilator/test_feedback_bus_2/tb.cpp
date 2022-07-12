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

  unsigned int seed_start = std::time(0);
  // Set to non zero to use
  unsigned int fixed_seed = 0;

  // First seed value after rng was working to give issue
  // fixed_seed = 1528613966;
  // fixed_seed = 1528640076;

  // if(fixed_seed != 0) {
  //   seed_start = fixed_seed;
  //   std::cout << "Starting with hard-coded seed " << seed_start << "\n";
  // } else {
  //   std::cout << "Starting with random seed " << seed_start << "\n";
  // }

  setup_random(fixed_seed);


  preReset(top);

  t->reset(40);

  postReset(top);

  int us = 120;

  // How many us to wait before first input
  unsigned us_base = 10;


  // Seed that is fed to dut
  uint32_t dut_seed = rand() & 0xffffff;

  for(unsigned int i = 0; i < us; i++) {
    if(i == 2) {
      // CS20 will wait for this seed before going
      t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | dut_seed);
    }

    t->tick(500);
  }


    int jamming = -1;
    int error_count = 0;

  for(uint32_t i = 0; i < t->outs["cs20out"]->data.size(); /*empty*/) {
      uint32_t word = t->outs["cs20out"]->data[i];
      feedback_frame_t *v;

      // Use uint32_t pointer arithmatic to and then do final cast before
      // assigning
      v = (feedback_frame_t*) (((uint32_t*)t->outs["cs20out"]->data.data())+i);

      uint32_t* vs = (uint32_t*) v;

      if(word != 0) {

          if((i+1)+16 > t->outs["cs20out"]->data.size()) {
              std::cout << "Breaking loop at word #" << i
                        << " because header goes beyond received words\n";
              break;
          }

          cout << "\n";
          print_feedback_generic((feedback_frame_t*)v);
          if(jamming != -1) {
              cout << "Was Jamming was for " << jamming << "\n";
              jamming = -1;
          }
      } else {
          if(jamming == -1) {
              jamming = 1;
          } else {
              jamming++;
          }
      }

      bool error = false;

      uint32_t advance = feedback_word_length((feedback_frame_t*)v, &error);

      if( error ) {
          std::cout << "Hard fail when parsing word #" << i << "\n";
          advance = 1;
          error_count++;
      }

      if( advance != 1 ) {
          // If zero we want to print because that's wrong
          // If 1 we don't want to spam during a flushing section
          // If larger we want to print because they are few
          cout << "Advance " << advance << "\n";
      }

      i += advance;
  }

  file_dump_vec(t->outs["cs20out"]->data, "cs20_out.hex");
  t->print_ringbus_out();
  assert(error_count == 0 && "Errors while parsing stream from CS20");


  std::cout << "All Tests Passed\n";

  // Final model cleanup
  top->final();

  // Close trace if opened
  if (tfp) {tfp->close();}

  // Destroy model
  delete top; top = NULL;

  exit(0);
}
