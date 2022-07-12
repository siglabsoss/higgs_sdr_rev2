#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <bitset>
#include <assert.h>
#include <verilated.h>
#include <sys/stat.h>
#include <fstream>
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"
#include "cpp_utils.hpp"
#include "feedback_bus_tb.hpp"
#include <verilated_vcd_c.h>
#include "higgs_helper.hpp"
#include "piston_c_types.h"
#include "vmem_types.h"

typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
// Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
top_t* top = new top_t;
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

  unsigned int fixed_seed = 0; // Set to non zero to use

  // Did extensive testing with this seed. Cap is 20 us
  // fixed_seed = 1528427758;
  // Crashing because TB is not waiting until last item has run
  // fixed_seed = 1528540978;
  // Build 18 fails
  // fixed_seed = 1529277192;
  // Build 19 fails
  // fixed_seed = 1529290884;
  // Build 20 fails
  // fixed_seed = 1529300722;
  // Build 22 fails
  // fixed_seed = 1529320397;
  // Build 23 fails
  // fixed_seed = 1529335532;
  // fixed_seed = 1535847746;
  // fixed_seed = 1560088867; // fails on 9f710b3

  setup_random(fixed_seed);

  preReset(top);

  t->reset(40);

  postReset(top);

  int us = 80+90;

  // How many us to wait before first input
  unsigned us_base = 10+90;

  // CS01 does not boot until 10 us if it has empty global array of 1024
  const uint32_t pull_gap_maximum_us = 6;
  const uint32_t vector_type_pull_maximum_samples = 1024;
  const uint32_t stream_type_pull_maximum_samples = 256;

  const uint32_t send_types = 3;

  std::vector<uint32_t> types_pulls = {};

  unsigned int next_pull = us_base + rand() % pull_gap_maximum_us;


  uint32_t type_ringbus_state = 0x1000;
  uint32_t vector_type_pull;
  uint32_t stream_type_pull;


  for(unsigned int i = 0; i < us; i++) {
    if( i == next_pull ) {
        uint32_t which = rand() % send_types;

        std::vector<uint32_t> arbdata = {};
        std::vector<uint32_t> streamdata = {};

        types_pulls.push_back(which);

        switch(which) {
            case 0:
                t->inStreamAppend("cs11in", feedback_ringbus_packet(
                  RING_ENUM_CS01,
                  EDGE_EDGE_IN | type_ringbus_state,
                  RING_ENUM_CS01,
                  EDGE_EDGE_IN | type_ringbus_state + 1,
                  RING_ENUM_CS01,
                  EDGE_EDGE_IN | type_ringbus_state + 2));
                type_ringbus_state += 3;
                types_pulls.push_back(0);
                break;
            case 1:
                stream_type_pull = \
                                (rand() % stream_type_pull_maximum_samples) + 1;
                streamdata = counter_stream(stream_type_pull);
                    t->inStreamAppend("cs11in",
                                      feedback_stream_packet(0, streamdata));
                types_pulls.push_back(stream_type_pull);
                break;
            case 2:
                vector_type_pull = \
                                rand() % vector_type_pull_maximum_samples + 1;
                for(uint32_t j = 0; j < vector_type_pull; j++) {
                    arbdata.push_back(j);
                }
                t->inStreamAppend("cs11in",
                                  feedback_vector_packet(0x0, arbdata));
                types_pulls.push_back(vector_type_pull);
                break;
            default:
                assert(0);
                break;
        }

        std::cout << "Type " << which << " Extra "
                  << types_pulls[types_pulls.size()-1] << "        at us "
                  << i << "\n";
        // How long does the switch type take to execute
        switch(which) {
            case 0:
                // Try this at 2
                next_pull += 2 + (rand() % pull_gap_maximum_us);
                break;
            case 1:
                // Takes 38 us to process 512 samples
                next_pull += (float(stream_type_pull) / 512.0 * 38) + \
                             (rand() % pull_gap_maximum_us);
                break;
            case 2:
                next_pull += 10 + (rand() % pull_gap_maximum_us);
                break;
            default:
                assert(0);
                break;
        }
    }

    t->tick(500);
  }

  // Burn off time until we would have made our next pull
  if(next_pull > us) {
    uint32_t missing_us = next_pull - us;
    t->tick(500*missing_us);
  }
    // Wait for CS11 to flush out of previous commands
    // Wait for things to GET to CS11 (half)
    t->tick(500*24);
    t->print_ringbus_out();

    counter_stream(0,1); // RESET counter stream

    uint32_t type_2_looking = 0;

    // Since this our first loop, and we will not add any pulls, count them as
    // well
    uint32_t type0_count_final = 0;
    uint32_t type1_count_final = 0;
    uint32_t type2_count_final = 0;

    for(uint32_t i = 0; i < types_pulls.size(); i+=2) {
        uint32_t type = types_pulls[i];
        uint32_t extra = types_pulls[i+1];

        // Verification for type 2, feedback bus type VECTOR
        if(type == 2) {
            bool t2_found;

            type_2_looking += extra;

            t2_found = VECTOR_FIND(t->outs["ringbusout"]->data, type_2_looking);

            if( !t2_found ) {
                std::cout << "Type 2, index " << i/2 << " has issues\n";
            }

            assert(t2_found &&
                   "Type 2 (vector) message didn't find correct value");
            type2_count_final++;  // Counting, used outside this loop
        }

        if( type == 0) {
            type0_count_final++;  // Counting, used outside this loop
        }
        if( type == 1) {
            std::vector<uint32_t> expected_stream = counter_stream(extra);
            
            // since both tb and vex include this same file, we can run the
            // hash here in the tb to see what expected rb message we would get
            uint32_t expected = xorshift32(0, expected_stream.data(), extra);

            cout << "Hash for stream extra " << extra
                 << " should be " << HEX32_STRING(expected);

            assert(VECTOR_FIND(t->outs["ringbusout"]->data, expected) &&
                   "Type 1 (stream) didn't find correct hash");

            std::cout << " (it was)\n";

            type1_count_final++;  // counting, used outside this loop
        }
    }

    std::cout  << "Finished iterating types_pulls.size() at " << us << " us\n";

    // Wait for things to GET to CS11 (half)
    t->tick(500*24);
    // CS01 will dump everything it got back
    t->send_ring(RING_ADDR_CS01, EDGE_EDGE_OUT | 0);

    // We wait a variable amount of time for CS01 to dump the goodies 3.5 us for
    // each ringbus type we went, as CS01 has stored 3 ringbus for each
    t->tick(500*(15+(type0_count_final*3.5)));

    t->print_ringbus_out();

    // Check for 0 (ringbus)

    uint32_t rb_looking = 0x1000;

    for(uint32_t i = 0; i < types_pulls.size(); i+=2) {
        uint32_t type = types_pulls[i];
        uint32_t extra = types_pulls[i+1];
        
        if(type == 0) {
            bool found = true;
            found &= VECTOR_FIND(t->outs["ringbusout"]->data, rb_looking+0);
            found &= VECTOR_FIND(t->outs["ringbusout"]->data, rb_looking+1);
            found &= VECTOR_FIND(t->outs["ringbusout"]->data, rb_looking+2);
            rb_looking+=3;
            assert(found && "Did not find expected ringbus message");
        }
    }    

    // Finally, CS01 ringbus should have sent us a RB message which is the
    // number of total RB
    uint32_t rb_looking_count = (rb_looking) - 0x1000;
    assert(VECTOR_FIND(t->outs["ringbusout"]->data, rb_looking_count));

    for(uint32_t i = 0; i < types_pulls.size(); i+=2) {
        uint32_t type = types_pulls[i];
        uint32_t extra = types_pulls[i+1];
        cout << "type " << type << " extra " << extra << endl;
    }

  std::cout << "All Tests Passed\n";

  // Final model cleanup
  top->final();

  // Close trace if opened
  if (tfp) {tfp->close();}

  // Destroy model
  delete top; top = NULL;

  exit(0);
}
