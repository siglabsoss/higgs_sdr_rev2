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
// typedef Vtb_higgs_top_vmem_dat_6_5__pi58 cs10_node_t;
// typedef Vtb_higgs_top_vmem_dat_6_5__pi21 cs20_node_t;
// typedef Vtb_higgs_top_vmem_dat_6_5__pi21 cs30_node_t;
// typedef Vtb_higgs_top_vmem_dat_6_5__pi20 eth_node_t;

// Vtb_higgs_top_vmem_dat_6_5__pi22 TOP__tb_higgs_top__cs10_top__vex_machine_top_inst__q_engine_inst__piston_inst__unode28
// Vtb_higgs_top_vmem_dat_6_5__pi21 TOP__tb_higgs_top__cs20_top__vex_machine_top_inst__q_engine_inst__piston_inst__unode28
// Vtb_higgs_top_vmem_dat_6_5__pi20 TOP__tb_higgs_top__eth_top__q_engine_inst__piston_inst__unode28

// Vtb_higgs_top_vmem_dat_6_5__pi59 TOP__tb_higgs_top__cs00_top__vex_machine_top_inst__q_engine_inst__piston_inst__unode28
// Vtb_higgs_top_vmem_dat_6_5__pi60 TOP__tb_higgs_top__cs01_top__vex_machine_top_inst__q_engine_inst__piston_inst__unode28
// Vtb_higgs_top_vmem_dat_6_5__pi58 TOP__tb_higgs_top__cs10_top__vex_machine_top_inst__q_engine_inst__piston_inst__unode28
// Vtb_higgs_top_vmem_dat_6_5__pi61 TOP__tb_higgs_top__cs11_top__vex_machine_top_inst__q_engine_inst__piston_inst__unode28
// Vtb_higgs_top_vmem_dat_6_5__pi57 TOP__tb_higgs_top__cs20_top__vex_machine_top_inst__q_engine_inst__piston_inst__unode28
// Vtb_higgs_top_vmem_dat_6_5__pi62 TOP__tb_higgs_top__cs21_top__vex_machine_top_inst__q_engine_inst__piston_inst__unode28
// Vtb_higgs_top_vmem_dat_6_5__pi64 TOP__tb_higgs_top__cs30_top__vex_machine_top_inst__q_engine_inst__piston_inst__unode28
// Vtb_higgs_top_vmem_dat_6_5__pi63 TOP__tb_higgs_top__cs31_top__vex_machine_top_inst__q_engine_inst__piston_inst__unode28






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

  // boot the processors
  t->tick(5*500);

  

  // set to 0 to be turnstile data
  //t->inStreamAppend(0, data_udp_packet() );
  t->inStreamAppend("cs20in", file_udp_packet("input.dat") );

  // provide input data for the adc
  t->inStreamAppend("cs00in", file_udp_packet("adc.dat") );

  // std::vector<uint32_t> din = get_counter(0,1024);
  // t->inStreamAppend(0, din);

  // t->tick(60);

  int us = 200;

  t->tick(us*500);




  // t->outs["ringbusout"]->data.resize(0); // erase what we printed

  // assert("Test did not start");
  // assert( t->outs["ringbusout"]->data[0] == 0xdeadbeef && "Test did not start");  
  // // assert( t->outs["ringbusout"]->data[t->outs["ringbusout"]->data.size()-1] == 0x1F) && "One or more tests didn't pass");

  // cout << "Output of begin and end\t:\t";

  // node_t * cs20_top = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;


  // cs30_node_t* cs30_node = top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  cs00_node_t* cs00_node = top->tb_higgs_top->cs00_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  cs01_node_t* cs01_node = top->tb_higgs_top->cs01_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;

  file_dump_T<cs00_node_t>(cs00_node, "cs00.vmem");
  file_dump_T<cs01_node_t>(cs01_node, "cs01.vmem");
  // file_dump_T<cs10_node_t>(cs10_node, "cs10.out");

  // hexdump_T<cs20_node_t>(cs20_node,"CS20",0, 32);
  // hexdump_T<cs10_node_t>(cs10_node,"CS10",0, 1024*4);

  // cout << "Ringbus got out" << endl;
  // for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
  //   cout << "0x" << HEX_STRING(*it) << endl;
  // }

  // cout << "CS10 sent to dac:" << endl;
  // for(auto it = t->outs[1].data.begin(); it != t->outs[1].data.end(); it++) {
  //   cout << "0x" << HEX_STRING(*it) << endl;
  // }

  // file_dump_vec(t->outs[1].data, "cs00_out.hex");
  // file_dump_vec(t->outs[2].data, "cs01_out.hex");

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
