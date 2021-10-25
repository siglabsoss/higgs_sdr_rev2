// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2017 by Wilson Snyder.
//======================================================================

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

// any additionaly files included directly from Riscv inc folder MUST
// come after higgs_helper.hpp
#include "unit_test_ring.h"


using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;



VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top; // = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
  return main_time; // Note does conversion to real, to match SystemC
}

std::vector<uint32_t> global_ring; // captured data



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
  // void (*negClock)(helper_t*);

void subtest(std::string vcd_filename, const char *cmd_options, void (*fn)(helper_t*) )
{
  // build top
  top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
  main_time = 0; // reset this to 0

  if (cmd_options && 0==strcmp(cmd_options, "+trace")) {
    Verilated::traceEverOn(true);  // Verilator must compute traced signals
    cout << "Enabling waves into " << vcd_filename << "...\n" << endl;
    tfp = new VerilatedVcdC;
    top->trace(tfp, 99);  // Trace 99 levels of hierarchy
    // mkdir("logs", 0777);
    tfp->open(vcd_filename.c_str());  // Open the dump file
  } else {
    cout << "WILL NOT WRITE .vcd WAVE FILE" << endl;
  }



  // This helper is what I built to make this function easy
  // this handles reset.  You can register an arbitrary number of inputs
  // and outputs.
  // calling things like `inStreamAppend()` allows user to easily specify queue
  // input data which will be ticked over when tick is called
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
  // ringbusout.i_data = &(top->cs20_out_data);
  // ringbusout.i_valid = &(top->cs20_o_data_vld);
  // ringbusout.i_ready = &(top->cs20_i_ready);
  // ringbusout.control_ready = 1;
  // t->outs.push_back(ringbusout);


  srand(1);

  // top->cs20_i_ringbus = 1;
  // top->eth_rst = 1;
  // cout << "start here" << endl;
  preReset(top);

  t->reset(40);

  postReset(top);

  // t->tick(0);

  // top->eth_rst = 0;

  fn(t);



  cout << "final" << endl;
  // Final model cleanup
  top->final();

  // Close trace if opened

  cout << "pre close" << endl;
  if (tfp) {
    tfp->close(); 
    tfp = NULL;
  }

  // Destroy model
  delete top;
  top = NULL;
  //print_vector(output_vector);
}

#include <ctime>
#include <cstdlib>

void test_single_edge_forever(helper_t *t) {
  cout << "in test_single_edge_forever at" << main_time << endl;
  // boot the processors
  t->tick(10);

  std::vector<uint32_t> m1 = {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,};
  std::vector<uint32_t> m2 = {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,};
  
  uint32_t expected = 0x100001B;

  std::srand(std::time(nullptr)); // use current time as seed for random generator
  int random_seed = std::rand();
  cout << "Picked Seed: " << HEX_STRING(random_seed) << endl;

  int picked_seed = random_seed;

  picked_seed = 0x7db14216;

  std::srand(picked_seed);

  cout << endl << endl;

  for(int j = 0; j < 3; j++) {

    int locs = std::rand() % 10;
    cout << "will erase " << locs << " positions" << endl;
    for(int k = 0; k < locs; k++) {
      int loc1 = std::rand() % m2.size();
      cout << "erase " << loc1 << endl;
      m2.erase(m2.begin()+loc1);
    }

    cout << endl;

    for(int l = 0; l < m2.size(); l++) {
      cout << m2[l] << ",";
    }
    
    cout << endl;


    VEC_R_APPEND(global_ring, m2);

    t->tick(1000);

    uint32_t result = -1;
    cout << "Dump " << "CS20" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
      cout << "0x" << HEX_STRING(*it) << endl;
      result = *it;
    }

    t->outs["ringbusout"]->data.resize(0);

    if(result != expected) {
      cout << "FAIL" << endl;
    } else {
      cout << "got it" << endl;
    }

  }

}



int main(int argc, char** argv, const char* env) {

  cout << "tb.cpp main()" << endl;

  // This is a more complicated example, please also see the simpler examples/hello_world_c.

  // Prevent unused variable warnings
  if (0 && argc && argv && env) {}
  // Pass arguments so Verilated code can see them, e.g. $value$plusargs
  Verilated::commandArgs(argc, argv);

  // Set debug level, 0 is off, 9 is highest presently used
  Verilated::debug(0);

  // Randomization reset policy
  Verilated::randReset(2);

  const char* flag = Verilated::commandArgsPlusMatch("trace");
  
  subtest("wave_dump.vcd", flag, &test_single_edge_forever);
  // subtest("wave_dump2.vcd", flag, test_single_edge_forever);

  // t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS31, ring_t_enc(4,0,0,0) ) );
  // t->tick(gap*9);

  // cout << "Dump " << "CS31" << endl;
  // for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
  //   cout << "0x" << HEX_STRING(*it) << endl;
  // }
  // t->outs["ringbusout"]->data.resize(0); // erase what we printed



  // Fin
  exit(0);
}
