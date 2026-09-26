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




void preReset() {
  // Initialize inputs
  top->RESET = 1;
  top->clk = 0;
}

void postReset() {
  // top->i_ringbus = 1;
  // top->i_data_eth = 0;
  top->i_data_adc = 0;
  // top->i_o_ready_eth = 1;
  top->i_o_ready_dac = 1;
  // top->i_data_valid_eth = 1;
  top->i_data_valid_adc = 1;
  cout << "after main time: " << main_time << endl;
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
  t->tick(1000);

  // std::vector<uint32_t> din = get_counter(0,1024);
  // t->inStreamAppend("ringbusin", din);

  // t->tick(60);

  int gap = 1000;


  t->tick(1000);


  // for(auto i = 0; i < 4; i++) {
  //   t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, bump_c0() ) );
  //   t->tick(140); // small gap so that back to back input dma interupts dont get lost
  //   // t->tick(gap);
  // }
  // for(auto i = 0; i < 4; i++) {
  //   t->tick(gap);
  // }

  uint32_t msg;
  // msg = RING_LED_CMD;

  t->inStreamAppend("ringbusin", ringbus_udp_packet(1, 0xF0000001 ) );

  t->tick(300);

  t->inStreamAppend("ringbusin", ringbus_udp_packet(2, 0xF0000002 ) );

  t->tick(300);
  t->tick(300);


  t->inStreamAppend("ringbusin", ringbus_udp_packet(3, 0xF0000003 ) );

  t->tick(300);

  t->inStreamAppend("ringbusin", ringbus_udp_packet(4, 0xF0000004 ) );

  t->tick(300);
  t->tick(300);
  t->tick(300);
  t->tick(300);






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
