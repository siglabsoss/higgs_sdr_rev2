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


  // attach streams
  // here we build a stream object which has poiners into top
  // and then we just pass this into the HiggsHelper.
  // after we do this, we can append data to these streams at any time, either here
  // or in posClock/negClock above

  // 0th input
  // Port32In cs20in;
  // cs20in.t_data = &(top->tx_turnstile_data_in);
  // cs20in.t_valid = &(top->tx_turnstile_data_valid);
  // cs20in.valid_meter = 0;
  // cs20in.valid_state = 0;
  // t->ins.push_back(cs20in);

  // // 1st input
  // Port32In ringbusin;
  // ringbusin.t_data = &(top->ringbus_in_data);
  // ringbusin.t_valid = &(top->ringbus_in_data_vld);
  // ringbusin.valid_meter = 0;
  // t->ins.push_back(ringbusin);

  // // 0th output
  // Port32Out ringbusout;
  // ringbusout.i_data = &(top->ringbus_out_data);
  // ringbusout.i_valid = &(top->ringbus_out_data_vld);
  // ringbusout.i_ready = &(top->ring_bus_i0_ready);
  // ringbusout.control_ready = 1;
  // t->outs.push_back(ringbusout);


  unsigned int seed_start = std::time(0);

  unsigned int fixed_seed = 0; // set to non zero to use

  // fixed_seed = 1527206951; // found the bug the first time (may 24, 2018)

  if(fixed_seed != 0) {
    seed_start = fixed_seed;
    cout << "starting with hard-coded seed " << seed_start << endl;
  } else {
    cout << "starting with random seed " << seed_start << endl;
  }


  srand(seed_start);


  unsigned int pull0,pull1,pull2;

  pull0 = rand() & 0xffffff;
  pull1 = rand() & 0xffffff;
  pull2 = rand() & 0xffffff;

  cout << "Pull 0: 0x" << HEX32_STRING(pull0) << endl;
  cout << "Pull 1: 0x" << HEX32_STRING(pull1) << endl;
  cout << "Pull 2: 0x" << HEX32_STRING(pull2) << endl;


  preReset(top);

  t->reset(40);

  postReset(top);

  // tb inputs starts here
  // user can tick the clock for a period
  // append data to input streams, and look at output streams
  // modify negClock() and posClock() above
  // you can also insert for check streams from those functins()

  uint32_t us = 40;

for(unsigned int i = 0; i < us; i++) {

    // sending the seed starts the test
    // start tx first
    if( i == 2 ) {
      t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS31, ETH_TEST_CMD | pull0 ) );
      // t->tick(per_cmd);
    }

    // start rx after delay (should mess with pushback)
    if( i == 4 ) {
        t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, ETH_TEST_CMD | pull1 ) );
    }

    // first pictured reset was i = 1000

    // first cut is at us666
    // if( i == 1250 ) {
    //   t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, SYNCHRONIZATION_CMD | 1 ) ); // Ask for a on-demand sync
    //   // t->tick(per_cmd);
    // }



    t->tick(500);

  }


  // boot the processors
  // t->tick(1000);

  // // std::vector<uint32_t> din = get_counter(0,1024);
  // // t->inStreamAppend("cs20in", din);

  // // t->tick(60);

  // int gap = 1000;

  // int per_cmd = 140;


  // // ring_unit_t p1;
  // // p1 = ring_t_decode(0xdeadbeef);

  // // // arg 1 is option 2, send ring message with data (arg 3) to enum (arg 2) (which is us)
  // // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, ring_t_enc(2,0,9,0) ) );
  // // t->tick(gap);

  // // // arg 1 is option 1, arg 2 is option 1: bump counter number (arg3) by 1 
  // // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, ring_t_enc(1,1,0,0) ) );
  // // t->tick(gap);

  // // // arg 1 is option 3, arg 2 is option 1,
  // // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, ring_t_enc(3,2,0,0) ) );
  // // t->tick(gap);

  // t->tick(1000);

  // // seed each fpga which applies to the gen packets
  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, gen_set_seed(pull0) ) );
  // t->tick(per_cmd);

  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS10, gen_set_seed(pull1) ) );
  // t->tick(per_cmd);

  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS31, gen_set_seed(pull2) ) );
  // t->tick(per_cmd);




  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, gen_type_2(RING_ENUM_CS01) ) );
  // t->tick(per_cmd);


  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS10, gen_type_2(RING_ENUM_CS11) ) );
  // t->tick(per_cmd);

  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS31, gen_type_2(RING_ENUM_CS20) ) );
  // t->tick(per_cmd);

  // t->tick(gap*30);








  cout << "Before Dumps, we got out" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }
  t->outs["ringbusout"]->data.resize(0); // erase what we printed


  // exit(0);


  // ring_unit_counters_t res20;
  // ring_unit_counters_t res10;
  // ring_unit_counters_t res00;
  // ring_unit_counters_t res01;
  // ring_unit_counters_t res11;
  // ring_unit_counters_t res21;
  // ring_unit_counters_t res31;
  // ring_unit_counters_t res30;

  // // this deletes the output of outs[0]
  // res20 = dump_print(t, RING_ENUM_CS20);
  // res10 = dump_print(t, RING_ENUM_CS10);
  // res00 = dump_print(t, RING_ENUM_CS00);
  // res01 = dump_print(t, RING_ENUM_CS01);
  // res11 = dump_print(t, RING_ENUM_CS11);
  // res21 = dump_print(t, RING_ENUM_CS21);
  // res31 = dump_print(t, RING_ENUM_CS31);
  // res30 = dump_print(t, RING_ENUM_CS30);

  // assert(res20.counters[0] == 15);
  // assert(res10.counters[0] == 0);
  // assert(res00.counters[0] == 0);
  // assert(res01.counters[0] == 15);
  // assert(res11.counters[0] == 15);
  // assert(res21.counters[0] == 0);
  // assert(res31.counters[0] == 0);
  // assert(res30.counters[0] == 0);

  // cout << "All Tests Passed" << endl;


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
