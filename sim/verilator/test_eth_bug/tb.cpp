#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
// Include common routines

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"





// // If "verilator --trace" is used, include the tracing class
# include <verilated_vcd_c.h>
// #include "dbus.hpp"



#define RESET MIB_MASTER_RESET

#include "higgs_helper.hpp"
#include "unit_test_ring.h"


using namespace std;


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




// See higgs_helper.hpp

ring_unit_counters_t dump_print(helper_t *t, uint32_t enumm) {
  int gap = 1000;
  uint32_t report_enum;
  uint32_t report_error;
  ring_unit_counters_t report_counters;
  uint32_t addr = ring_unit_lookup_addr(enumm);

  // feed command in and wait
  t->inStreamAppend("ringbusin", ringbus_udp_packet(addr, ring_t_enc(4,0,0,0) ) );
  t->tick(gap*10);

  cout << "Dump " << get_enum_string(enumm) << " at " << main_time << endl;
  // for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
  //   cout << "    0x" << HEX_STRING(*it) << endl;
  // }

  // convert response to an object we can look at
  report_error = vector_to_ring_unit_counters(t->outs["ringbusout"]->data, &report_enum, &report_counters);

  assert(report_error == 0);

  print_ring_unit_counters_t(report_enum, report_counters);

  cout << endl << endl;

  // assert(report_enum == enumm);

  t->outs["ringbusout"]->data.resize(0); // erase what we printed

  return report_counters;
}



int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 
  

  unsigned int seed_start = std::time(0);

  unsigned int fixed_seed = 0; // set to non zero to use

  // fixed_seed = 1529907185; // entire tb developed on this seed
  if(fixed_seed != 0) {
    seed_start = fixed_seed;
    cout << "starting with hard-coded seed " << seed_start << endl;
  } else {
    cout << "starting with random seed " << seed_start << endl;
  }


  srand(seed_start);


  unsigned int pull0,pull1,pull2;

  // pull0 = rand() & 0xffffffff;
  // pull1 = rand() & 0xffffffff;
  // pull2 = rand() & 0xffffffff;

  // cout << "Pull 0: 0x" << HEX32_STRING(pull0) << endl;
  // cout << "Pull 1: 0x" << HEX32_STRING(pull1) << endl;
  // cout << "Pull 2: 0x" << HEX32_STRING(pull2) << endl;


  preReset(top);

  t->reset(40);

  postReset(top);

  // tb inputs starts here
  // user can tick the clock for a period
  // append data to input streams, and look at output streams
  // modify negClock() and posClock() above
  // you can also insert for check streams from those functins()

  // boot the processors
  t->tick(1000);

  // std::vector<uint32_t> din = get_counter(0,1024);
  // t->inStreamAppend("cs20in", din);

  // t->tick(60);

  int gap = 1000;

  int per_cmd = 140; //800; // 140


  t->tick(1000);

  // uint32_t p0,p1,p2,p3,p4,p5;

  // p0 = gen_set_seed(pull0);
  // p1 = gen_set_seed(pull1);
  // p2 = gen_set_seed(pull2);
  // p3 = gen_type_2(RING_ENUM_CS01);
  // p4 = gen_type_2(RING_ENUM_CS11);
  // p5 = gen_type_2(RING_ENUM_CS20);

  // p0 = 0xf0000000;
  // p1 = 0xf0000001;
  // p2 = 0xf0000002;
  // p3 = 0xf0000003;
  // p4 = 0xf0000004;
  // p5 = 0xf0000005;

  uint32_t p_start = 0xf0000000;
  
  // internal buffer is 32, should be able to handle this no matter what
  // due to lag
  uint32_t p_count = rand() % 33;
  uint32_t gap_max = 350;

  // Send first
  t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, p_count ) );
  t->tick(per_cmd);

  cout << "sent: " << endl;
  for(uint32_t i = 0; i < p_count; i++) {

      // seed each fpga which applies to the gen packets
      t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, p_start + i ) );

      pull0 = rand() % gap_max;

      cout << "ticking for " << pull0 << endl;

      t->tick(pull0);
      cout << HEX32_STRING(p_start+i) << endl;

    }

    cout << "main time is " << main_time << endl;

  // math to calculate guard
  // 1.25us per rb, plus guard, gurad is below
    // second term assumes gap_max was 100, on average it was 50
    // an inaccurate way of finding out how far we've gone
  int min_guard = p_count * 1.5 * 500  - (p_count*(gap_max/2));
  t->tick(min_guard);

  cout << "main time after guard is " << main_time << endl;

  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, p1 ) );
  // t->tick(per_cmd);

  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, p2 ) );
  // t->tick(per_cmd);

  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, p3 ) );
  // t->tick(per_cmd);

  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, p4 ) );
  // t->tick(per_cmd);

  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, p5 ) );
  // t->tick(per_cmd);




  // cout << HEX32_STRING(p1) << endl;
  // cout << HEX32_STRING(p2) << endl;
  // cout << HEX32_STRING(p3) << endl;
  // cout << HEX32_STRING(p4) << endl;
  // cout << HEX32_STRING(p5) << endl;



  t->tick(gap*10);


  cout << endl;
  cout << "Ring got out" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }

  // returns 0xa0000000 + failures
  assert(t->outs["ringbusout"]->data[0] == 0xa0000000);

  // t->outs["ringbusout"]->data.resize(0); // erase what we printed

  // exit(0);

  cout << "All Tests Passed" << endl;


  // Final model cleanup
  top->final();

  // Close trace if opened

  if (tfp) { tfp->close(); }

  // Destroy model
  delete top; top = NULL;
  //print_vector(output_vector);
  // Fin
  exit(0);
}
