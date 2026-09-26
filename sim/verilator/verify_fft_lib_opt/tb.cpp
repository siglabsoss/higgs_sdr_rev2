#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <iomanip>

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"



// 



// // If "verilator --trace" is used, include the tracing class
#include <verilated_vcd_c.h>
// #include "dbus.hpp"

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))


#define RESET MIB_MASTER_RESET

#include "higgs_helper.hpp"

#include "Vtb_higgs_top_tb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"
#define NSLICES (16)


using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

// typedef Vtb_higgs_top_q_engine__pi3 q_engine_t;

// eth
// typedef Vtb_higgs_top_vmem_dat_5_4__pi8 node_t;

VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
  return main_time; // Note does conversion to real, to match SystemC
}


// void hexdump(node_t * node, string msg, unsigned int start, unsigned int len);


// See higgs_helper.hpp




int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



  srand(1);

  preReset(top);

  t->reset(40);


  postReset(top);

  unsigned int pc_done[3] = {0xE4, 0xE8, 0xEC};

  int us;
  us = 800;

  for(unsigned int i = 0; i < us; i++) {
    t->tick(500); // tick 1 us

    unsigned int eth_pc = 0;//top->tb_higgs_top->eth_top->q_engine_inst->get_iBus_cmd_payload_pc();
    if(
      eth_pc == pc_done[0] ||
      eth_pc == pc_done[1] ||
      eth_pc == pc_done[2]
      ) {
      break;
    }
  }

  // node_t * eth_node = top->tb_higgs_top->eth_top->q_engine_inst->piston_inst->unode15;

  // hexdump(eth_node,"ALL",0, 1024*NSLICES / 4);

  // t->tick(us*500);
  // runtime = 1000*140;

  // boot the processors
  // for(unsigned int i = 0; i < runtime; i++) {
    // t->tick(1);
      
    // unsigned int cmd_valid = top->tb_higgs_top->eth_top->q_engine_inst->get_xbaseband_cmd_valid();
    // unsigned int cmd_instruction = top->tb_higgs_top->eth_top->q_engine_inst->get_xbaseband_cmd_payload_instruction();
    // if(cmd_valid) {
    //   cout << "0x" << HEX_STRING(cmd_instruction) << endl;
    // }


  // }

  // t->tick(1000*140);

  // assert( t->outs["ringbusout"]->data.size() != 0 && "didn't get any output");

  cout << "Ring got out " << t->outs["ringbusout"]->data.size() << " items." << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }
  // t->outs["ringbusout"]->data.resize(0); // erase what we printed

  // assert( t->outs["ringbusout"]->data.size() == 2 && "got wrong number of dma items");  

  assert( t->outs["ringbusout"]->data[0] == 0xdeadbeef && "Test did not start");  
  assert( t->outs["ringbusout"]->data[t->outs["ringbusout"]->data.size()-1] == 0x02 && "One or more tests didn't pass");

  // for(auto i =0; i < 4; i++) {
  //   assert( t->outs["ringbusout"]->data[i] == i+0xf0 && "got wrong value in dma");  
  // }

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




// void hexdump(node_t * node, string msg, unsigned int start, unsigned int len){    
//   unsigned int vmem_start = start/NSLICES;
//   cout << msg << endl;
//   cout << "hexdump... " << endl;
//   cout << "start : 0x" << HEX32_STRING(start) << endl;
//   cout << "length: 0x" << HEX32_STRING(len) << endl;
//   for (auto i = vmem_start; i < vmem_start + len; i++){
//     cout << "0x" << HEX32_STRING(((i*NSLICES)<<2)+0x40000) << ": ";
//     cout << HEX32_STRING( node->mem_slice_0->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_1->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_2->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_3->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_4->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_5->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_6->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_7->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_8->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_9->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_10->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_11->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_12->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_13->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_14->dpram_inst->rtn_mem(i) ) << " ";
//     cout << HEX32_STRING( node->mem_slice_15->dpram_inst->rtn_mem(i) ) << " ";
//     cout << endl;
//   }
//   cout << endl;
// }
