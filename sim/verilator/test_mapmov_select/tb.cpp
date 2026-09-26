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
#include "mapmov.h"

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

    int us = 440-250+60;

    uint32_t amount = 5;
    uint32_t direction = 1;
    int custom_size;

    bool found1 = false;
    bool found2 = false;
    bool found3 = false;
    bool found4 = false;
    bool found5 = false;
    bool found6 = false;
    bool found7 = false;

    unsigned int pull1 = 16;
    std::vector<uint32_t> dumb;
    dumb.resize(pull1);
    auto check_awake = feedback_vector_packet(
                FEEDBACK_VEC_STATUS_REPLY,
                dumb,
                FEEDBACK_PEER_8, FEEDBACK_DST_HIGGS);

    auto check_initial_settings = 70+40;

    auto go_1 = 71;
    auto go_2 = 102;
    auto go_3 = 390-250;

    bool start_with_check = true;

    bool mm_found_0, mm_found_1, mm_found_2;
    mm_found_0 = mm_found_1 = mm_found_2 = false;
    
    uint32_t mm_enabled, mm_start, mm_end;
    mm_enabled = mm_start = mm_end = 0;

    const double enabled_subcarriers = 32;

    for(unsigned int i = 0; i < us; i++) {
        if(i == 70) {
            t->send_ring(RING_ADDR_ETH, MAPMOV_MODE_CMD | MAPMOV_MODE_DEBUG);
        }

        if(i == check_initial_settings) {
            for(auto it = t->outs["ringbusout"]->data.begin();
                it != t->outs["ringbusout"]->data.end(); it++) {
                auto data = *it;
                if( (data&0xff000000) == MAPMOV_MODE_REPORT  ) {
                    unsigned int dmode = (data >> 16) & 0xff;
                    unsigned int data_16 = data & 0xffff;
                    switch(dmode) {
                        case 1:
                            mm_found_0 = true;
                            mm_enabled = data_16;
                            std::cout << "Eth reported it was moving "
                                      << mm_enabled << " subcarriers\n";
                            break;
                        case 2:
                            mm_found_1 = true;
                            mm_start = data_16;
                            std::cout << "Eth reported it was start trim "
                                      << mm_start << "\n";
                            break;
                        case 3:
                            mm_found_2 = true;
                            mm_end = data_16;
                            std::cout << "Eth reported it was end trim "
                                      << mm_end << "\n";
                            break;

                    }
                }
            }
        }

        if(start_with_check &&  i == go_1 ) {
            t->inStreamAppend("cs11in", check_awake);
        }

        if(i == go_2) {
            std::vector<uint32_t> data;

            dataSync(&data, 0xcafebabe, 16);
            dataSync(&data, 0xdeadbeef, 16);
            dataSync(&data, 0xdeadbeef, 16);
            dataSync(&data, 0xdeadbeef, 16);

            std::cout << "About to send " << data.size() << " words\n";
            std::cout << "or " << data.size()*32 << " bits\n";
            std::cout << "or " << data.size()*16 << " subcarriers worth\n";

            const int header = 16;

            // Assumes that 0 and 1023 are the boundaries
            custom_size = header + \
                          ceil((data.size()*16)/enabled_subcarriers)*1024;
            auto packet = feedback_vector_packet_mapmov(
                                                    FEEDBACK_VEC_TX_USER_DATA,
                                                    data,
                                                    custom_size,
                                                    FEEDBACK_PEER_8,
                                                    FEEDBACK_DST_HIGGS);
            t->inStreamAppend("cs11in", packet);
        }

        if(start_with_check &&  i == go_3 ) {
            t->inStreamAppend("cs11in",check_awake);
        }

        t->tick(500);
    }

    for(auto it = t->outs["ringbusout"]->data.begin();
        it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
        if( (*it == FEEDBACK_ALIVE) ) {
            if(found1 == false) {
                found1 = true; // Check to see if fbbus is alive
            } else {
                found2 = true;
            }
        }
        if( (*it == 0xfe00001f) ) {
            found3 = true;
        }
        if( (*it == DEBUG_4_PCCMD) ) {
            found4 = true;
        }
        if( (*it == DEBUG_3_PCCMD) ) {
            std::cout << "Found fail marker\n";
            found5 = true;
        }
    }

    // More work at the end
    t->print_ringbus_out();

    t->outs["ringbusout"]->data.erase(t->outs["ringbusout"]->data.begin(),
                                      t->outs["ringbusout"]->data.end());

    // ask for mode?
    t->send_ring(RING_ADDR_ETH, REQUEST_MAPMOV_REPORT);

    t->tick(500*3);

    std::cout << "Ringbus got out after erase\n";
    for(auto it = t->outs["ringbusout"]->data.begin();
        it != t->outs["ringbusout"]->data.end(); it++) {
        std::cout << "0x" << HEX_STRING(*it) << "\n";
        if( (*it == (0x43040000 | MAPMOV_MODE_DEBUG) ) ) {
            found6 = true;
        }
        if( (*it == ( 0x43010000 | (int)enabled_subcarriers) ) ) {
            found7 = true;
        }
    }

    t->allStreamDump();

    assert(found1);
    assert(found2);
    assert(found3);
    assert(found4);
    assert(!found5);
    assert(found6);
    assert(found7);

    std::cout << "All Tests Passed\n";


    // Final model cleanup
    top->final();

    // Close trace if opened

    if (tfp) { tfp->close(); }

    // Destroy model
    delete top; top = NULL;

    exit(0);
}

