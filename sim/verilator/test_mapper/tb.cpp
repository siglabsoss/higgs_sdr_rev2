#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
// Include common routines
#include <iomanip>

#include <fstream>

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

// this defines  HEX_STRING, HEX32_STRING
#include "cpp_utils.hpp"


// // If "verilator --trace" is used, include the tracing class
# include <verilated_vcd_c.h>
// #include "dbus.hpp"

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))

#define RESET MIB_MASTER_RESET
#define NSLICES (16)

#include "higgs_helper.hpp"

#include "Vtb_higgs_top_tb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"


using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

#include "vmem_types.h"
#include "piston_c_types.h"


VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
  return main_time; // Note does conversion to real, to match SystemC
}

// void hexdump(node_t * node, string msg, unsigned int start, unsigned int rows); //def
// uint32_t vmem(node_t * node, uint32_t r_addr);

// See higgs_helper.hpp


int main(int argc, char** argv, char** env) {

  std::ofstream outFile;
  std::ifstream inFile;
  outFile.open("vmem.out");
  inFile.open("vmem.in");

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

  // boot the processors

  // std::vector<uint32_t> din = get_counter(0,1024);
  // t->inStreamAppend(0, din);

  // t->tick(60);

  int runtime = 10000;

  // t->tick(5000+18000); //simulation length

  for(int i = 0; i < runtime; i++) {
    t->tick(1);
  }


  // for(auto i = 0; i < 4; i++) {
  //   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS30, bump_c0() ) );
  //   t->tick(140); // small gap so that back to back input dma interupts dont get lost
  //   // t->tick(gap);
  // }
  // for(auto i = 0; i < 4; i++) {
  //   t->tick(gap);
  // }

//   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS00, gen_type_2(RING_ENUM_CS01) ) );
//   t->tick(140);
//
//
//   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS10, gen_type_2(RING_ENUM_CS11) ) );
//   t->tick(140);
//
//   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS31, gen_type_2(RING_ENUM_CS20) ) );
//   t->tick(140);


  // for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
  //   cout << "0x" << HEX_STRING(*it) << endl;
  // }
  cs30_node_t* cs30_node = top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;

  // hexdump_T<cs30_node_t>(cs30_node,"BPSK TABLE",3000*NSLICES, 4);
  // hexdump_T<cs30_node_t>(cs30_node,"QPSK TABLE",2048*NSLICES, 4);
  // hexdump_T<cs30_node_t>(cs30_node,"SOURCE DATA",0*NSLICES, 8);
  // hexdump_T<cs30_node_t>(cs30_node,"DEST DATA",1024*NSLICES, 8);

  cout << "We got this from ringbus udp" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }
  t->outs["ringbusout"]->data.resize(0); // erase what we printed


  // cout << "We got this from cs30 udp" << endl;
  //   for(auto it = t->outs[1].data.begin(); it != t->outs[1].data.end(); it++) {
  //     cout << "0x" << HEX_STRING(*it) << endl;
  //   }
//    t->outs["ringbusout"]->data.resize(0); // erase what we printed



  // t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS31, ring_t_enc(4,0,0,0) ) );
  // t->tick(gap*9);

  // cout << "Dump " << "CS31" << endl;
  // for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
  //   cout << "0x" << HEX_STRING(*it) << endl;
  // }
  // t->outs["ringbusout"]->data.resize(0); // erase what we printed


  // unsigned int rtn = 0x0;

  // if (outFile.is_open()){
  //   for(unsigned int i = 0; i < 4096; i++){
  //     for(unsigned int j = 0; j < NSLICES; j++){
  //       outFile << HEX32_STRING(vmem(cs30_node, i*NSLICES | j)) << " ";
  //     }
  //     outFile << endl;
  //   }
  //   outFile.close();
  // }

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


