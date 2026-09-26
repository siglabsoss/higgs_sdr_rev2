#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"


// this file comes directly from the riscv/c/inc folderi
// also it uses the VERILATE_TESTBENCH define we set above
#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_PC
#include "ringbus2_post.h"





// // If "verilator --trace" is used, include the tracing class
#include <verilated_vcd_c.h>
// #include "dbus.hpp"

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))


#define RESET MIB_MASTER_RESET

#include "higgs_helper.hpp"

#include "Vtb_higgs_top_tb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "piston_c_types.h"
#include "vmem_types.h"

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


// void hexdump_file(node_t *node, string fname, unsigned int start, unsigned int len);
// void hexdump(node_t * node, string msg, unsigned int start, unsigned int len);



// See higgs_helper.hpp
std::vector<uint32_t> udp_data8_packet(uint32_t w0, uint32_t w1) {
  std::vector<uint32_t> out;
  out.push_back(w0);
  out.push_back(w1);

  return out;
}


std::string translate_test1(unsigned int x) {
  switch(x) {
    case 0:
      return "next->NULL";
      break;
    case 0x1:
      return "";
    case 23556:
      return "next->node 0";
      break;
    case 0x40000:
      return "node 0";
      break;
    case 0x5c0c:
      return "next->node 1";
      break;
    case 0x48000:
      return "node 1";
      break;
    case 0x5c14:
      return "next->node 2";
      break;
    case 0x50000:
      return "node 2";
      break;
    case 0x5c1c:
      return "next->node 3";
      break;
    case 0x58000:
      return "node 3";
      break;
    case 0x5c24:
      return "next->node 4";
      break;
    case 0x60000:
      return "node 4";
      break;
    case 0x5c2c:
      return "next->node 5";
      break;
    case 0x68000:
      return "node 5";
      break;
    case 0x5c34:
      return "next->node 6";
      break;
    case 0x70000:
      return "node 6";
      break;
    case 0x5c3c:
      return "next->node 7";
      break;
    case 0x78000:
      return "node 7";
      break;
    default:
      return std::to_string(x);
      break;
  }
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
  // t->ins.push_back(cs20in);

  // // 1st input
  // Port32In ringbusin;
  // ringbusin.t_data = &(top->ringbus_in_data);
  // ringbusin.t_valid = &(top->ringbus_in_data_vld);
  // t->ins.push_back(ringbusin);

  // // 0th output
  // Port32Out ringbusout;
  // ringbusout.i_data = &(top->ringbus_out_data);
  // ringbusout.i_valid = &(top->ringbus_out_data_vld);
  // ringbusout.i_ready = &(top->ring_bus_i0_ready);
  // ringbusout.control_ready = 1;
  // t->outs.push_back(ringbusout);


  srand(1);

  preReset(top);

  t->reset(40);


  postReset(top);


  // boot the processors
  // for(unsigned int i = 0; i < 1000*140; i++) {
  //   t->tick(1);
      
  //   unsigned int cmd_valid = top->tb_higgs_top->eth_top->q_engine_inst->get_xbaseband_cmd_valid();
  //   unsigned int cmd_instruction = top->tb_higgs_top->eth_top->q_engine_inst->get_xbaseband_cmd_payload_instruction();
  //   if(cmd_valid) {
  //     // cout << main_time << endl;
  //     cout << "0x" << HEX_STRING(cmd_instruction) << endl;
  //   }
  // }

  //       t->inStreamAppend("cs20in", udp_data8_packet(0x8765432a, 0xffffffff) );
  // 0x00050000: 80010000 7fff0000 80010000 7fff0000 80010000 7fff0000 80010000 80010000 7fff0000 7fff0000 80010000 80010000 80010000 80010000 7fff0000 80010000 
  // 0x00050040: 7fff0000 80010000 7fff0000 80010000 80010000 7fff0000 7fff0000 80010000 7fff0000 7fff0000 7fff0000 80010000 80010000 80010000 80010000 7fff0000 
  // 0x00050080: 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 
  // 0x000500c0: 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 7fff0000 


  // for 0x00000021
  //
  // 0x00050000: 7fff0000 80010000 80010000 80010000 80010000 7fff0000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 


  // for 0x00000011, 0x00000000
  //
  // 0x00050000: 7fff0000 80010000 80010000 80010000 7fff0000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 
  // 0x00050040: 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 
  // 0x00050080: 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 
  // 0x000500c0: 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 80010000 



  // these will change if crt_standard.S changes
  unsigned int pc_done[3] = {0xE4, 0xE8, 0xEC};

  unsigned int us = 300;


  // loop for 300 us or until cs30 is done
  for(unsigned int i = 0; i < us; i++) {
    t->tick(500); // tick 1 us


    if(i == 9) {                         //   first       second
      // t->inStreamAppend("cs20in", udp_data8_packet(0xffedcba9, 0x87654321) );
      t->inStreamAppend("cs20in", udp_data8_packet(0x8765432a, 0xffffffff) );
      //t->inStreamAppend("cs20in", udp_data8_packet(0x12345678, 0xdeadbeef) );
    }
    unsigned int cs20_pc = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
    if(
      cs20_pc == pc_done[0] ||
      cs20_pc == pc_done[1] ||
      cs20_pc == pc_done[2]
      ) {
      break;
    }
  }

  // flush ringbus
  t->tick(500*10);

  cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  // hexdump(cs20_node,"ALL",0, 1024*NSLICES / 4);
  cs10_node_t* cs10_node = top->tb_higgs_top->cs10_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;

  // hexdump_vmem<cs20_node_t>(cs20_node,"CS20",0, 4096);
  hexdump_vmem<cs10_node_t>(cs10_node,"CS10",0, 256);

  // hexdump(cs10_node,"ALL",0, 1024*NSLICES / 4);

  // hexdump_file(cs10_node,"cs10_vmem.hex", 0, 1024*NSLICES / 4);

  assert( t->outs["ringbusout"]->data.size() != 0 && "didn't get any output");

  cout << "Ring got out " << t->outs["ringbusout"]->data.size() << " items." << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    // cout << "" << HEX_STRING(translate_test1(*it)) << endl;
    cout << "" << HEX_STRING(*it) << endl;
  }


  assert( t->outs["ringbusout"]->data[t->outs["ringbusout"]->data.size()-1] == 0x01 && "Some tests did not pass");  
  assert( t->outs["ringbusout"]->data[3] == 0xF && "Some tests failed, expected 0xF");  

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


// void hexdump_file(node_t *node, string fname, unsigned int start, unsigned int len) {
//   ofstream myfile;
//   myfile.open (fname);
//   unsigned int vmem_start = start/NSLICES;
//   for (auto i = vmem_start; i < vmem_start + len; i++){
//     myfile << HEX32_STRING( node->mem_slice_0->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_1->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_2->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_3->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_4->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_5->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_6->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_7->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_8->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_9->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_10->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_11->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_12->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_13->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_14->dpram_inst->rtn_mem(i) ) << endl;
//     myfile << HEX32_STRING( node->mem_slice_15->dpram_inst->rtn_mem(i) ) << endl;
//   }

//   myfile.close();
// }


// void hexdump(node_t * node, string msg, unsigned int start, unsigned int len) {
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
