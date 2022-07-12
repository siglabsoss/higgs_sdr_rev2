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



    int us = 9 * 1000 * 1000; // 300 for dumping
    // int data_end_us = 300; // time when mammov is consumed

    // int send_eq_1 = data_end_us + 1 + (rand() % 10);
    // int send_eq_2 = send_eq_1 + 2 + (rand() % 5);

    // us = 300;


    // dump is here because we require size 1 due to a bug
    std::vector<uint32_t> dumb;
    dumb.resize(16);
    auto check_awake = feedback_vector_packet(
                FEEDBACK_VEC_STATUS_REPLY,
                dumb,
                FEEDBACK_PEER_8, FEEDBACK_DST_HIGGS);

    auto corrupt_packet = file_read_hex("../../../libs/s-modem/soapy/js/test/data/crash_mapmov_24_a.hex");


    auto eq_packet = file_read_hex("../../../libs/s-modem/soapy/js/test/data/default_eq_packet.hex");


    auto fb2 = file_read_hex("../../../libs/s-modem/soapy/js/test/data/crash_mapmov_24_a.hex");
    fb2.resize(16);
    for(int i = 0; i < 72; i++) {
        fb2.push_back(0xff000000 + i);
    }
    fb2[1] = 9232;

    auto rbcrashsplit = file_read_hex("../../data/ringbus_loop_crash_1.hex");

    std::vector<uint32_t> rbcrash;

    int i = 0;
    uint32_t upperlower = 0;
    for(auto w : rbcrashsplit ) {
        // cout << HEX_STRING(w) << "\n";
        
        if( i%2 == 0 ) {

            upperlower |= w;
        } else {
            upperlower |= w;
            rbcrash.push_back(upperlower);
            upperlower = 0;
        }

        i++;
    }


    // for(auto w : rbcrash ) {
    //     cout << HEX32_STRING(w) << "\n";
    // }

    // exit(0);

    std::vector<uint32_t> build;

    for(uint32_t i = 0; i < rbcrash.size(); i++ ) {
        uint32_t bad = rbcrash[i];
        build.push_back(bad);

        for(auto j = 0; j < 15; j++) {
            build.push_back(rand()%0xffffffff);
        }

    }







    uint32_t dut_seed = rand() & 0xffffff;

    int next_pull = 86;


    int extra_zeros = 0;// rand() % 133;
    int front_zeros = 0;//rand() % 2;
    int front_zeros_extra = 0;//rand() % 32;

    for(unsigned int i = 0; i < us; i++) {


        // if( i == 84 && front_zeros) {
        //     std::vector<uint32_t> zrs2;
        //     zrs2.resize(16 + front_zeros_extra, 0);
        //     t->inStreamAppend("cs20in", zrs2);
        //     cout << "injecting " << zrs2.size() << " front zeros at " << i << endl;
        // }

        if(i == 84) {
            // cs20 will wait for this seed before going
            t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, EDGE_EDGE_IN | dut_seed ) );
        }

        if( i > 100 && i % 100 == 0) {


            uint32_t seconds = 10;
            uint32_t count = seconds*24; // fundamental property of code, see cs20

            // raw_ringbus_t rb0 = {RING_ADDR_CS20, CS20_PROGRESS_CMD|count};
            // dsp->ringbusPeerLater(attached->peer_id, &rb0, 0);

            t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, CS20_PROGRESS_CMD|count ) );

        }


        if( i == next_pull ) {
            cout << "sent packet at " << next_pull << endl;
            

            // auto fb3 = fb2;
            // corrupt it
            // fb3.resize( fb3.size() - 16 + (rand() % 33) );


            t->inStreamAppend("cs20in", build);
            // t->inStreamAppend("cs20in", eq_packet);

            if( rand()%2 == 0 ) {
                cout << "sent zeros  at " << next_pull << endl;
                std::vector<uint32_t> zrs3;
                zrs3.resize(16 + (rand()%32), 0);
                t->inStreamAppend("cs20in", zrs3);
            }


            next_pull += 800 + (rand() % 200);

            // pad with zeros at tail
            // std::vector<uint32_t> zrs3;
            // zrs3.resize(64, 0);
            // t->inStreamAppend("cs20in", zrs3);
        }

        // if( i == send_eq_1 ) {
        //     t->inStreamAppend("cs20in", eq_packet);
        // }

        // if( i == send_eq_2 ) {
        //     t->inStreamAppend("cs20in", eq_packet);
        // }
        


        t->tick(500);
    }


    int found1 = 0;
    int found2 = 0;
    bool found3 = false;
    int found4 = 0;
    int found5 = 0;
    int found6 = 0;
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
    }


    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        // cout << "0x" << HEX_STRING(*it) << endl;
        uint32_t cmd = (*it & 0xff000000);
        switch(cmd) {
            case 0x44000000:
            case 0x46000000:
            case 0xfe000000:
            case CS20_PROGRESS_REPORT_PCCMD:
            case CS20_EPOC_REPORT_PCCMD:
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

    bool pass = found5 == 0 && found6 == 0;

    cout << "Found " << found4 << " check alive replies" << endl;

    if( pass ) {
        cout << "!!!!!!!!!!!!!!!! ALL PASSED !!!!!!!!!!!!!!!!" << endl;
    } else {
        cout << "!!!!!!!!!!!!!!!! ALL FAILED !!!!!!!!!!!!!!!!" << endl;
    }



    t->allStreamDump();
    cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
    file_dump_T<cs20_node_t>(cs20_node,"cs20_vmem.out");

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