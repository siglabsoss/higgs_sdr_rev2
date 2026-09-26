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
#include "higgs_helper.hpp"


using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

#include "piston_c_types.h"
#include "vmem_types.h"
#include "schedule.h"
#include "feedback_bus.h"
#include "feedback_bus_types.h"




VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
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

  // tb inputs starts here
  // user can tick the clock for a period
  // append data to input streams, and look at output streams
  // modify negClock() and posClock() above
  // you can also insert for check streams from those functins()

  // delay between sending inputs
  unsigned int adv = 20;

  int us = 600;

  uint32_t amount = 5;
  uint32_t direction = 1;

  std::vector<uint32_t> vin = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

  for(unsigned int i = 0; i < us; i++) {

    // after 70, output has stopped glitching
    // if(i == 70) {
    //     amount = amount & 0xffffff;
    //     direction = direction & 0xffffff;

    //     t->inStreamAppend("ringbusin",
    //         ringbus_udp_packet(
    //             RING_ADDR_CS10,
    //             SFO_PERIODIC_ADJ_CMD | amount
    //         )
    //     );
    // }

    // if( i == 80 ) {
    //       t->inStreamAppend("ringbusin",
    //         ringbus_udp_packet(
    //             RING_ADDR_CS10,
    //             SFO_PERIODIC_SIGN_CMD | direction
    //         )
    //     );
    // }

    // if( i == 81 ) {
    //         std::vector<uint32_t> sch;
    //         sch.resize(SCHEDULE_SLOTS);
    //         for(auto& n : sch) {
    //             // n = feedback_peer_to_mask(tx_peer_0);
    //             n = 0;
    //         }

    //         auto packet = feedback_vector_packet(
    //             FEEDBACK_VEC_SCHEDULE,
    //             sch,
    //             0x1<<1,
    //             FEEDBACK_DST_HIGGS);

    //         t->inStreamAppend("cs20in",packet);
    // }
   

    t->tick(500);
  }




  // cs30_node_t* cs30_node = top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  // cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  // cs10_node_t* cs10_node = top->tb_higgs_top->cs10_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;

  // file_dump_T<cs30_node_t>(cs30_node, "cs30.out");
  // file_dump_T<cs20_node_t>(cs20_node, "cs20.out");
  // file_dump_T<cs10_node_t>(cs10_node, "cs10.out");

  // hexdump_T<cs20_node_t>(cs20_node,"CS20",0, 32);
  // hexdump_T<cs10_node_t>(cs10_node,"CS10",0, 1024*4);

  cout << "Ringbus got out" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }

  // cout << "CS10 sent to dac:" << endl;
  // for(auto it = t->outs[1].data.begin(); it != t->outs[1].data.end(); it++) {
  //   cout << "0x" << HEX_STRING(*it) << endl;
  // }

  t->allStreamDump();
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
