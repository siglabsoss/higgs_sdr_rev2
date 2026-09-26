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
#include <verilated_vcd_c.h>
#include "higgs_helper.hpp"


using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

#include "piston_c_types.h"
#include "vmem_types.h"
#include "schedule.h"
#include "feedback_bus.h"
#include "feedback_bus_types.h"
// #include "feedback_bus_tb.hpp"




VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
    return main_time; // Note does conversion to real, to match SystemC
}


int32_t epoc_recent_reply_frames;



void test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    helper_t* t = new helper_t(top,&main_time,tfp); 


    preReset(top);

    t->reset(40);

    postReset(top);



    int us = 300; //3000; // maybe 1300 total
    int data_end_us = 1032; // time when mammov is consumed

    us = 430; // this value finds the error when both eth and cs20 dont have counter stops

    us = 60;



    bool first_pull = true;
    int next_pull = 84;


    t->registerRb([&](const uint32_t word) {
        cout << "cb 1 0xff " << HEX32_STRING(word) << "\n";
    }, 0xff000000);

    t->registerRb([&](const uint32_t word) {
        cout << "cb 2 0xde " << HEX32_STRING(word) << "\n";
    }, 0xde000000);

    for(int i = 0; i < us; i++) {

        if( false ) {
            // us needs to be about 300 for all these to flush out
            static const std::vector<uint32_t> reportRb = {
                CS20_REPORT_ERRORS_CMD,
                FB_REPORT_STATUS_CMD
            };
            auto injectReport = meteredRingbusSendUni<top_t>(t, reportRb, (data_end_us+30), RING_ADDR_CS20);
            injectReport(i);
        }

        t->tick(500);
    }


    if( false ) {
        for(auto x : t->outs8["ethbyteout"]->data ) {
            for(auto y : x ) {
                cout << HEX8_STRING((int)y);
            }
            cout << "\n\n";
        }
    }

    // t->parseEthRxPackets();



    int found1 = 0;
    int found2 = 0;
    bool found3 = false;
    int found4 = 0;
    uint32_t schedule_reply;

    cout << "Ringbus got out" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
        uint32_t cmd = (*it & 0xff000000);
        // if((*it & 0xff000000)==CS20_FILL_LEVEL_PCCMD) {
        //     // schedule_reply = ;
        //     // found3 = true;
        //     handleFillLevelReply(*it);
        // }
        // if( *it == 0xdead0202 ) {
        //     found1++;
        // }
        // if( *it == 0xdead0207 ) {
        //     found2++;
        // }
        // if( cmd == FEEDBACK_ALIVE ) {
        //     found4++;
        // }
        

    }

    // if(found3) {
        
    // }







    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        uint32_t cmd = (*it & 0xff000000);

        switch(cmd) {
            case CS20_USERDATA_ERROR: {
                auto msg = getErrorStringFeedbackBusParse(*it);
                cout << msg;
            }
            break;
            default:
                break;
        }
    }

    bool pass = found4 > 1;

    // cout << "Found " << found4 << " check alive replies" << endl;

    // if( pass ) {
    //     cout << "!!!!!!!!!!!!!!!! ALL PASSED !!!!!!!!!!!!!!!!" << endl;
    // } else {
    //     cout << "!!!!!!!!!!!!!!!! ALL FAILED !!!!!!!!!!!!!!!!" << endl;
    // }

    // assert(pass);

    t->allStreamDump();


    // cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
    // file_dump_T<cs20_node_t>(cs20_node,"cs20_vmem.out");

    cout << "Eth data got " << t->outs8["ethbyteout"]->data.size() << " packets" << endl;

    if( false ) {
        cout << "Packet list --- " << endl;
        for(auto x : t->outs8["ethbyteout"]->data ) {
            for( auto y : x ) {
                cout << HEX_STRING((int)y) << ",";
            }
            cout << endl;
        }
    }


    // Final model cleanup
    top->final();

    // Close trace if opened

    if (tfp) { tfp->close(); }

    // Destroy model
    // delete top; top = NULL;
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
    // fixed_seed = 1551685392;
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