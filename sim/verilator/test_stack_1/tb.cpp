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

#include "piston_c_types.h"
#include "vmem_types.h"

#include "cpp_utils.hpp"



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
// typedef Vtb_higgs_top_vmem_dat_6_5__pi8 eth_node_t;

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


uint32_t mock(uint32_t x) {
    return 0;
}



int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 


  srand(1);

  preReset(top);

  t->reset(40);


  postReset(top);

  int us;
  us = 300;

  for(unsigned int i = 0; i < us; i++) {
    t->tick(500); // tick 1 us

  }


  cout << "Ring got out " << t->outs["ringbusout"]->data.size() << " items." << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }
  cout << "\n";

  // cout << "All Tests Passed" << endl;
  // cs30_node_t* cs30_node = top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  // hexdump_vmem<cs30_node_t>(cs30_node,"CS30",0, 256);

  // auto inspect_cs30 = std::bind(&Foo::doSomething, this);

  // std::function<uint32_t(uint32_t)> inspect = &mock;
  // std::function<uint32_t(uint32_t)> inspect = 
  // std::bind(
  //       &top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->mem->inspect_scalar,
  //       top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->mem,
  //       std::placeholders::_1
  //       );

  // works
  std::function<uint32_t(uint32_t)> inspect = 
  std::bind(
        &Vtb_higgs_top_scalar_memory__pi12::inspect_scalar,
        top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->mem,
        std::placeholders::_1
        );

  int start;
  // start = 0x7000;
  start = 0x6400;
  // start = 0;



  hexprint_scalarmem(
    inspect
    ,"cs30"
    ,start
    // ,0
    );

  // for(int i = 0; i < 8192; i++) {
  //   auto memword = top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->mem->inspect_scalar(i);
  //   cout << HEX32_STRING(memword) << " ";
  //   if( i % 16 == 15) {
  //       cout << "\n";
  //   }
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
