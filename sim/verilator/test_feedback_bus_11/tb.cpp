#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <assert.h>
#include <verilated.h>
#include <sys/stat.h>
#include <fstream>
#include <math.h>
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"
#include "cpp_utils.hpp"
#include "feedback_bus_tb.hpp"
#include <verilated_vcd_c.h>
#include "higgs_helper.hpp"
#include "piston_c_types.h"
#include "vmem_types.h"
#include "schedule.h"
#include "feedback_bus.h"
#include "feedback_bus_types.h"

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

int32_t epoc_recent_reply_frames;

void applyFillLevelToMember(uint32_t word) {
    uint32_t dmode = (word & 0x00ff0000) >> 16;
    int32_t frame_delta = word & 0x0000ffff;

    // Do this to sign extend
    frame_delta <<= 16;
    frame_delta >>= 16;

    int epoc_delta = (int8_t)dmode;

    // Unwinds some values, I think this is just to save compute in CS11
    while(0 < epoc_delta) {
        frame_delta += SCHEDULE_FRAMES;
        epoc_delta--;
    }

    // Copy to class
    epoc_recent_reply_frames = frame_delta;
}

// See handleEpocReply
void handleFillLevelReply(uint32_t word) {
    // Updates epoc_recent_reply_frames
    applyFillLevelToMember(word);

    std::cout << "Epoc Delta represented as frames "
              << epoc_recent_reply_frames << endl;

    int target = 512*3;

    if( abs(epoc_recent_reply_frames) > (SCHEDULE_FRAMES*2) ) {
        std::cout << "Epoc Delta is WAY out of estimate\n";
    }

    int target_error = target - epoc_recent_reply_frames;

    // We want this to be negative. I think if this is too large, we go too late
    // actually if that's true, 0 really is ideal.
    std::cout << "   target_error " << target_error << " (large is late)\n";
    
    if( abs(target_error) < 100 ) { // was 200
        cout << "   Skipping doesn't need update\n";
        return;
    }
}

void test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    helper_t* t = new helper_t(top,&main_time,tfp); 

    preReset(top);

    t->reset(40);

    postReset(top);

    bool try_repair = true;
    // 300 for dumping
    int us = 400;
    // Time when mammov is consumed
    int data_end_us = 90;


    // Dump is here because we require size 1 due to a bug
    std::vector<uint32_t> dumb;
    dumb.resize(16);
    auto check_awake = feedback_vector_packet(
                FEEDBACK_VEC_STATUS_REPLY,
                dumb,
                FEEDBACK_PEER_8, FEEDBACK_DST_HIGGS);

    const uint32_t junk_small_length = 4 + (rand() % 16);
    const uint32_t junk_vs_type = 2;

        const std::vector<uint32_t> junk_small = {0x00000002,
                                            junk_small_length,
                                            0x00000004,
                                            0x80000000,
                                            junk_vs_type,
                                            0x00000000,
                                            0x00000000,
                                            0x00000000,
                                            0x00000000,
                                            0x00000000,
                                            0x00000000,
                                            0x00000000,
                                            0x00000000,
                                            0x00000000,
                                            0x00000000,
                                            0x00000000};


    const int trim = 0;
    const int extra_zeros = rand() % 133;
    const int front_zeros = rand() % 2;
    const int front_zeros_extra = rand() % 32;

    for(unsigned int i = 0; i < us; i++) {

        if( false ) {
            // Time needs to be about 300 for all these to flush out
            static const std::vector<uint32_t> reportRb = {
                CS11_REPORT_ERRORS_CMD,
                FB_REPORT_STATUS_CMD
            };
            auto injectReport = meteredRingbusSendUni<top_t>(t,
                                                             reportRb,
                                                             (data_end_us+30),
                                                             RING_ADDR_CS11);
            injectReport(i);
        }


        if( i == 84 && front_zeros) {
            std::vector<uint32_t> zrs2;
            zrs2.resize(16 + front_zeros_extra, 0);
            t->inStreamAppend("cs11in", zrs2);
            cout << "Injecting " << zrs2.size() << " front zeros at " << i
                 << "\n";
        }


        if( i == 85 ) {
            // Corrupt ?
            t->inStreamAppend("cs11in", junk_small);
            cout << "Injecting junk_small with len: " << junk_small_length << "\n";

            // Pad with zeros at tail
            std::vector<uint32_t> zrs3;
            zrs3.resize(64, 0);
            t->inStreamAppend("cs11in", zrs3);
            cout << "Injecting zeros: " << zrs3.size() << "\n";
        }
        
        if( i == 88 ) {
             t->inStreamAppend("cs11in", check_awake);
             t->inStreamAppend("cs11in", check_awake);
             t->inStreamAppend("cs11in", check_awake);
        }


        t->tick(500);
    }


    int found1 = 0;
    int found2 = 0;
    bool found3 = false;
    int found4 = 0;
    uint32_t schedule_reply;

    cout << "Ringbus got out" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin();
        it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
        uint32_t cmd = (*it & 0xff000000);
        if((*it & 0xff000000)==0x46000000) {
            schedule_reply = *it;
            found3 = true;
        }
        if( *it == 0xdead0202 ) {
            found1++;
        }
        if( *it == 0xdead0207 ) {
            found2++;
        }
        if( cmd == FEEDBACK_ALIVE ) {
            found4++;
        }
        

    }

    if(found3) {
        handleFillLevelReply(schedule_reply);
    }

    for(auto it = t->outs["ringbusout"]->data.begin();
        it != t->outs["ringbusout"]->data.end(); it++) {
        uint32_t cmd = (*it & 0xff000000);

        switch(cmd) {
            case CS11_USERDATA_ERROR: {
                auto msg = getErrorStringFeedbackBusParse(*it);
                cout << msg;
            }
            break;
            default:
                break;
        }
    }

    const bool pass = found4 > 1;

    std::cout << "Found " << found4 << " check alive replies\n";

    if( pass ) {
        std::cout << "All Tests Passed\n";
    } else {
        std::cout << "All Tests Failed\n";
    }


    t->allStreamDump();
    cs11_node_t* cs11_node = top->
                             tb_higgs_top->
                             cs11_top->
                             vex_machine_top_inst->
                             q_engine_inst->piston_inst->UNODE_NAME;
    file_dump_T<cs11_node_t>(cs11_node,"cs11_vmem.out");

    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) {tfp->close();}

    assert(pass);

    // Destroy model
    delete top; top = NULL;

    exit(0);
}









int main(int argc, char** argv, char** env) {
    uint32_t test_select = 0;

    if(const char* env_p = std::getenv("TEST_SELECT")) {
        unsigned int env_test_select = atoi(env_p);
        cout << "Environment variable TEST_SELECT was set to: "
             << env_test_select << endl;
        test_select = env_test_select;
    }

    // Set to non zero to use
    unsigned int fixed_seed = 0;
    // fixed_seed = 1550997712;
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