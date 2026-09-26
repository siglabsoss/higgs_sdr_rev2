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
#include <verilated_vcd_c.h>
#include "ringbus.h"
#include "ringbus2_pre.h"
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

std::string lookup_ringbus_enum(unsigned int v, bool upper = false);

int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 

  unsigned int seed_start = std::time(0);
  // Set to non zero to use
  unsigned int fixed_seed = 0;

  // fixed_seed = 1525984490;

  if(fixed_seed != 0) {
    seed_start = fixed_seed;
    std::cout << "Starting with hard-coded seed " << seed_start << "\n";
  } else {
    std::cout << "Starting with random seed " << seed_start << "\n";
  }


  srand(seed_start);

  // unsigned int pick1, pick2;

  // pick1 = rand() & 0x00ffffff;
  // pick2 = rand() & 0x00ffffff;

  // std::cout << "\nPick 1,2:  0x" << HEX_STRING(pick1) << ", 0x"
  //           << HEX_STRING(pick2) << "\n";

  preReset(top);

  t->reset(40);

  postReset(top);

  int us = 1700;

  unsigned us_base = 14;

  unsigned int seeds_sent = 0;

  unsigned int what_to_do = 0;

  for(unsigned int i = 0; i < us; i++) {


    t->tick(500);
  }

  t->tick(500*14);

  t->print_ringbus_out();

  unsigned int num_passed = 0;
  unsigned int total_fpga = 9;
  unsigned int recv_fpga = 2;

  std::cout << "Results:\n";
  for(auto it = t->outs["ringbusout"]->data.begin();
      it != t->outs["ringbusout"]->data.end(); it++) {
    if( (*it & 0xf0000000) == 0xf0000000 ) {
      unsigned int fpga = (*it & 0x000f0000) >> 16;
      unsigned int error = (*it & 0x0000ffff);
      std::cout << "FPGA " << fpga << " " << lookup_ringbus_enum(fpga)
                << " reported " << HEX_STRING(error) << "\n";
      if( error == 0 ){
        std::cout << "   passed\n";
        // FIXME: one fpga reporting 6 times can FOOL THIS!
        if( 
          fpga == RING_ENUM_CS01 ||
          fpga == RING_ENUM_CS02 ||
          fpga == RING_ENUM_CS32 ||
          fpga == RING_ENUM_CS22 ||
          fpga == RING_ENUM_CS21 ||
          fpga == RING_ENUM_CS20 ||
          fpga == RING_ENUM_CS12
          ) {
          num_passed++;
        }

      } else {
        std::cout << "   failed\n";
        exit(1);
      }
    }
    std::cout << "0x" << HEX_STRING(*it) << "\n";
  }

  assert(num_passed == 7 && "Some FPGA's did not pass internally");

  t->allStreamDump();

  // Final model cleanup
  top->final();

  // Close trace if opened
  if (tfp) { tfp->close(); }

  // Destroy model
  delete top; top = NULL;

  exit(0);
}
