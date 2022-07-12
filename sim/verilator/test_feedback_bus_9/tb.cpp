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

    // if( allow_epoc_adjust ) {
    //     cout << "allow_epoc_adjust: " << endl;
    //     int add = target_error / 2;
    //     cout <<  "     will add: " << add << endl;
    //     cout <<  "     epoc_calibration goes from " << epoc_calibration << " to ";
    //     epoc_calibration += add;
    //     cout << epoc_calibration << endl;

    //     // epoc_estimated = adjust_frames_to_schedule(epoc_estimated, target_error);
    // }
}


void test0(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    helper_t* t = new helper_t(top,&main_time,tfp); 



    preReset(top);

    t->reset(40);

    postReset(top);

    // handleFillLevelReply(0x467c0003);

    // tb inputs starts here
    // user can tick the clock for a period
    // append data to input streams, and look at output streams
    // modify negClock() and posClock() above
    // you can also insert for check streams from those functins()

    // delay between sending inputs

    bool disk_mapmov = false;
    bool disk_eq = true; // EQ IS THE PROBLEM, A SHORT PACKET!!!!!!!!
    bool try_repair = true;

    int us = 1500;
    int data_end_us = 1032; // time when mammov is consumed

    if( !disk_mapmov ) {
        us = 700;
        data_end_us = 250;
    }


    uint32_t amount = 5;
    uint32_t direction = 1;
    int custom_size;


    bool found1 = false;
    bool found2 = false;

    // dump is here because we require size 1 due to a bug
    std::vector<uint32_t> dumb;
    dumb.resize(16);
    auto check_awake = feedback_vector_packet(
                FEEDBACK_VEC_STATUS_REPLY,
                dumb,
                FEEDBACK_PEER_8, FEEDBACK_DST_HIGGS);

    auto corrupt_packet = file_read_hex("../../../libs/s-modem/soapy/js/test/data/crash_mapmov_4.hex");
    auto eq_packet = file_read_hex("../../../libs/s-modem/soapy/js/test/data/crash_mapmov_3.hex");

    // cout << HEX32_STRING(corrupt_packet[corrupt_packet.size()-1]) << endl;

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

        if( i == (data_end_us+118) || i == (data_end_us+268) ) {
            cout << "Injecting dump command() at " << i << endl;
            // t->inStreamAppend(
            //     "ringbusin",
            //     ringbus_udp_packet(RING_ADDR_CS20, FB_REPORT_STATUS_CMD)
            // );
            
            t->inStreamAppend(
                "ringbusin",
                ringbus_udp_packet(RING_ADDR_CS20, FB_REPORT_STATUS_CMD)
            );
        }

        // if(i == (data_end_us+208) ) {
        //     t->inStreamAppend("cs20in", check_awake);
        // }

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


        // // hc.RING_ADDR_CS20, ADD_LIFETIME_CMD | run 
        // static const std::vector<uint32_t> advanceEpoc = {
        //     RESET_LIFETIME_CMD,
        //     ADD_LIFETIME_CMD | 8388607,
        //     ADD_LIFETIME_CMD | 8388607,
        //     ADD_LIFETIME_CMD | 8388607,
        //     ADD_LIFETIME_CMD | 8388607,
        //     ADD_LIFETIME_CMD | 8388607,
        //     ADD_LIFETIME_CMD | 8388607,
        //     ADD_LIFETIME_CMD | (3503358+7930),
        // };

        // send the vector above
        // auto f = meteredRingbusSendUni<top_t>(t, advanceEpoc, 92, RING_ADDR_CS20, 10);
        // f(i);
        if( disk_mapmov && i == 86 ) {
            t->inStreamAppend("cs20in", corrupt_packet);
        }


        if( disk_eq ) {
            if( (!disk_mapmov) && i == 90 ) {
                t->inStreamAppend("cs20in", eq_packet);
            }
        } else {
            if( i == 200 || i == 210 ) {
                t->inStreamAppend("cs20in", check_awake);
            }
        }

        if(try_repair) {
            bool repair_reset = false;
            bool repair_zeros = true;
            if( i == (data_end_us + 200) ) {

                if( repair_zeros ) {
                    std::vector<uint32_t> zrs;
                    zrs.resize(400+192+32+32);
                    t->inStreamAppend("cs20in", zrs);

                    t->inStreamAppend("cs20in", check_awake);

                }

            }
        }


        if( (!disk_mapmov) && i == 86 ) {
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


            uint32_t epoc = 0x834;     // seq2
            uint32_t timeslot = 0xc; // seq

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

            // for(int j = 0; j < 20; j++) {
            //     cout << HEX_STRING(packet[j]) << endl;
            // }

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

    bool found3 = false;
    uint32_t schedule_reply;

    cout << "Ringbus got out" << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
        if((*it & 0xff000000)==0x46000000) {
            schedule_reply = *it;
            found3 = true;
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