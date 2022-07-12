#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
// Include common routines

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

#include <fstream>
#include <math.h>

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"
#include <verilated_fst_c.h>
#include "higgs_helper.hpp"


using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t,VerilatedFstC> helper_t;

#include "piston_c_types.h"
#include "vmem_types.h"
#include "schedule.h"
#include "feedback_bus.h"
#include "feedback_bus_types.h"
// #include "feedback_bus_tb.hpp"


void new_subcarrier_data_sync2(std::vector<uint32_t> *data, uint32_t sync, int subcarriers) {
    uint32_t temp1, temp2;
    uint32_t temp3;
    uint32_t sync_frame;
    for (int i = 30; i >= 0; i -= 2) {
        sync_frame = 0;
        temp1 = (sync >> i)&0x1;
        temp2 = (sync >> (i+1))&0x1;
        temp3 = (temp1) | (temp2<<1);
        for (int j = 0; j < 16; j++){
            sync_frame = (sync_frame << 2) | temp3;
        }
        for (int k = 0; k < subcarriers/16; k++) {
            data->push_back(sync_frame);
        }
    }
}




VerilatedFstC* tfp = NULL;
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


void test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    helper_t* t = new HiggsHelper<top_t,VerilatedFstC>(top,&main_time,tfp); 



    preReset(top);

    t->reset(40);

    postReset(top);

    // tb inputs starts here
    // user can tick the clock for a period
    // append data to input streams, and look at output streams
    // modify negClock() and posClock() above
    // you can also insert for check streams from those functins()

    // delay between sending inputs

    int us = 460;

    us = 200;

    uint32_t amount = 5;
    uint32_t direction = 1;
    int custom_size;

    // std::vector<uint32_t> vin = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

    bool found1 = false;
    bool found2 = false;

    // dump is here because we require size 1 due to a bug
    std::vector<uint32_t> dumb;
    dumb.resize(1);
    auto check_awake = feedback_vector_packet(
                FEEDBACK_VEC_STATUS_REPLY,
                dumb,
                FEEDBACK_PEER_8, FEEDBACK_DST_HIGGS);

    auto check_initial_settings = 70;

    auto go_1 = 80;

    // how long to inject the 2nd check
    int inject_after_mapmov = 1; // can be 0 to 16 probably

    auto inject_mapmov_at = 190;

    int adjust_offset = -10; // negative numbers means the packet will be sent later

    bool start_with_check = true;

    bool mm_found_0, mm_found_1, mm_found_2;
    mm_found_0 = mm_found_1 = mm_found_2 = false;
    
    uint32_t mm_enabled, mm_start, mm_end;
    mm_enabled = mm_start = mm_end = 0;

    for(unsigned int i = 0; i < us; i++) {

        if( false && i == (110) ) {
            cout << "Injecting dump command() at " << i << endl;
            // t->inStreamAppend(
            //     "ringbusin",
            //     ringbus_udp_packet(RING_ADDR_CS20, FB_REPORT_STATUS_CMD)
            // );
            
            t->inStreamAppend(
                "ringbusin",
                ringbus_udp_packet(RING_ADDR_CS20, CS20_MAIN_REPORT_STATUS_CMD)
            );
        }

        if(false && start_with_check &&  i == go_1 ) {
            t->inStreamAppend("cs20in", check_awake);
        }

        if(false && start_with_check &&  i == (go_1+28) ) {
            cout << "at firt time " << (go_1+28) << endl;
            auto saved = t->outs["ringbusout"]->data.begin();
            bool do_erase = false;
            for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
                cout << "0x" << HEX_STRING(*it);
                if( *it == FEEDBACK_ALIVE ) {
                    found1 = true; // look to see if fbbus is alive
                    cout << " <- erasing";
                    saved = it;
                    do_erase = true;
                }
                cout << endl;
            }
            if( do_erase ) {
                t->outs["ringbusout"]->data.erase(saved);
            }
        }


        // hc.RING_ADDR_CS20, ADD_LIFETIME_CMD | run 
        static const std::vector<uint32_t> advanceEpoc = {
            RESET_LIFETIME_CMD,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 8388607,
            ADD_LIFETIME_CMD | 3428358,
        };

        // send the vector above
        // auto f = meteredRingbusSendUni<top_t>(t, advanceEpoc, 92, RING_ADDR_CS20, 4);
        // f(i);
        // cout << "inputing " << i << endl;
        // if( i >= 90 && i <= 150 ) {
        //     unsigned sel = i-90;
        //     if( sel < advanceEpoc.size() ) {
        //         t->inStreamAppend(
        //             "ringbusin",
        //             ringbus_udp_packet(RING_ADDR_CS20, advanceEpoc[sel])
        //         );
        //     }
        // }

        if( i == inject_mapmov_at ) {
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
            dataSync(&data, 0xdeadbeef, 16);
            dataSync(&data, 0xdeadbeef, 16);
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

            const int header = 16;
            const double enabled_subcarriers = 128;

            cout << "About to send " << data.size() << " words " << endl;
            cout << "or " << data.size()*32 << " bits" << endl;
            cout << "or " << data.size()*16 << " subcarriers worth " << endl;
            cout << "or " << (double)data.size()*16.0/(double)enabled_subcarriers << " frames worth " << endl;


            uint32_t epoc = 1;     // seq2
            uint32_t timeslot = 0x11; // seq

            // assumes that 0 and 1023 are the boundaries
            custom_size = header + ceil((data.size()*16)/enabled_subcarriers)*1024;

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



        if( i == (inject_mapmov_at + inject_after_mapmov) ) {
            t->inStreamAppend("cs20in", check_awake);
        }

        t->tick(500);
    }

    if(!found1) {
        cout << "found1 was NOT set, something is wrong" << endl;
    } else {
        cout << "found1 was set" << endl;
    }

    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        if( *it == FEEDBACK_ALIVE ) {
            found2 = true; // look to see if fbbus is alive
        }
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
    cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
    file_dump_T<cs20_node_t>(cs20_node,"cs20_vmem.out");

    // cout << "All Tests Passed" << endl;

    // assert(found1);
    // assert(found2);

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









int main(int argc, char** argv, char** env) {
    uint32_t test_select = 0;

    if(const char* env_p = std::getenv("TEST_SELECT")) {
        unsigned int env_test_select = atoi(env_p);
        cout << "environment variable TEST_SELECT was set to: " << env_test_select << endl;
        test_select = env_test_select;
    }


    unsigned int fixed_seed = 0; // set to non zero to use
    setup_random(fixed_seed);

    switch(test_select) {
        case 0:
            test0(argc, argv, env);
            break;
        default:
            cout << "Invalid test selected" << endl;
            exit(1);
            break;
    }

}