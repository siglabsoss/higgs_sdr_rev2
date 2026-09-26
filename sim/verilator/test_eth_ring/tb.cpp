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
#include "unit_test_ring.h"

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

ring_unit_counters_t dump_print(helper_t *t, uint32_t enumm) {
  const int gap = 1100;
  uint32_t report_enum;
  uint32_t report_error;
  ring_unit_counters_t report_counters;
  const uint32_t addr = ring_unit_lookup_addr(enumm);

  // Feed command in and wait
  t->send_ring(addr, ring_t_enc(4,0,0,0));
  t->tick(gap*10);

  std::cout << "Dump " << get_enum_string(enumm) << " at " << main_time << "\n";

  // Convert response to an object we can look at
  report_error = vector_to_ring_unit_counters(t->outs["ringbusout"]->data,
                                              &report_enum,
                                              &report_counters);

  assert(report_error == 0);

  print_ring_unit_counters_t(report_enum, report_counters);

  cout << "\n\n";

  t->outs["ringbusout"]->data.resize(0); // erase what we printed

  return report_counters;
}

int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 
  

  unsigned int seed_start = std::time(0);

  unsigned int fixed_seed = 0; // Set to non zero to use
  // Found the bug the first time (may 24, 2018)
  // fixed_seed = 1527206951;
  // Not really that special, but was using during fixing another eth bug
  // fixed_seed = 1529900455;
  
  setup_random(fixed_seed);

  t->warn_ringbus_erased = false;

  const unsigned int pull0 = rand() & 0xffffffff;
  const unsigned int pull1 = rand() & 0xffffffff;
  const unsigned int pull2 = rand() & 0xffffffff;

  cout << "Pull 0: 0x" << HEX32_STRING(pull0) << "\n";
  cout << "Pull 1: 0x" << HEX32_STRING(pull1) << "\n";
  cout << "Pull 2: 0x" << HEX32_STRING(pull2) << "\n";

  preReset(top);

  t->reset(40);

  postReset(top);

  t->tick(1000);

  const int gap = 1000;
  const int per_cmd = 800;

  t->tick(90*500);

  // seed each fpga which applies to the gen packets
  t->send_ring(RING_ADDR_CS31, gen_set_seed(pull0));
  t->tick(per_cmd);

  t->send_ring(RING_ADDR_CS01, gen_set_seed(pull1));
  t->tick(per_cmd);

  t->send_ring(RING_ADDR_CS20, gen_set_seed(pull2));
  t->tick(per_cmd);



  t->send_ring(RING_ADDR_CS31, gen_type_2(RING_ENUM_CS32));
  t->tick(per_cmd);

  t->send_ring(RING_ADDR_CS01, gen_type_2(RING_ENUM_CS22));
  t->tick(per_cmd);

  t->send_ring(RING_ADDR_CS20, gen_type_2(RING_ENUM_CS11));
  t->tick(per_cmd);

  t->tick(gap*30);

  ring_unit_counters_t res20;
  ring_unit_counters_t res01;
  ring_unit_counters_t res11;
  ring_unit_counters_t res21;
  ring_unit_counters_t res31;
  ring_unit_counters_t res02;
  ring_unit_counters_t res12;
  ring_unit_counters_t res22;
  ring_unit_counters_t res32;

  res20 = dump_print(t, RING_ENUM_CS20);
  res01 = dump_print(t, RING_ENUM_CS01);
  res11 = dump_print(t, RING_ENUM_CS11);
  res21 = dump_print(t, RING_ENUM_CS21);
  res31 = dump_print(t, RING_ENUM_CS31);
  res02 = dump_print(t, RING_ENUM_CS02);
  res12 = dump_print(t, RING_ENUM_CS12);
  res22 = dump_print(t, RING_ENUM_CS22);
  res32 = dump_print(t, RING_ENUM_CS32);

  assert(res20.counters[0] == 0);
  assert(res01.counters[0] == 0);
  assert(res11.counters[0] == 15);
  assert(res21.counters[0] == 0);
  assert(res31.counters[0] == 0);
  assert(res02.counters[0] == 0);
  assert(res12.counters[0] == 0);
  assert(res22.counters[0] == 15);
  assert(res32.counters[0] == 15);

  std::cout << "All Tests Passed\n";

  // Final model cleanup
  top->final();

  // Close trace if opened
  if (tfp) {tfp->close();}

  // Destroy model
  delete top; top = NULL;

  exit(0);
}
