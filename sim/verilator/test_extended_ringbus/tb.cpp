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

#include "cpp_utils.hpp"


//////////////////////////
//
//   Vex includes
// 
// this file comes directly from the riscv/c/inc folder
// also it uses the VERILATE_TESTBENCH define we set above



#include "extended_ringbus.h"
///////////////////////////


// // If "verilator --trace" is used, include the tracing class
#include <verilated_vcd_c.h>
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
int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



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
  // t->inStreamAppend("cs20in", din);

  // t->tick(60);

  // int gap = 1000;

  int per_cmd = 140;


  // ring_unit_t p1;
  // p1 = ring_t_decode(0xdeadbeef);

  // // arg 1 is option 2, send ring message with data (arg 3) to enum (arg 2) (which is us)
  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, ring_t_enc(2,0,9,0) ) );
  // t->tick(gap);

  // // arg 1 is option 1, arg 2 is option 1: bump counter number (arg3) by 1 
  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, ring_t_enc(1,1,0,0) ) );
  // t->tick(gap);

  // // arg 1 is option 3, arg 2 is option 1,
  // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, ring_t_enc(3,2,0,0) ) );
  // t->tick(gap);

  t->tick(1000);

  uint32_t a0,a1,a2;

  

  // do a normal DMA out (forced to be address 0)
  t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, DMA_OUT_CMD | 400 ) );

  t->tick(13*500); // 12 seems to be bare minimum to receive ringbus and then flush output dma

  cout << "First CS30 out: (" << t->outs["cs30out"]->data.size() << ")" << endl;
  for(auto it = t->outs["cs30out"]->data.begin(); it != t->outs["cs30out"]->data.end(); it++) {
    // cout << "0x" << HEX_STRING(*it) << endl;
    assert( *it == 0 && "didn't get out zeros");
  }
  t->outs["cs30out"]->data.resize(0); // erase what CS30 has sent so far


  // test of an extended ringbus message write_vmem_sequence

  a0 = 0;
  a1 = 256;
  a2 = 0xf00;

  t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, EXTENDED_SR_CMD | a0 ) );
  t->tick(per_cmd);
  t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, EXTENDED_SR_CMD | a1 ) );
  t->tick(per_cmd);
  t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, EXTENDED_SR_CMD | a2 ) );
  t->tick(per_cmd);
  t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, EXTENDED_EXECUTE_CMD | ERB_WRITE_VMEM_SEQUENCE ) );
  t->tick(per_cmd);

  t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, DMA_OUT_CMD | 256 ) );

  t->tick(4*500);  // time to write memory
  t->tick(13*500); // time to flush out

  cout << "Second CS30 out: (" << t->outs["cs30out"]->data.size() << ")" << endl;
  uint32_t expected;
  expected = 0xf00;
  for(auto it = t->outs["cs30out"]->data.begin(); it != t->outs["cs30out"]->data.end(); it++) {
    assert( *it == expected && "didn't get expected value");
    expected++;
  }
  t->outs["cs30out"]->data.resize(0); // erase what CS30 has sent so far


  // now text extended ringbus dma_block_send();
  // this time we start sending half way into the previously written sequence
  a0 = 128;
  a1 = 128;
  t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, EXTENDED_SR_CMD | a0 ) );
  t->tick(per_cmd);
  t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, EXTENDED_SR_CMD | a1 ) );
  t->tick(per_cmd);
  t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS30, EXTENDED_EXECUTE_CMD | ERB_DMA_BLOCK_SEND ) );
  t->tick(per_cmd);
  t->tick(13*500); // time to flush out

  cout << "Third CS30 out: (" << t->outs["cs30out"]->data.size() << ")" << endl;
  expected = 0xf00 + 128;
  for(auto it = t->outs["cs30out"]->data.begin(); it != t->outs["cs30out"]->data.end(); it++) {
    assert( *it == expected && "didn't get expected value");
    expected++;
  }
  t->outs["cs30out"]->data.resize(0); // erase what CS30 has sent so far




  cout << "Ringbus got out" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }
  // t->outs["ringbusout"]->data.resize(0); // erase what we printed


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
