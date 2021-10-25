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

  preReset(top);

  t->reset(40);

  postReset(top);

  int simulation_time_us = 200;

  int bump = 10;
  int base = 10;

  for(unsigned int i = 0; i < simulation_time_us; i++) {
    if(i == base + bump*0) {
      t->send_ring(RING_ADDR_CS11, EDGE_EDGE_IN|0xa2);
    }
    if(i == base + bump*1) {
      t->send_ring(RING_ADDR_CS01, EDGE_EDGE_IN|0xa3);
    }
    if(i == base + bump*2) {
      t->send_ring(RING_ADDR_CS02, EDGE_EDGE_IN|0xa4);
    }
    if(i == base + bump*3) {
      t->send_ring(RING_ADDR_CS12, EDGE_EDGE_IN|0xa5);
    }
    if(i == base + bump*4) {
      t->send_ring(RING_ADDR_CS22, EDGE_EDGE_IN|0xa6);
    }
    if(i == base + bump*5) {
      t->send_ring(RING_ADDR_CS32, EDGE_EDGE_IN|0xa7);
    }
    if(i == base + bump*6) {
      t->send_ring(RING_ADDR_CS31, EDGE_EDGE_IN|0xa8);
    }
    if(i == base + bump*7) {
      t->send_ring(RING_ADDR_CS21, EDGE_EDGE_IN|0xa9);
    }
    if(i == base + bump*8) {
      t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN|0xb0);
    }
    t->tick(500);

  }

  t->tick(500*14); // flush (1 us per FPGA)
  t->print_ringbus_out();

  bool a,b,c,d,e,f,g,h,i;

  a = VECTOR_FIND(t->outs["ringbusout"]->data, 0x200a2);
  b = VECTOR_FIND(t->outs["ringbusout"]->data, 0x300a3);
  c = VECTOR_FIND(t->outs["ringbusout"]->data, 0x400a4);
  d = VECTOR_FIND(t->outs["ringbusout"]->data, 0x500a5);
  e = VECTOR_FIND(t->outs["ringbusout"]->data, 0x600a6);
  f = VECTOR_FIND(t->outs["ringbusout"]->data, 0x700a7);
  g = VECTOR_FIND(t->outs["ringbusout"]->data, 0x800a8);
  h = VECTOR_FIND(t->outs["ringbusout"]->data, 0x900a9);
  h = VECTOR_FIND(t->outs["ringbusout"]->data, 0xa00b0);

  assert(a &&
         b && 
         c && 
         d && 
         e && 
         f && 
         g && 
         h);

  std::cout << "All Tests Passed\n";


  // Final model cleanup
  top->final();

  // Close trace if opened

  if (tfp) { tfp->close(); }

  // Destroy model
  delete top; top = NULL;

  exit(0);
}
