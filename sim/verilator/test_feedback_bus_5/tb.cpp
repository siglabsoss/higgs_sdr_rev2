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

void dataSync(std::vector<uint32_t> *data, uint32_t sync, int subcarriers) {
    uint32_t temp1, temp2;
    uint32_t sync_frame;
    for (int i = 30; i >= 0; i -= 2) {
        sync_frame = 0;
        temp1 = (sync >> i)&0x1;
        temp2 = (sync >> i+1)&0x1;
        for (int j = 0; j < 32; j++){
            if(j<16){
                sync_frame = (sync_frame << 1)|(temp2);
            } else {
                sync_frame = (sync_frame << 1)|(temp1);
            }
        }
        for (int k = 0; k < subcarriers/16; k++) {
            data->push_back(sync_frame);
        }
    }
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

  // fixed_seed = 1537163975;

  setup_random(fixed_seed);



  preReset(top);

  t->reset(40);

  postReset(top);

  // tb inputs starts here
  // user can tick the clock for a period
  // append data to input streams, and look at output streams
  // modify negClock() and posClock() above
  // you can also insert for check streams from those functins()


  unsigned int adv = 8;

  int us = 80+90;

  // how many us to wait before first input
  unsigned us_base = 10+90;

  // cs10 does not boot until 10 us if it has empty global array of 1024
  const uint32_t pull_gap_maximum_us = 6;
  const uint32_t vector_type_pull_maximum_samples = 1024;
  const uint32_t stream_type_pull_maximum_samples = 256;

  const uint32_t send_types = 3;

  std::vector<uint32_t> types_pulls = {};

  unsigned int next_pull = us_base + rand() % pull_gap_maximum_us;

    unsigned int pull_count = 0;
    unsigned int pull_count_limit = 2 + (rand()%2); // 2 or 3

  uint32_t type_ringbus_state = 0x1000;
  uint32_t vector_type_pull;// = rand() & 0xffff;
  uint32_t stream_type_pull;


  for(unsigned int i = 0; i < us; i++) {

    if( i == next_pull ) {
        uint32_t which = rand() % send_types;

        if( which == 0 ) {
            which++; // ringbus type is illegal now
        }

        

        std::vector<uint32_t> arbdata = {};
        std::vector<uint32_t> streamdata = {};

        types_pulls.push_back(which);

        switch(which) {
            case 0:
                t->inStreamAppend("cs20in", feedback_ringbus_packet(
                  RING_ENUM_CS10,
                  EDGE_EDGE_IN | type_ringbus_state,
                  RING_ENUM_CS10,
                  EDGE_EDGE_IN | type_ringbus_state+1,
                  RING_ENUM_CS10,
                  EDGE_EDGE_IN | type_ringbus_state+2 )
                   );
                type_ringbus_state += 3;
                types_pulls.push_back(0);
                break;

            case 1:
                stream_type_pull = (rand() % stream_type_pull_maximum_samples)+1;
                streamdata = counter_stream(stream_type_pull);
                    t->inStreamAppend("cs20in", feedback_stream_packet(
                    0,
                    streamdata)
                    );
                types_pulls.push_back(stream_type_pull);
                break;

            case 2:
                vector_type_pull = rand() % vector_type_pull_maximum_samples+1;
                for(uint32_t j = 0; j < vector_type_pull; j++) {
                    arbdata.push_back(j);
                }
                t->inStreamAppend("cs20in", feedback_vector_packet(
                    0x0,
                    arbdata)
                    );
                types_pulls.push_back(vector_type_pull);
                break;

            default:
                assert(0);
                break;
        }

        cout << "type " << which << " extra " << types_pulls[types_pulls.size()-1] << "        at us " << i << endl;

        // how long does the switch type take to execute
        switch(which) {
            case 0:
                // try this at 2
                next_pull += 2 + (rand() % pull_gap_maximum_us);
                break;
            case 1:
                // takes 38 us to process 512 samples
                next_pull += (float(stream_type_pull) / 512.0 * 38) + (rand() % pull_gap_maximum_us);
                break;
            case 2:
                next_pull += 10 + (rand() % pull_gap_maximum_us);
                break;
            default:
                assert(0);
                break;
        }


        pull_count++;
    } // if i == pull

    t->tick(500);


    if(pull_count == pull_count_limit) {
        break;
    }

    // unsigned int cs20pc = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();

    // if(t->pc_exicted_main(cs20pc)) {
    //   cout << "Exiting early due to main() returning us(" << i << ") " << main_time  << endl;
    //   break;
    // }

  } // for us

    t->tick(500*7);

  // burn off time untill we would have made our next pull
  if(next_pull > us) {
    uint32_t missing_us = next_pull - us;
    t->tick(500*missing_us);
  }

    // 0 is first
    // 1 is second
    // 2 is both

    int corruption_mode = (rand() % 3);


    bool do_outside_sabotage = corruption_mode == 0 || corruption_mode == 2;

     // perfectly good code for doing an outside jam
    if(do_outside_sabotage) {
        // randomly jam
        uint32_t jam_amount = (rand() % 22) + 1;
        uint32_t jam_with = 3;

        cout << " ---- will do outside jam with " << jam_amount << " words" << endl;

        std::vector<uint32_t> do_jam;
        for(auto i = 0; i < jam_amount; i++) {
            do_jam.push_back(jam_with);
        }

        t->inStreamAppend("cs20in", do_jam);
    }


    bool do_inside_sabotage = corruption_mode == 1 || corruption_mode == 2;

    if( do_inside_sabotage ) {
        // can bo up to 0x7f
        uint32_t pull_bump = rand() % 0xf;

        cout << "Sending inside sabotage command for " << pull_bump << " samples at time " << t->us() << endl;

        t->inStreamAppend(
            "ringbusin",
            ringbus_udp_packet(RING_ADDR_CS20, SABOTAGE_CMD | pull_bump)
        );
        
    }




    // wait for cs20 to flush out of previous commands, wait for things to GET to cs20 (half), also 
    t->tick(500*30);


    cout << "Ringbus got out early" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
    }
    t->outs["ringbusout"]->data.erase(t->outs["ringbusout"]->data.begin(), t->outs["ringbusout"]->data.end());


    cout << "would send reset at " << t->us() << endl;
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_ETH, MAPMOV_RESET_CMD) );
    // this reset takes FOREVER to internally run
    t->tick(500*90);


    // try and recover from the jam
    int recover_us = 120*1.5;

    for(unsigned int i = 0; i < recover_us; i++) {

        if( i % 5 == 0 && i < 50 ) {
            auto zero_pack = 10 + (rand() % 5);
            t->inStreamAppend("cs20in", feedback_flush_packet( zero_pack ) );
            cout << "at " << t->us() << " injecting " << zero_pack << " zeros" << endl;
        }

        // if( i == 50 ) {
        //     t->inStreamAppend("cs20in", SUGGESTED_FEEDBACKBUS_FLUSH_TAIL() );
        // }


        // if( i == 50 || i == 55 || i == 60 || i == 65 ) {
        //     cout << "at " << t->us() << " sent flush tail " << " (" << i << ")" << endl;
        //     t->inStreamAppend("cs20in", feedback_unjam_packet() );
        // }


        if( i == 70 ) {
            std::vector<uint32_t> dumb;
            dumb.resize(16);
            auto check_awake = feedback_vector_packet(
                        FEEDBACK_VEC_STATUS_REPLY,
                        dumb,
                        0,
                        FEEDBACK_DST_HIGGS);
            t->inStreamAppend("cs20in", check_awake);
        }

        if( i == 72 ) {
            std::vector<uint32_t> data;
            dataSync(&data, 0xcafebabe, 16);
            // dataSync(&data, 0xdeadbeef, 16);
            // dataSync(&data, 0xdeadbeef, 16);
            // dataSync(&data, 0xdeadbeef, 16);
            cout << "About to send " << data.size() << " words " << endl;
            cout << "or " << data.size()*32 << " bits" << endl;
            cout << "or " << data.size()*16 << " subcarriers worth " << endl;

            const int header = 16;
            const double enabled_subcarriers = 128;

            uint32_t epoc = 1;     // seq2
            uint32_t timeslot = 0x11; // seq

            // assumes that 0 and 1023 are the boundaries
            auto custom_size = header + ceil((data.size()*16)/enabled_subcarriers)*1024;

            cout << "custom size " << custom_size << endl;

            auto packet = 
                feedback_vector_mapmov_scheduled(
                    FEEDBACK_VEC_TX_USER_DATA, 
                    data, 
                    custom_size, 
                    FEEDBACK_PEER_8, 
                    FEEDBACK_DST_HIGGS,
                    timeslot,
                    epoc
                    );

            t->inStreamAppend("cs20in", packet);
        }

        // unsigned delay_pull = rand() % 20;

        // if( i == 73 + delay_pull ) {
            
        // }

        if( i == 110 ) {

        }


        t->tick(500);
    }

    t->tick(500*(24+25));

    std::vector<uint32_t> dumb;
    dumb.resize(16);
    auto check_awake = feedback_vector_packet(
                FEEDBACK_VEC_STATUS_REPLY,
                dumb,
                0,
                FEEDBACK_DST_HIGGS);
    t->inStreamAppend("cs20in", check_awake);


    t->tick(500*(25));



    bool found1 = false;
    bool found2 = false;

    cout << "Ringbus got out End" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {

        if( (*it == FEEDBACK_ALIVE) ) {
            if(found1 == false) {
                found1 = true; // look to see if fbbus is alive
            } else {
                found2 = true;
            }
        }

        cout << "0x" << HEX_STRING(*it) << endl;
    }

    if(!found1) {
        cout << "found1 was NOT set, something is wrong" << endl;
    } else {
        cout << "found1 was set" << endl;
    }

    if(!found2) {
        cout << "found2 was NOT set, something is wrong" << endl;
    } else {
        cout << "found2 was set" << endl;
    }



    for(uint32_t i = 0; i < types_pulls.size(); i+=2) {
        uint32_t type = types_pulls[i];
        uint32_t extra = types_pulls[i+1];
        cout << "type " << type << " extra " << extra << endl;
    }









  t->allStreamDump();


    assert(found1);
    assert(found2);

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
