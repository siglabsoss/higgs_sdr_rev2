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

    // tb inputs starts here
    // user can tick the clock for a period
    // append data to input streams, and look at output streams
    // modify negClock() and posClock() above
    // you can also insert for check streams from those functins()

    // delay between sending inputs
    unsigned int adv = 20;

    int us = 440-250;

    uint32_t amount = 5;
    uint32_t direction = 1;
    int custom_size;

    bool found1 = false;
    bool found2 = false;


    unsigned int fixed_seed = 0; // set to non zero to use

    setup_random(fixed_seed);

    // dump is here because we require size 1 due to a bug
    unsigned int pull1 = (rand()%15) + 1;
    cout << "Pull 1: " << pull1 << endl;
    std::vector<uint32_t> dumb;
    dumb.resize(pull1);
    auto check_awake = feedback_vector_packet(
                FEEDBACK_VEC_STATUS_REPLY,
                dumb,
                FEEDBACK_PEER_8, FEEDBACK_DST_HIGGS);

    auto check_initial_settings = 70;

    auto go_1 = 71;
    auto go_2 = 86;
    auto go_3 = 390-250;

    bool start_with_check = true;

    bool mm_found_0, mm_found_1, mm_found_2;
    mm_found_0 = mm_found_1 = mm_found_2 = false;
    
    uint32_t mm_enabled, mm_start, mm_end;
    mm_enabled = mm_start = mm_end = 0;

    for(unsigned int i = 0; i < us; i++) {

        if( i == 10 ) {
            auto pull2 = (rand() & 0xffffff);
            cout << "Pull 2: " << pull2 << endl;
            t->inStreamAppend(
                "ringbusin",
                ringbus_udp_packet(RING_ADDR_CS20, SEED_RANDOM_CMD | pull2)
            );
        }

        if( i == check_initial_settings ) {
            for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
                // cout << "0x" << HEX_STRING(*it) << endl;
                auto data = *it;
                if( (data&0xff000000) == MAPMOV_MODE_REPORT  ) {
                    unsigned int dmode = (data >> 16) & 0xff;
                    unsigned int data_16 = data & 0xffff;
                    switch(dmode) {
                        case 1:
                            mm_found_0 = true;
                            mm_enabled = data_16;
                            cout << "Eth reported it was moving " << mm_enabled << " subcarriers" << endl;
                            break;
                        case 2:
                            mm_found_1 = true;
                            mm_start = data_16;
                            cout << "Eth reported it was start trim " << mm_start << endl;
                            break;
                        case 3:
                            mm_found_2 = true;
                            mm_end = data_16;
                            cout << "Eth reported it was end trim " << mm_end << endl;
                            break;

                    }
                }
            }
        }

        if(start_with_check &&  i == go_1 ) {
            t->inStreamAppend("cs20in",check_awake);
        }

        // if(start_with_check && i == (go_1+30) ) {
        //     cout << "at first time " << (go_1+30) << endl;
        //     for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        //         cout << "0x" << HEX_STRING(*it) << endl;
        //         if( *it == FEEDBACK_ALIVE ) {
        //             found1 = true; // look to see if fbbus is alive
        //             t->outs["ringbusout"]->data.erase(it);
        //             break;
        //         }
        //     }
        //     cout << "--------" << endl << "above ringbus will be repeated below however 1 ringbus may be deleted" << endl << "--------" << endl;
        // }


        if( i == go_2 ) {
            // std::vector<uint32_t> sch;
            // sch.resize(SCHEDULE_SLOTS);
            // for(auto& n : sch) {
            //     // n = feedback_peer_to_mask(tx_peer_0);
            //     n = 0;
            //     }
            std::vector<uint32_t> data;

            // cout << "data: " << endl;
            // for(auto it = data.begin(); it != data.end(); it++) {
            //     cout << "0x" << HEX_STRING(*it) << endl;
            // }
            // cout << endl << endl;;

            // data.resize(8);

            dataSync(&data, 0xcafebabe, 16);
            dataSync(&data, 0xdeadbeef, 16);
            dataSync(&data, 0xdeadbeef, 16);
            dataSync(&data, 0xdeadbeef, 16);

            // cout << "data: " << endl;
            // for(auto it = data.begin(); it != data.end(); it++) {
            //     cout << "0x" << HEX_STRING(*it) << endl;
            // }
            // cout << endl << endl;;


            //dataSync(&data, 0xabcddead, 16);

            // feedback_vector_packet_mapmov

            cout << "About to send " << data.size() << " words " << endl;
            cout << "or " << data.size()*32 << " bits" << endl;
            cout << "or " << data.size()*16 << " subcarriers worth " << endl;



            const int header = 16;
            const double enabled_subcarriers = 128;

            // assumes that 0 and 1023 are the boundaries
            custom_size = header + ceil((data.size()*16)/enabled_subcarriers)*1024;
            auto packet = feedback_vector_packet_mapmov(FEEDBACK_VEC_TX_USER_DATA, data, custom_size, FEEDBACK_PEER_8, FEEDBACK_DST_HIGGS);

            t->inStreamAppend("cs20in",packet);
        }


        if(start_with_check &&  i == go_3 ) {
            t->inStreamAppend("cs20in",check_awake);
        }

        // if(start_with_check &&  i == (go_3+30) ) {
        //     cout << "at firt time " << (go_3+30) << endl;
        //     for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        //         cout << "0x" << HEX_STRING(*it) << endl;
        //         if( (*it == FEEDBACK_ALIVE) ) {
        //             if(found1 == false) {
        //                 found1 = true; // look to see if fbbus is alive
        //             } else {
        //                 found2 = true;
        //             }
        //         }
        //     }
        //     cout << "--------" << endl << "above ringbus will be repeated below" << endl << "--------" << endl;
        // }

        t->tick(500);
    }

    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
        if( (*it == FEEDBACK_ALIVE) ) {
            if(found1 == false) {
                found1 = true; // look to see if fbbus is alive
            } else {
                found2 = true;
            }
        }
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

    cout << "Ringbus got out" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
    }

    // cout << "CS10 sent to dac:" << endl;
    // for(auto it = t->outs[1].data.begin(); it != t->outs[1].data.end(); it++) {
    //   cout << "0x" << HEX_STRING(*it) << endl;
    // }

    t->allStreamDump();
    // cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
    // file_dump_T<cs20_node_t>(cs20_node,"cs20_vmem.out");

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

