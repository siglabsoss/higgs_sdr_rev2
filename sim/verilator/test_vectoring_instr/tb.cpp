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
  // top->i_o_ready_dac = 1;
  // top->i_data_valid_eth = 1;
  top->i_data_valid_adc = 1;
  cout << "after main time: " << main_time << endl;
}


void negClock(helper_t *t) {
  // you must call this, or else data will not stream in/out
  t->handleDataNeg();
  // cout << "time: " << main_time << endl;
}

void posClock(helper_t *t) {
  // you must call this, or else data will not stream in/out
  t->handleDataPos();

}

std::vector<uint32_t> get_counter(int start, int stop) {
  std::vector<uint32_t> out;
  for(int i = start; i < stop; i++) {
    out.push_back(i);
  }

  if(0) {
    cout << "get_counter( " << start << ", " << stop << ")" << endl;
    for(auto it = out.begin(); it < out.end(); it++) {
      cout << *it << endl;
    }
  }
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
  if (flag && 0==strcmp(flag, "+trace")) {
    Verilated::traceEverOn(true);  // Verilator must compute traced signals
    cout << "Enabling waves into wave_dump.vcd...\n" << endl;
    tfp = new VerilatedVcdC;
    top->trace(tfp, 99);  // Trace 99 levels of hierarchy
    // mkdir("logs", 0777);
    tfp->open("wave_dump.vcd");  // Open the dump file
  } else {
    cout << "WILL NOT WRITE .vcd WAVE FILE" << endl;
    cout << "  \"make show\" will be stale " << endl << endl;
  }

  // This helper is what I built to make this function easy
  // this handles reset.  You can register an arbitrary number of inputs
  // and outputs.
  // calling things like `inStreamAppend()` allows user to easily specify queue
  // input data which will be ticked over when tick is called
  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);

  // attach handlers
  t->negClock = &negClock;
  t->posClock = &posClock;


  // attach streams
  // here we build a stream object which has poiners into top
  // and then we just pass this into the HiggsHelper.
  // after we do this, we can append data to these streams at any time, either here
  // or in posClock/negClock above

  // 0th input
  Port32In cs20in;
  cs20in.t_data = &(top->tx_turnstile_data_in);
  cs20in.t_valid = &(top->tx_turnstile_data_valid);
  t->ins.push_back(cs20in);

  // 1st input
  Port32In ringbusin;
  ringbusin.t_data = &(top->ringbus_in_data);
  ringbusin.t_valid = &(top->ringbus_in_data_vld);
  t->ins.push_back(ringbusin);

  // 0th output
  Port32Out ringbusout;
  ringbusout.i_data = &(top->ringbus_out_data);
  ringbusout.i_valid = &(top->ringbus_out_data_vld);
  ringbusout.i_ready = &(top->ring_bus_i0_ready);
  ringbusout.control_ready = 1;
  t->outs.push_back(ringbusout);


  srand(1);

  preReset();

  t->reset(40);


  postReset();


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

  // these will change if crt_standard.S changes
  unsigned int pc_done[3] = {0xE4, 0xE8, 0xEC};

  unsigned int us = 1000;
  // t->tick(500*300);

  // loop for 300 us or until cs30 is done
  for(unsigned int i = 0; i < us; i++) {
    t->tick(500); // tick 1 us
    unsigned int cs30_pc = top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
    if(
      cs30_pc == pc_done[0] ||
      cs30_pc == pc_done[1] ||
      cs30_pc == pc_done[2]
      ) {
        cout << "Breaking at " << main_time << "(" << i << " us) PC was " << cs30_pc << endl;
      break;
    }
  }

  // flush ringbus
  t->tick(500*6); // set to 500*100 if you are dumping many ringbus with ring_block_send_eth()

  // assert( t->outs[0].data.size() != 0 && "didn't get any output");

  cout << "Ring got out " << t->outs[0].data.size() << " items." << endl;
  for(auto it = t->outs[0].data.begin(); it != t->outs[0].data.end(); it++) {
    // cout << "" << HEX_STRING(translate_test1(*it)) << endl;
    cout << "" << HEX_STRING(*it) << endl;
  }
  // t->outs[0].data.resize(0); // erase what we printed

  // assert( t->outs[0].data.size() == 4 && "got wrong number of dma items");  

  assert( t->outs[0].data[t->outs[0].data.size()-1] == 0xff && "Some tests did not pass");  
  // assert( t->outs[0].data[3] == 0xF && "Some tests failed, expected 0xF");  

  // for(auto i =0; i < 4; i++) {
  //   assert( t->outs[0].data[i] == i+0xf0 && "got wrong value in dma");  
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

