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
#include <verilated_vcd_c.h>
// #include "dbus.hpp"

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))


#define RESET MIB_MASTER_RESET

#include "higgs_helper.hpp"


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
  unsigned int us = 130;

  for(unsigned int i = 0; i < us; i++) {
    t->tick(500); // tick 1 us
    // if( t->pc_exicted_main("cs30") ) {
    //     cout << "exiting main early at us " << i << endl;
    //   break;
    // }
  }

  // assert( t->outs["ringbusout"]->data.size() != 0 && "didn't get any output");

  cout << "Ring got out " << t->outs["ringbusout"]->data.size() << " items." << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }
  // t->outs["ringbusout"]->data.resize(0); // erase what we printed

  assert( t->outs["ringbusout"]->data.size() == 2 && "got wrong number of dma items");  

  assert( t->outs["ringbusout"]->data[0] == 0xdeadbeef && "Test did not start");  
  assert( t->outs["ringbusout"]->data[1] == 0x7f7 && "One or more tests did not pass");  

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

