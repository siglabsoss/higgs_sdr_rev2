#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
// Include common routines

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

#include <fstream>

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"





#include <verilated_vcd_c.h>


#define RESET MIB_MASTER_RESET

#define GARBAGE_ADDR       (4094*NSLICES)
#define SCRATCH_ADDR       (4095*NSLICES)

#include "higgs_helper.hpp"



using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

#include "piston_c_types.h"
#include "vmem_types.h"




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

std::vector<uint32_t> data_udp_packet() {
  std::vector<uint32_t> out;
  out.push_back(0xdeadbeef);
  out.push_back(0x12345678);
  out.push_back(0x0000ffff);
  return out;
}



int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



  srand(1);

  preReset(top);

  t->reset(40);

  postReset(top);

  // tb inputs starts here
  // user can tick the clock for a period
  // append data to input streams, and look at output streams
  // modify negClock() and posClock() above
  // you can also insert for check streams from those functins()

  // delay between sending inputs
  unsigned int adv = 1;

  int us = 350;

  std::vector<uint32_t> vin = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

  for(unsigned int i = 0; i < us; i++) {
    if(i == 14 + adv*0) {
      t->inStreamAppend("cs20in", counter_stream(16) );
      t->inStreamAppend("cs20in", counter_stream(16) );
    }

    if(i == 14 + adv*1) {
      t->inStreamAppend("cs20in", counter_stream(16) );
      t->inStreamAppend("cs20in", counter_stream(16) );
    }

    if(i == 14 + adv*2) {
      t->inStreamAppend("cs20in", counter_stream(16) );
      t->inStreamAppend("cs20in", counter_stream(16) );
    }

    if(i == 14 + adv*3) {
      t->inStreamAppend("cs20in", counter_stream(16) );
      t->inStreamAppend("cs20in", counter_stream(16) );
    }

    if(i == 14 + adv*4) {
      t->inStreamAppend("cs20in", counter_stream(16) );
      t->inStreamAppend("cs20in", counter_stream(16) );
    }

    if(i == 14 + adv*5) {
      t->inStreamAppend("cs20in", counter_stream(16) );
      t->inStreamAppend("cs20in", counter_stream(16) );
    }

    if(i == 14 + adv*6) {
      t->inStreamAppend("cs20in", counter_stream(16) );
      t->inStreamAppend("cs20in", counter_stream(16) );
    }

    t->tick(500);
  }

  

  // int gap = 1000;


  // ring_unit_t p1;
  // p1 = ring_t_decode(0xdeadbeef);

  // // arg 1 is option 2, send ring message with data (arg 3) to enum (arg 2) (which is us)
  // t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS30, ring_t_enc(2,0,9,0) ) );
  // t->tick(gap);

  // // arg 1 is option 1, arg 2 is option 1: bump counter number (arg3) by 1 
  // t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS30, ring_t_enc(1,1,0,0) ) );
  // t->tick(gap);

  // // arg 1 is option 3, arg 2 is option 1,
  // t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS30, ring_t_enc(3,2,0,0) ) );
  // t->tick(gap);

  // t->tick(1000);


  // t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS00, gen_type_2(RING_ENUM_CS01) ) );
  // t->tick(140);


  // t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS10, gen_type_2(RING_ENUM_CS11) ) );
  // t->tick(140);

  // t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS31, gen_type_2(RING_ENUM_CS20) ) );
  // t->tick(140);

  // t->tick(gap*18);




  // t->outs["ringbusout"]->data.resize(0); // erase what we printed

  // assert("Test did not start");
  // assert( t->outs["ringbusout"]->data[0] == 0xdeadbeef && "Test did not start");  
  // // assert( t->outs["ringbusout"]->data[t->outs["ringbusout"]->data.size()-1] == 0x1F) && "One or more tests didn't pass");

  // cout << "Output of begin and end\t:\t";

  // node_t * cs20_top = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;


  // cs30_node_t* cs30_node = top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  cs10_node_t* cs10_node = top->tb_higgs_top->cs10_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;

  // file_dump_T<cs30_node_t>(cs30_node, "cs30.out");
  file_dump_T<cs20_node_t>(cs20_node, "cs20.out");
  file_dump_T<cs10_node_t>(cs10_node, "cs10.out");

  // hexdump_T<cs20_node_t>(cs20_node,"CS20",0, 32);
  // hexdump_T<cs10_node_t>(cs10_node,"CS10",0, 1024*4);

  cout << "Ringbus got out" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }

  for(unsigned int i = 0; i < t->outs["ringbusout"]->data.size(); i++ ) {
    t->outs["ringbusout"]->data[i];
  }

  // cout << "CS10 sent to dac:" << endl;
  // for(auto it = t->outs[1].data.begin(); it != t->outs[1].data.end(); it++) {
  //   cout << "0x" << HEX_STRING(*it) << endl;
  // }

  t->outStreamDump("cs10out");
  
  t->outStreamDump("cs20out");

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
