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


#include "ringbus.h"
#include "ringbus2_pre.h"
// #include "tb_ringbus.hpp"

std::string lookup_ringbus_enum( unsigned int v, bool upper = false );

#include "higgs_helper.hpp"


#include <verilated_vcd_c.h>


#define RESET MIB_MASTER_RESET


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

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



    unsigned int seed_start = std::time(0);

    unsigned int fixed_seed = 0; // set to non zero to use

    fixed_seed = 1536045987;  // first failure, seems like it glitches for 1 sample
    // 1536047291
    // 1536047339

    if(const char* env_p = std::getenv("TEST_SEED")) {
        unsigned int env_seed = atoi(env_p);
        cout << "environment variable TEST_SEED was set to: " << env_seed << endl;
        fixed_seed = env_seed;
    }

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


    unsigned int adv = 8;

    int us = 2000; // worst case for loop, impossible big

    unsigned int seeds_sent = 0;

    unsigned int what_to_do = 0;

    unsigned us_base = 10;

    const uint32_t counter_start = 0xf0000000;
    uint32_t counter_value = counter_start;

    // how many samples on the tail, this gives us about 5/10 us to spare
    const uint32_t fixed_tail = 260;

    const uint32_t initial_pull_max = 30;
    const uint32_t pull_gap_maximum_us = 1 + (rand() % 6) ;
    const uint32_t pull_maximum_samples = (rand() % 2048) + 9;
    bool first_loop_done = false;
    unsigned int next_pull = us_base + (rand() % initial_pull_max);


    cout << "starting with: " << endl;
    cout << "  initial_pull:             " << next_pull << endl;
    cout << "  pull_gap_maximum_us:      " << pull_gap_maximum_us << endl;
    cout << "  pull_maximum_samples:     " << pull_maximum_samples << endl;
    cout << endl;

    
    unsigned int i = 0;
    for(i = 0; i < us; i++) {
        if( i == next_pull ) {
            uint32_t sam = (rand() % pull_maximum_samples);


            auto start = counter_value;
            auto end = counter_value+sam;


            if( end >= (counter_start + 0x1000) ) {
                end = (counter_start + 0x1000);
                first_loop_done = true;
            }

            cout << " injecting  " << (end-start) << "    samples " << "        at us " << i << endl;

            auto values = get_counter(start, end);

            t->inStreamAppend("cs11in", values);
            counter_value = end;

            next_pull += 1 + (rand() % pull_gap_maximum_us);
        }

        if( first_loop_done ) {
            break;
        }

        t->tick(500);
    }

    if( !first_loop_done ) {
        assert(false && "first loop ran too short");
    }

    cout << " first loop done at " << i << endl;

    t->tick(500*fixed_tail); // flush (1us per fpga)

    cout << " tail done at " << i + fixed_tail << endl;

  cout << "Ringbus got out" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }


  unsigned int num_passed = 0;

  cout << "Results:" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    if( (*it & 0xf0000000) == 0xf0000000 ) {
      unsigned int fpga = (*it & 0x000f0000) >> 16;
      unsigned int error = (*it & 0x0000ffff);
      cout << "FPGA " << fpga << " " << lookup_ringbus_enum(fpga) << " reported " << HEX_STRING(error);
      if( error == 0 ){
        cout << "   passed" << endl;

        // FIXME: one fpga reporting 6 times can FOOL THIS!
        if( 
          fpga == RING_ENUM_CS11
          ) {
          num_passed++;
        }

      } else {
        cout << "   failed" << endl;
        exit(1);
      }
    }
    cout << "0x" << HEX_STRING(*it) << endl;
  }

    cs11_node_t* cs11_node = top->tb_higgs_top->cs11_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
    // hexdump_vmem<cs11_node_t>(cs11_node,"CS11", 0, 1024, false);

    // t->allStreamDump();



    // Close trace if opened


    // Destroy model
    // delete top; top = NULL;

    assert( num_passed == 1 && "Some FPGA's did not pass internally");
    
    cout << "All Tests Passed" << endl;


    // Final model cleanup
    top->final();
    if (tfp) { tfp->close(); }
    //print_vector(output_vector);
    // Fin
    exit(0);
}
