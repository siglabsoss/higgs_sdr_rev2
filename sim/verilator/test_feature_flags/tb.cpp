#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <bitset>
// Include common routines

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

#include <fstream>

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"

// also gets feedback_bus.h from riscv
#include "feedback_bus_tb.hpp"



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




VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
  return main_time; // Note does conversion to real, to match SystemC
}





int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

  // This helper is what I built to make this function easy
  // this handles reset.  You can register an arbitrary number of inputs
  // and outputs.
  // calling things like `inStreamAppend()` allows user to easily specify queue
  // input data which will be ticked over when tick is called
  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);

  unsigned int seed_start = std::time(0);

  unsigned int fixed_seed = 0; // set to non zero to use

  if(fixed_seed != 0) {
    seed_start = fixed_seed;
    cout << "starting with hard-coded seed " << seed_start << endl;
  } else {
    cout << "starting with random seed " << seed_start << endl;
  }

  srand(seed_start);



  preReset(top);

  t->reset(40);

  postReset(top);

  // tb inputs starts here
  // user can tick the clock for a period
  // append data to input streams, and look at output streams
  // modify negClock() and posClock() above
  // you can also insert for check streams from those functins()

  unsigned int adv = 8;

  int us = 35;

  // how many us to wait before first input
  unsigned us_base = 10;

  // cs10 does not boot until 10 us if it has empty global array of 1024
  const uint32_t pull_gap_maximum_us = 6;
  const uint32_t vector_type_pull_maximum_samples = 1024;
  const uint32_t stream_type_pull_maximum_samples = 256;

  const uint32_t send_types = 3;

  std::vector<uint32_t> types_pulls = {};

  unsigned int next_pull = us_base + rand() % pull_gap_maximum_us;


  uint32_t type_ringbus_state = 0x1000;
  uint32_t vector_type_pull;// = rand() & 0xffff;
  uint32_t stream_type_pull;


  for(unsigned int i = 0; i < us; i++) {

        // uint32_t which = rand() % send_types;
    

    if( i == 7 ) {
      t->inStreamAppend(
          "ringbusin", 
          ringbus_udp_packet(RING_ADDR_CS30, FEATURE_FLAGS_B_CMD )
       );
    } 

    // if( i == 8 ) {
    //   t->inStreamAppend(
    //       "ringbusin", 
    //       ringbus_udp_packet(RING_ADDR_CS30, FEATURE_FLAGS_A_CMD )
    //    );
    // } 

    // if i == pull
// if( i == 50 ) {
//       // t->tick(per_cmd);
//     }
        

        // std::vector<uint32_t> arbdata = {};
        // std::vector<uint32_t> streamdata = {};

        // types_pulls.push_back(which);

        // switch(which) {
        //     case 0:
        //         t->inStreamAppend("cs20in", feedback_ringbus_packet(
        //           RING_ENUM_CS10,
        //           EDGE_EDGE_IN | type_ringbus_state,
        //           RING_ENUM_CS10,
        //           EDGE_EDGE_IN | type_ringbus_state+1,
        //           RING_ENUM_CS10,
        //           EDGE_EDGE_IN | type_ringbus_state+2 )
        //            );
        //         type_ringbus_state += 3;
        //         types_pulls.push_back(0);
        //         break;

        //     case 1:
        //         stream_type_pull = (rand() % stream_type_pull_maximum_samples)+1;
        //         streamdata = counter_stream(stream_type_pull);
        //             t->inStreamAppend("cs20in", feedback_stream_packet(
        //             0,
        //             streamdata)
        //             );
        //         types_pulls.push_back(stream_type_pull);
        //         break;

        //     case 2:
        //         vector_type_pull = rand() % vector_type_pull_maximum_samples;
        //         for(uint32_t j = 0; j < vector_type_pull; j++) {
        //             arbdata.push_back(j);
        //         }
        //         t->inStreamAppend("cs20in", feedback_vector_packet(
        //             0x0,
        //             arbdata)
        //             );
        //         types_pulls.push_back(vector_type_pull);
        //         break;

        //     default:
        //         assert(0);
        //         break;
        // }

        // cout << "type " << which << " extra " << types_pulls[types_pulls.size()-1] << "        at us " << i << endl;

        // // how long does the switch type take to execute
        // switch(which) {
        //     case 0:
        //         // try this at 2
        //         next_pull += 1 + (rand() % pull_gap_maximum_us);
        //         break;
        //     case 1:
        //         // takes 38 us to process 512 samples
        //         next_pull += (float(stream_type_pull) / 512.0 * 38) + (rand() % pull_gap_maximum_us);
        //         break;
        //     case 2:
        //         next_pull += 1 + (rand() % pull_gap_maximum_us);
        //         break;
        //     default:
        //         assert(0);
        //         break;
        // }

    t->tick(500);

    // unsigned int cs20pc = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();

    // if(t->pc_exicted_main(cs20pc)) {
    //   cout << "Exiting early due to main() returning us(" << i << ") " << main_time  << endl;
    //   break;
    // }

  } // for us

  //   // wait for cs20 to flush out of previous commands, wait for things to GET to cs20 (half), also 
    t->tick(500*5);
