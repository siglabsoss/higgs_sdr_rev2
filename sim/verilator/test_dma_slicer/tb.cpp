#define VERILATE_TESTBENCH
#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))

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


eth_node_t * eth_node = top->\
                        tb_higgs_top->\
                        eth_top->q_engine_inst->piston_inst->unode28;


int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 

  unsigned int seed_start = std::time(0);

  unsigned int fixed_seed = 0; // set to non zero to use

  // fixed_seed = 1526435876;

  if(fixed_seed != 0) {
    seed_start = fixed_seed;
    cout << "starting with hard-coded seed " << seed_start << endl;
  } else {
    cout << "starting with random seed " << seed_start << endl;
  }

  srand(seed_start);

  preReset(top);

  t->reset(40);

  postReset(top);

  int us = 15;

  // Boot the processors
  t->tick(500*us);


  assert( t->outs["ringbusout"]->data.size() != 0 && "Didn't get any output");

  eth_node_t* eth_node = top->\
                         tb_higgs_top->\
                         eth_top->q_engine_inst->piston_inst->UNODE_NAME;

  file_dump_T<eth_node_t>(eth_node, "eth.vmem");

  std::cout << "Eth DMA out got " << t->outs["ringbusout"]->data.size()
            << " items\n";

  uint32_t ver [] = {0x0,
                     0x10001,
                     0x20002,
                     0x30003,
                     0x40004,
                     0x50005,
                     0x60006,
                     0x70007,
                     0x80008,
                     0x90009,
                     0xa000a,
                     0xb000b,
                     0xc000c,
                     0xd000d,
                     0xe000e,
                     0xf000f,
                     0x100010,
                     0x110011,
                     0x120012,
                     0x130013,
                     0x140014,
                     0x150015,
                     0x160016,
                     0x170017,
                     0x180018,
                     0x190019,
                     0x1a001a,
                     0x1b001b,
                     0x1c001c,
                     0x1d001d,
                     0x1e001e,
                     0x1f001f,
                     0x200020,
                     0x210021,
                     0x220022,
                     0x230023,
                     0x240024,
                     0x250025,
                     0x260026,
                     0x270027,
                     0x280028,
                     0x290029,
                     0x2a002a,
                     0x2b002b,
                     0x2c002c,
                     0x2d002d,
                     0x2e002e,
                     0x2f002f,
                     0x300030,
                     0x310031,
                     0x320032,
                     0x330033,
                     0x340034,
                     0x350035,
                     0x360036,
                     0x370037,
                     0x380038,
                     0x390039,
                     0x3a003a,
                     0x3b003b,
                     0x3c003c,
                     0x3d003d,
                     0x3e003e,
                     0x3f003f,
                     0x1010000,
                     0x3030202,
                     0x5050404,
                     0x7070606,
                     0x9090808,
                     0xb0b0a0a,
                     0xd0d0c0d,
                     0xf0f0e0e,
                     0x11111010,
                     0x13131212,
                     0x15151414,
                     0x17171616,
                     0x19191818,
                     0x1b1b1a1a,
                     0x1d1d1c1c,
                     0x1f1f1e1e,
                     0x21212020,
                     0x23232222,
                     0x25252424,
                     0x27272626,
                     0x29292828,
                     0x2b2b2a2a,
                     0x2d2d2c2c,
                     0x2f2f2e2e,
                     0x31313030,
                     0x33333232,
                     0x35353434,
                     0x37373636,
                     0x39393838,
                     0x3b3b3a3a,
                     0x3d3d3c3c,
                     0x3f3f3e3e,
                     0x41414040,
                     0x43434242,
                     0x45454444,
                     0x47474646,
                     0x49494848,
                     0x4b4b4a4a,
                     0x4d4d4c4c,
                     0x4f4f4e4e,
                     0x51515050,
                     0x53535252,
                     0x55555454,
                     0x57575656,
                     0x59595858,
                     0x5b5b5a5a,
                     0x5d5d5c5c,
                     0x5f5f5e5e,
                     0x61616060,
                     0x63636262,
                     0x65656464,
                     0x67676666,
                     0x69696868,
                     0x6b6b6a6a,
                     0x6d6d6c6c,
                     0x6f6f6e6e,
                     0x71717070,
                     0x73737272,
                     0x75757474,
                     0x77777776,
                     0x79797878,
                     0x7b7b7a7a,
                     0x7d7d7c7c,
                     0x7f7f7e7e,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x33221100,
                     0x77665544,
                     0xbbaa9988,
                     0xffeeddcc,
                     0x0,
                     0x0,
                     0x0,
                     0x0,
                     0x0,
                     0x0,
                     0x0,
                     0x0,
                     0x44444444,
                     0x44444444,
                     0x44444444,
                     0x44444444,
                     0x44444444,
                     0x44444444,
                     0x44444444,
                     0x44444444,
                     0x88888888,
                     0x88888888,
                     0x88888888,
                     0x88888888,
                     0x88888888,
                     0x88888888,
                     0x88888888,
                     0x88888888,
                     0xcccccccc,
                     0xcccccccc,
                     0xcccccccc,
                     0xcccccccc,
                     0xcccccccc,
                     0xcccccccc,
                     0xcccccccc,
                     0xcccccccc,
                     0xdeadbeef,
                     0xcafebabe,
                     0x1,
                     0x2,
                     0x3,
                     0x4,
                     0x5,
                     0x6,
                     0x7,
                     0x8,
                     0x9};

  for(std::size_t i = 0; i != t->outs["ringbusout"]->data.size(); i++) {
    if (t->outs["ringbusout"]->data[i] == ver[i]) {
    } else {
      std::cout << "0x" << HEX_STRING(t->outs["ringbusout"]->data[i])
           << " <<<< error. Expected " << HEX_STRING(ver[i]) << " \n";
      assert(0 &&  "Got wrong item from DMA");
    }
    cout << "0x" << HEX_STRING(t->outs["ringbusout"]->data[i]) << "\n";
  }

  assert(t->outs["ringbusout"]->data.size() == ARRAY_SIZE(ver) &&
         "Got wrong number of items from DMA");

  std::cout << "All Tests Passed\n";


  // Final model cleanup
  top->final();

  // Close trace if opened

  if (tfp) { tfp->close(); }  
  // Destroy model
  delete top; top = NULL;

  exit(0);
}

