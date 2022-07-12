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

void dataSync(std::vector<uint32_t> *data, uint32_t sync, int subcarriers) {
    uint32_t temp1, temp2;
    uint32_t sync_frame;
    for (int i = 30; i >= 0; (i = i - 2)) {
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

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 

    srand(1);

    preReset(top);

    t->reset(40);

    postReset(top);

    unsigned int adv = 20;

    int us = 600;

    uint32_t amount = 5;
    uint32_t direction = 1;
    int custom_size;

    bool found1 = false;
    bool found2 = false;

    unsigned int fixed_seed = 0;

    setup_random(fixed_seed);

    auto go_2 = 86;

    for(unsigned int i = 0; i < us; i++) {


        if( i == go_2 ) {

            std::vector<uint32_t> data;

            dataSync(&data, 0xbeefbabe, 128);
            
            auto counter_start = 0xdead0000;

            for(auto j = 0; j < 128*2; j++) {
                data.push_back(counter_start + j);
            }

            std::cout << "About to send " << data.size() << " words\n";
            std::cout << "or " << data.size()*32 << " bits\n";
            std::cout << "or " << data.size()*16 << " subcarriers worth\n";

            const int header = 16;
            const double enabled_subcarriers = 128;

            // Assumes that 0 and 1023 are the boundaries
            custom_size = header + \
                            ceil((data.size()*16)/enabled_subcarriers)*1024;
            auto packet = feedback_vector_packet_mapmov(
                                                    FEEDBACK_VEC_TX_USER_DATA,
                                                    data,
                                                    custom_size,
                                                    FEEDBACK_PEER_8,
                                                    FEEDBACK_DST_HIGGS);
            t->inStreamAppend("cs11in",packet);
        }


        t->tick(500);
    }

    for(auto it = t->outs["ringbusout"]->data.begin();
        it != t->outs["ringbusout"]->data.end(); it++) {
        std::cout << "0x" << HEX_STRING(*it) << "\n";
        if( (*it == FEEDBACK_ALIVE) ) {
            if(found1 == false) {
                found1 = true;
            } else {
                found2 = true;
            }
        }
    }

    t->print_ringbus_out();

    t->allStreamDump();
 
    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) {tfp->close();}

    // Destroy model
    delete top; top = NULL;

    exit(0);
}