// 
    cout << "Ringbus got out early" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
    }


    // check for type 1 and 2 (type 0 will check later)


    // finally, cs10ringbus should have sent us a rb message (the first one it sent, but we are checking later), which is the number of total rb
    // uint32_t rb_looking_count = (rb_looking) - 0x1000;
    // cout << "should find " << HEX32_STRING(rb_looking_count) << "near start" << endl;
    

    // assert(VECTOR_FIND(t->outs["ringbusout"]->data, rb_looking_count));


  // t->outStreamDump("cs30out");

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












  // // note to put an ENUM here not ADDR
    // if( i == 10 ) {
      
    // }

    // if( i == 20 ) {
    //   t->inStreamAppend("ringbusin", feedback_ringbus_packet(
    //   RING_ENUM_CS10,
    //   EDGE_EDGE_IN | 0xf0,
    //   RING_ENUM_CS10,
    //   EDGE_EDGE_IN | 0xf1 )
    //    );
    // }

    // if( i == 22 ) {
    //   t->inStreamAppend(0, feedback_ringbus_packet(
    //   RING_ENUM_CS10,
    //   EDGE_EDGE_IN | 0xf2 )
    //    );
    // }




    // if( i == 35 ) {
    //   std::vector<uint32_t> arbdata = {32768, 65536, 131072, 0x42, 0x10, 0, 0xf0000000,1,2,3};
    //   t->inStreamAppend(0, feedback_vector_packet(
    //   0xff,
    //   arbdata)
    //    );
    // }

    // 512 samples takes 38 us to xor
    // if( i == 35 ) {
    //   std::vector<uint32_t> streamdata = counter_stream(512);
    //   t->inStreamAppend(0, feedback_stream_packet(
    //   0,
    //   streamdata)
    //    );
    // }

    // if( i == 40 ) {
    //   t->inStreamAppend(0, feedback_ringbus_packet(
    //   RING_ENUM_CS10,
    //   EDGE_EDGE_IN | 0xd0,
    //   RING_ENUM_CS10,
    //   EDGE_EDGE_IN | 0xd1 )
    //    );
    // }

    // if( i == 12 ) {
    //   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS10, EDGE_EDGE_IN | 0x35 ) ); // call this up to 1024 times to store data
    //   // t->tick(per_cmd);
    // }

    // if( i == 14 ) {
    //   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS10, EDGE_EDGE_IN | 0x36 ) ); // call this up to 1024 times to store data
    //   // t->tick(per_cmd);
    // }

    // if( i == 16 ) {
    //   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS10, EDGE_EDGE_IN | 0x37 ) ); // call this up to 1024 times to store data
    //   // t->tick(per_cmd);
    // }




    // if( i > us_base && ((us_base + i) % adv) == 0) {
    //   t->inStreamAppend(0, swizzle_reverse_all(counter_stream(16)) );
    // }