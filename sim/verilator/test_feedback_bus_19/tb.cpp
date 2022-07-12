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


    int us = 500;


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


    uint32_t go_ts   = 0x0000000c; //[5]
    uint32_t go_epoc = 0x0000304d; //[6]


    auto fb2 = file_read_hex("../../../libs/s-modem/soapy/js/test/data/crash_mapmov_24_a.hex");
    fb2.resize(16); // just the header

    // see higgs_sdr_rev2/libs/s-modem/soapy/js/examples/den2.js
    uint32_t input_size = 10000;
    uint32_t output_size_128 = 0;
    uint32_t output_size_320 = 256016;

    for(int i = 0; i < input_size; i++) {
        uint32_t val;
        val = 0xff000000 + i; // counter
        val = rand() & 0xffffffff; // random
        fb2.push_back(val);
    }
    fb2[1] = output_size_128;

    // const int header = 16;
    // const double enabled_subcarriers = 128;
    // const double bits_per_subcarrier = 4;
    // auto custom_size = header + ceil((fb2.size()*32.0/bits_per_subcarrier)/enabled_subcarriers)*1024;

    fb2[7] = input_size;
    fb2[8] = FEEDBACK_MAPMOV_QAM16;


/*
    auto fb3 = file_read_hex("../../../libs/s-modem/soapy/js/test/data/packet_30.hex");
    fb3.resize(16+2000);
    fb3[1] = output_size;
    fb3[7] = input_size;
    fb3[8] = FEEDBACK_MAPMOV_QAM16;

    fb3[5] = fb2[5]; // take schedule epoc and timeslot
    fb3[6] = fb2[6];
*/  

    bool replace_data_of_packet = true;


    auto fb4 = file_read_hex("../../../libs/s-modem/soapy/js/test/data/packet_320_qam16_2.hex");
    fb4.resize(16+input_size);

    if( replace_data_of_packet ) {
        for(int i = 0; i < input_size; i++) {
            uint32_t val;
            val = rand() & 0xffffffff; // random
            fb4[i+16] = val;
        }
    }


    fb4[1] = output_size_320;
    fb4[7] = input_size;
    // fb4[8] = FEEDBACK_MAPMOV_QAM16; // should already be set

    fb4[5] = go_ts; // take schedule epoc and timeslot
    fb4[6] = go_epoc;

    // for( auto w : fb4 ) {
    //     cout << HEX32_STRING(w) << '\n';
    // }


    std::string test;
    // test = "120";
    test = "320";
    // test = ""; // no injection

    std::string send_eq;

    // send_eq = ""; // none
    // send_eq = "mod"; // see loop and trim_low above
    send_eq = "default"; // default eq with pilots for 1 radio







    bool update_bs = true;
    int send_bs_at = 135; // ok to send this while eth is updating mapmov


    // uint32_t bs[5] = {0xf, 0xf, 0xf, 0xf, 0xf};  // original
    uint32_t bs[5] = {0x0f, 0x10, 0x0f, 0x0f, 0x0f}; // original way I got it to work
    // uint32_t bs[5] = {0x10, 0x10, 0xe, 0x10, 0xe};
    // uint32_t bs[5] = {0x10, 0x0f, 0xf, 0xf, 0xf};
    // uint32_t bs[5] = {0x0f, 0x13, 0x0f, 0x0f, 0x0f};


    // uint32_t dut_seed = rand() & 0xffffff;

    int next_pull = 200;

    next_pull += rand() % 25; // random start a bit


    //////////////////////////// sfo
    int set_sfo = 120 + rand()%3000;

    cout << "will apply sfo at " << set_sfo << endl;


    int r = rand() % 1000000;
    double f =  (double)r / 1000000.0 * (0.128258);

    if(rand()%2 == 0 ) {
        f = -f;
    }
    

    double estimate = f;
    
    uint32_t sfo_amount = 25600.0 / abs(estimate);
    uint32_t sfo_direction  = (estimate>0)?2:1;


    //////////////////////////// cfo

    int set_cfo = 300;

    double cfo_estimated = 4.4;

    bool sign = (cfo_estimated<0.0);
    uint32_t cfo_estimated_abs = (uint32_t)abs(cfo_estimated*131.072);

    uint32_t cfo_lower = (cfo_estimated_abs&0xffff);
    uint32_t cfo_upper = ((cfo_estimated_abs>>16)&0xffff);

    if(sign)
    {
      cfo_upper |= 0x010000;
    }

    // raw_ringbus_t rb0 = {2, TX_CFO_LOWER_CMD | cfo_lower};
    // raw_ringbus_t rb1 = {2, TX_CFO_UPPER_CMD | cfo_upper};









    int gap;
    gap = 46; // error
    // gap = 47; // ok

    if( rand() % 2 == 0 ) {
        gap = 47;
    }

    for(unsigned int i = 0; i < us; i++) {

        if( update_bs ) {
            static const std::vector<uint32_t> bsRb = {
                 FFT_BARREL_SHIFT_CMD | 0x00000  | bs[0]
                ,FFT_BARREL_SHIFT_CMD | 0x10000  | bs[1]
                ,FFT_BARREL_SHIFT_CMD | 0x20000  | bs[2]
                ,FFT_BARREL_SHIFT_CMD | 0x30000  | bs[3]
                ,FFT_BARREL_SHIFT_CMD | 0x40000  | bs[4] // cs10 will write_out data when this index is sent
            };
            // 4 may be too short for a list of rb longer than ~4
            auto updateBs = meteredRingbusSendUni<top_t>(t, bsRb, send_bs_at, RING_ADDR_CS10, 10);
            updateBs(i);
        }

        if( i == 63 && test == "320" ) { 
            t->inStreamAppend("ringbusin",
                ringbus_udp_packet(RING_ADDR_ETH, MAPMOV_MODE_CMD | MAPMOV_MODE_320_CENTERED )
            );
        }

        if( i == set_sfo ) {

            cout << "applying sfo estimate of " << estimate << " at time " << i << endl;
            cout << "   " << HEX32_STRING(sfo_amount) << endl;
            cout << "   " << HEX32_STRING(sfo_direction) << endl;

            t->inStreamAppend("ringbusin",
                ringbus_udp_packet(RING_ADDR_CS10, SFO_PERIODIC_ADJ_CMD | sfo_amount )
            );
            t->inStreamAppend("ringbusin",
                ringbus_udp_packet(RING_ADDR_CS10, SFO_PERIODIC_ADJ_CMD | sfo_direction )
            );
        }

        if( i == set_cfo ) {
            cout << "applying cfo estimate of " << cfo_estimated << " at time " << i << endl;
            
            t->inStreamAppend("ringbusin",
                ringbus_udp_packet(RING_ADDR_CS10, TX_CFO_LOWER_CMD | cfo_lower )
            );
            t->inStreamAppend("ringbusin",
                ringbus_udp_packet(RING_ADDR_CS10, TX_CFO_UPPER_CMD | cfo_upper )
            );
        }

        if( i == next_pull ) {
            cout << "sent packet at " << next_pull << " " << test << endl;
            // t->inStreamAppend("cs20in", fb2);
            // t->inStreamAppend("cs20in", fb2);

            if( test == "120") {
                t->inStreamAppend("cs20in", fb2);
            }
            if( test == "320") {
                t->inStreamAppend("cs20in", fb4);
            }
        }


        if( (send_eq == "mod" || send_eq == "default") && i == next_pull-30 ) {
            if( send_eq == "mod" ) {
                t->inStreamAppend("cs20in", mod_eq_packet);
            }
            if( send_eq == "default" ) {
                t->inStreamAppend("cs20in", default_eq_packet);
            }
        }

        t->tick(500);
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

    // bool pass = found7 == 0 && found1 == 1;

    // cout << "Found " << found4 << " check alive replies" << endl;

    // if( pass ) {
    //     cout << "!!!!!!!!!!!!!!!! ALL PASSED !!!!!!!!!!!!!!!!" << endl;
    // } else {
    //     cout << "!!!!!!!!!!!!!!!! ALL FAILED !!!!!!!!!!!!!!!!" << endl;
    // }



    t->allStreamDump();
    cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
    file_dump_T<cs20_node_t>(cs20_node,"cs20_vmem.out");

    // Final model cleanup
    top->final();

    // Close trace if opened

    if (tfp) { tfp->close(); }

    // assert(pass);

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
