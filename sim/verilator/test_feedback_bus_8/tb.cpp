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

    int us = 290;

    uint32_t amount = 5;
    uint32_t direction = 1;
    int custom_size;

    bool found1 = false;
    bool found2 = false;
    int found3 = 0;
    int found4 = 0;
    int found5 = 0;
    bool found6 = false;
    int found6_val = 0;

    unsigned int fixed_seed = 0; // set to non zero to use

    setup_random(fixed_seed);

    auto go_2 = 86;

    for(unsigned int i = 0; i < us; i++) {
        if(i == go_2) {

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

        if( i == (250)) {
            std::cout << "Injecting dump errors command() at " << i << "\n";
            t->send_ring(RING_ADDR_CS11, CS11_REPORT_ERRORS_CMD);
        }

        if(i == 95 || i == 100 || i == 105 ||
           i == 110 || i == 115 || i == 120) {
            std::cout << "Injecting multi dump command() at " << i << "\n";
            t->send_ring(RING_ADDR_CS11, FB_REPORT_STATUS_CMD);
        }

        t->tick(500);
    }

    std::cout << "Ringbus got out\n";
    for(auto it = t->outs["ringbusout"]->data.begin();
        it != t->outs["ringbusout"]->data.end(); it++) {
        std::cout << "0x" << HEX_STRING(*it) << "\n";
        if( (*it == FEEDBACK_ALIVE) ) {
            if(found1 == false) {
                found1 = true; // Check to see if fbbus is alive
            } else {
                found2 = true;
            }
        }

        uint32_t normal_mask = (*it) & 0xff000000;

        switch(normal_mask) {
            case CS11_USERDATA_ERROR:
                std::cout << "CS11_USERDATA_ERROR " << HEX_STRING(*it) << "\n";
                found5++;
                break;
            default:
                break;
        }

        switch(*it) {
            case 0x5000000:
            case 0x5000001:
            case 0x5000002:
            case 0x5000003:
            case 0x5000004:
                found3++;
                break;
            default:
                break;
        }

        uint32_t upper_mask = (*it) & 0xffff0000;

        switch(upper_mask) {
            case 0x4b000000:
            case 0x4b010000:
            case 0x4b020000:
            case 0x4b030000:
            case 0x4b040000:
            case 0x4b050000:
            case 0x4b060000:
            case 0x4b070000:
            case 0x4b080000:
            case 0x4b090000:
            case 0x4b0a0000:
            case 0x4b0b0000:
            case 0x4b0c0000:
            case 0x4b0d0000:
            case 0x4b0e0000:
            case 0x4b0f0000:
            case 0x4b100000:
            case 0x4b110000:
            case 0x4b120000:
            case 0x4b130000:
                found4++;
                break;
            case 0x48a00000:
                found6 = true;
                found6_val = (*it)&0xfffff; // 5 mask
                break;
            default:
                break;
        }

    }

    assert(found3 == 5);

    assert(found5 == 1);

    std::cout << "Found REPORT2 " << found4 << " times\n";

    std::cout << "Reported that we overflowed and dropped "
              << found6_val << " ringbus\n";

    assert(found6);
    assert(found6_val > 5);


    t->allStreamDump();
 
    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) {tfp->close();}

    // Destroy model
    delete top; top = NULL;

    exit(0);
}

