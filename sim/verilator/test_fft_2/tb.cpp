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



// 



// // If "verilator --trace" is used, include the tracing class
#include <verilated_vcd_c.h>
// #include "dbus.hpp"

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))


#define RESET MIB_MASTER_RESET

#include "higgs_helper.hpp"


using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

// typedef Vtb_higgs_top_q_engine__pi3 q_engine_t;



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




int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



  srand(1);

  preReset(top);

  t->reset(40);


  postReset(top);

  int runtime;
  runtime = 1000*600;
  unsigned us = 1200;
  t->tick(us*500);
  // runtime = 1000*140;

  // boot the processors
  // for(unsigned int i = 0; i < runtime; i++) {
  //   t->tick(1);
      
  //   // unsigned int cmd_valid = top->tb_higgs_top->eth_top->q_engine_inst->get_xbaseband_cmd_valid();
  //   // unsigned int cmd_instruction = top->tb_higgs_top->eth_top->q_engine_inst->get_xbaseband_cmd_payload_instruction();
  //   // if(cmd_valid) {
  //   //   cout << "0x" << HEX_STRING(cmd_instruction) << endl;
  //   // }


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
  assert( t->outs["ringbusout"]->data[t->outs["ringbusout"]->data.size()-1] == 0xffffffff && "FFT test did not pass");  

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

