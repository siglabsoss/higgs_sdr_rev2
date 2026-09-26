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
#include "mapmov.h"
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

void applyFillLevelToMember(uint32_t word)
{
    uint32_t dmode =      (word & 0x00ff0000) >> 16;
    int32_t frame_delta =  word & 0x0000ffff;

    // do this to sign extend
    frame_delta <<= 16;
    frame_delta >>= 16;

    int epoc_delta = (int8_t)dmode;

    // unwinds some values, I think this is just to save compute
    // in cs20
    while(0 < epoc_delta) {
        frame_delta += SCHEDULE_FRAMES;
        epoc_delta--;
    }

    // copy to class
    epoc_recent_reply_frames = frame_delta;
    // epoc_recent_reply_frames_valid = true;
}

// see handleEpocReply
void handleFillLevelReply(uint32_t word) {

    applyFillLevelToMember(word); // updates epoc_recent_reply_frames

    cout << "epoc_delta represented as frames " << epoc_recent_reply_frames << endl;

    int target = 512*3; // this could be put somewhere else

    if( abs(epoc_recent_reply_frames) > (SCHEDULE_FRAMES*2) ) {
        cout << "epoc delta is WAY out of estimate" << endl;
    }

    // int target_error = epoc_recent_reply_frames - target;
    int target_error = target - epoc_recent_reply_frames;

    // we want this to be negative. I think if this is too large, we go too late
    // actually if that's true, 0 really is ideal.
    cout << "   target_error " << target_error << " (large is late)" << endl;
    
    // map_mov_acks_received++;

    if( abs(target_error) < 100 ) { // was 200
        cout << "   Skipping doesn't need update" << endl;
        return;
    }

}


void test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    helper_t* t = new helper_t(top,&main_time,tfp); 


    preReset(top);

    t->reset(40);

    postReset(top);

    t->ins["cs20in"]->valid_meter = 4;


    int us = 350*500;


    // dump is here because we require size 1 due to a bug
    std::vector<uint32_t> dumb;
    dumb.resize(16);
    auto check_awake = feedback_vector_packet(
                FEEDBACK_VEC_STATUS_REPLY,
                dumb,
                FEEDBACK_PEER_8, FEEDBACK_DST_HIGGS);

    // auto corrupt_packet = file_read_hex("../../../libs/s-modem/soapy/js/test/data/crash_mapmov_24_a.hex");


    auto eq_packet = file_read_hex("../../../libs/s-modem/soapy/js/test/data/random_rotation_eq_packet.hex");
    auto default_eq_packet = file_read_hex("../../../libs/s-modem/soapy/js/test/data/default_eq_packet.hex");


    auto counter_eq_packet = default_eq_packet;

    for(int i = 16; i < 1024+16; i++) {
        int sc = i-16;
        counter_eq_packet[i] = 0xff000000 + sc;
    }


    // copy from default eq
    auto mod_eq_packet = default_eq_packet;

    int trim_low = 288;
    trim_low = 256;
    trim_low = 400;
    const int trim_high = 1024-trim_low;

    for(int i = 16; i < 1024+16; i++) {
        int sc = i-16;
        if( sc > trim_low && sc < trim_high ) {
            mod_eq_packet[i] = 0;
        }

        // if( sc < 4 || sc > 1024 ) {
        //     mod_eq_packet[i] = 0;
        // }
    }




    std::string test;
    // test = "120";
    test = "320";
    // test = ""; // no injection

    std::string send_eq;

    // send_eq = ""; // none
    // send_eq = "mod"; // see loop and trim_low above
    // send_eq = "default"; // default eq with pilots for 1 radio
    send_eq = "counter"; // counter eq



    int next_pull = 130*500;



    int eq_sent = 0;

    int pulls = 2 + rand()%5;

    for(unsigned int i = 0; i < us; i++) {


        if( (send_eq == "mod" || send_eq == "default" || send_eq == "counter")
            && (i == (next_pull) )
                                                          ) {
            if( send_eq == "mod" ) {
                t->inStreamAppend("cs20in", mod_eq_packet);
            }
            if( send_eq == "default" ) {
                t->inStreamAppend("cs20in", default_eq_packet);
            }
            if( send_eq == "counter" ) {
                t->inStreamAppend("cs20in", counter_eq_packet);
            }
            eq_sent++;

            cout << "sent " << send_eq << " packet at " << i/500 << " (" << i << ")" << endl;

            if( eq_sent < pulls ) {
                next_pull = i + 20 + (rand()%(500*30));
            }


        }

        t->tick(1);
    }


    int found1 = 0;
    int found2 = 0;
    bool found3 = false;
    int found4 = 0;
    int found5 = 0;
    int found6 = 0;
    int found7 = 0;
    uint32_t schedule_reply;

    cout << "Ringbus got out" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
        uint32_t cmd = (*it & 0xff000000);
        uint32_t cmd2 = (*it & 0xff0000ff);
        if((*it & 0xff000000)==CS20_FILL_LEVEL_PCCMD) {
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
        if( cmd2 == 0x44000015) {
            found5++;
        }

        if( cmd2 == 0x44000010) {
            found7++;
        }
    }


    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        // cout << "0x" << HEX_STRING(*it) << endl;
        uint32_t cmd = (*it & 0xff000000);
        switch(cmd) {
            case 0x44000000:
            case 0x46000000:
            case 0xfe000000:
                break;
            default:
                found6++;
        }
    }



    if(found3) {
        handleFillLevelReply(schedule_reply);
    }

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


    

    // auto &ideal = file_read_hex("./cs20_in_ideal.hex");
    auto &ideal = t->monitors["mmapmovin"]->data;

    auto &got = t->monitors["mcs20in"]->data;


    bool pass = (found1 == eq_sent) && (got == ideal) ;

    // for( auto w :  ) {
    //     cout << HEX32_STRING(w) << endl;
    // }

    // cout << "Found " << found4 << " check alive replies" << endl;

    if( pass ) {
        cout << "!!!!!!!!!!!!!!!! ALL PASSED !!!!!!!!!!!!!!!!" << endl;
    } else {
        cout << "!!!!!!!!!!!!!!!! ALL FAILED !!!!!!!!!!!!!!!!" << endl;
    }



    bool dump = false;

    if( dump ) {
        t->allStreamDump();
        cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
        file_dump_T<cs20_node_t>(cs20_node,"cs20_vmem.out");
    }

    // usleep(5000);


    // Final model cleanup
    top->final();

    // Close trace if opened

    if (tfp) { tfp->close(); }

    assert(pass);

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
    // fixed_seed = 1550997712;
    // fixed_seed = 1554249069;
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