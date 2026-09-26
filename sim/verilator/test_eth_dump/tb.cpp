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
# include <verilated_vcd_c.h>
// #include "dbus.hpp"



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


void negClock(helper_t *t) {
  // you must call this, or else data will not stream in/out
  t->handleDataNeg();
  // cout << "time: " << main_time << endl;
}

void posClock(helper_t *t) {
  // you must call this, or else data will not stream in/out
  t->handleDataPos();

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

  unsigned int us = 300;

    for(unsigned int i = 0; i < us; i++) {


          if( true ) {
            // us needs to be about 300 for all these to flush out
            static const std::vector<uint32_t> reportRb = {
                ETH_GET_STATUS_CMD
            };
            auto injectReport = meteredRingbusSendUni<top_t>(t, reportRb, 90, RING_ADDR_ETH);
            injectReport(i);
        }
        t->tick(500);
    }

  cout << "Ringbus got out" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }


  // assert(t->outs["ringbusout"]->data.size() != 0x0 && "Testbench didn't get any ringbus");

  // assert(t->outs["ringbusout"]->data[0] == 0xdead && "Test did not begin");

  // assert(t->outs["ringbusout"]->data.size() >= 5 && "Test did not finish, or output too few items");

  // unsigned int div_time = t->outs["ringbusout"]->data[1];
  // unsigned int mul_time = t->outs["ringbusout"]->data[3];

  // cout << endl;

  // 202, 84 for soft
  // 45,  12 for hardware
  // cout << "Divide took: " << div_time << " clock cycles" << endl;
  // cout << "Multiply took: " << mul_time << " clock cycles" << endl;
  // cout << endl;

  // assert( div_time < 60 );
  // assert( mul_time < 40 );

  // t->outs["ringbusout"]->data.resize(0); // erase what we printed


  // cout << "All Tests Passed (Hardware Multiplier is present)" << endl;


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
