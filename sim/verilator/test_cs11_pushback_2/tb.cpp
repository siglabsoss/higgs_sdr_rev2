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

    // fixed_seed = 1536045987;  // first failure, seems like it glitches for 1 sample
    // fixed_seed = 1560674931;
    // fixed_seed = 1560676363;

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


    // unsigned int adv = 8;

    int us = 200; // worst case for loop, impossible big

    // us = 140;

    uint32_t pull1 = rand() & 0xffffff;
    uint32_t pull2 = rand() & 0xf;
    cout << "pull 1 0x" << HEX32_STRING(pull1) << "\n";
    cout << "pull 2 0x" << HEX32_STRING(pull2) << "\n";


    // select values that are not too large
    // we do not pay attention to feedback bus packets here
    // so we must choose values that will be ok if parsed as any field in the mapmov
    // we choose value that are not 2 or 5 as to avoid a mapmov type, and then we choose
    // ones that are small enough to be parsed as lengths
    std::vector<uint32_t> vinput;
    std::vector<uint32_t> choices = {0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30};


    for( int i = 0; i < 4096; i++ ) {
        unsigned select = (i+pull2)%choices.size();
        vinput.push_back(choices[select]);
    }

    for(int i = 1070; i < 1080; i++) {
        vinput[i] = i;
    }

    file_dump_vec(vinput, "ideal.hex");



    
    unsigned int i = 0;
    for(i = 0; i < us; i++) {

        if( i == 15 ) {
            t->send_ring(RING_ADDR_TX_0, SEED_RANDOM_CMD | pull1);
        }

        if( i == 20 ) {
            t->inStreamAppend("cs11in", vinput);
        }
        
        t->tick(500);
    }



  cout << "Ringbus got out" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }

    t->allStreamDump();

    top->final();
    if (tfp) { tfp->close(); }


    // positive is short
    int length_difference = vinput.size() - t->monitors["mcs11in"]->data.size();

    bool show_diff = false;
    bool fail = false;

    if( length_difference < 0 ) {
        cout << "Got too much data, wrong!\n";
        show_diff = true;
        fail = true;
    }

    if( length_difference > 0x30 ) {
        cout << "output length was " << length_difference << " which is way too short\n";
        show_diff = true;
        fail = true;
    }

    // here we may have a small difference in length, this is ok due to how the mapmov/fb bus
    // work

    for(int i = 0; i < t->monitors["mcs11in"]->data.size(); i++) {
        if( t->monitors["mcs11in"]->data[i] != vinput[i] ) {
            fail = true;
            show_diff = true;
            cout << "Failed to match at position " << i << "\n";
            break;
        } 
    }


    // for(const auto w : t->monitors["mcs11in"]->data) {
    //     cout << HEX32_STRING(w) << "\n";
    // }


    usleep(1000);

    if( show_diff ) {

        auto packed = CmdRunner::runOnce("diff cs11_in.hex ideal.hex");

        int retval = std::get<0>(packed);
        std::string output = std::get<1>(packed);

        if( retval != 0) {
            cout << "Data was not correctly injected through eth -> mapmov -> cs11\n";
            cout << "Data failed with diff:\n\n";
            cout << output << "\n";
        } else {
            cout << "Data made it through correctly\n";
        }

        if( !fail ) {
            // chance to fail from diff as well
            if( retval != 0 ) {
                fail = true;
            }
        }
    }

    if( fail ) {
        exit(1);
    }

    exit(0);
}
