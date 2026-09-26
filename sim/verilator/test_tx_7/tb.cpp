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
#include "GenericOperator.hpp"
// #include "feedback_bus_tb.hpp"

using namespace siglabs::rb;


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


void test0(int argc, char** argv, char** env) {
}








void test1(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



    preReset(top);

    t->reset(40);

    postReset(top);

    // tb inputs starts here
    // user can tick the clock for a period
    // append data to input streams, and look at output streams
    // modify negClock() and posClock() above
    // you can also insert for check streams from those functins()

    // delay between sending inputs

    int us = 300;

    uint32_t amount = 5;
    uint32_t direction = 1;
    int custom_size;

    // std::vector<uint32_t> vin = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

    bool found1 = false;

    // dump is here because we require size 1 due to a bug
    std::vector<uint32_t> dumb;
    dumb.resize(1);
    auto check_awake = feedback_vector_packet(
                FEEDBACK_VEC_STATUS_REPLY,
                dumb,
                FEEDBACK_PEER_8, FEEDBACK_DST_HIGGS);

    std::vector<uint32_t> was_crashing0 = {
0x00000002,
0x00000042,
0x00000004,
0x80000000,
0x00000003,
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
0x00000000
    };

std::vector<uint32_t> was_crashing1 = {
0x00000002,
0x00000042,
0x00000008,
0x80000000,
0x00000003,
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
0x00000000,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff
    };


    auto check_initial_settings = 70;

    auto go_1 = 60;

    bool start_with_check = true;

    bool mm_found_0, mm_found_1, mm_found_2;
    mm_found_0 = mm_found_1 = mm_found_2 = false;
    
    uint32_t mm_enabled, mm_start, mm_end;
    mm_enabled = mm_start = mm_end = 0;

    for(unsigned int i = 0; i < us; i++) {

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
                    found1 = true; // look to see if fbbus is alive
                }
            }
        }

        if(start_with_check &&  i == go_1 ) {
            t->inStreamAppend("cs20in",check_awake);
        }

        if(start_with_check &&  i == (go_1+28) ) {
            cout << "at firt time " << (go_1+28) << endl;
            for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
                cout << "0x" << HEX_STRING(*it) << endl;
                if( *it == FEEDBACK_ALIVE ) {
                    found1 = true; // look to see if fbbus is alive
                }
            }
        }

        if( i == 75 ) {
            t->inStreamAppend("cs20in", was_crashing0);
        }

        if( i == 110 ) {
            t->inStreamAppend("cs20in", was_crashing1);
        }

        // if( i == 84 ) {
        //     std::vector<uint32_t> data;

        //     // cout << "data: " << endl;
        //     // for(auto it = data.begin(); it != data.end(); it++) {
        //     //     cout << "0x" << HEX_STRING(*it) << endl;
        //     // }
        //     // cout << endl << endl;;

        //     // data.resize(8);

        //     dataSync(&data, 0xcafebabe, 16);
        //     dataSync(&data, 0xdeadbeef, 16);
        //     dataSync(&data, 0xdeadbeef, 16);
        //     dataSync(&data, 0xdeadbeef, 16);

        //     // cout << "data: " << endl;
        //     // for(auto it = data.begin(); it != data.end(); it++) {
        //     //     cout << "0x" << HEX_STRING(*it) << endl;
        //     // }
        //     // cout << endl << endl;;


        //     //dataSync(&data, 0xabcddead, 16);

        //     // feedback_vector_packet_mapmov

        //     cout << "About to send " << data.size() << " words " << endl;
        //     cout << "or " << data.size()*32 << " bits" << endl;
        //     cout << "or " << data.size()*16 << " subcarriers worth " << endl;

        //     const int header = 16;
        //     const double enabled_subcarriers = 128;

        //     uint32_t epoc = 1;     // seq2
        //     uint32_t timeslot = 0x11; // seq

        //     // assumes that 0 and 1023 are the boundaries
        //     custom_size = header + ceil((data.size()*16)/enabled_subcarriers)*1024;
        //     auto packet = 
        //         feedback_vector_mapmov_scheduled(
        //             FEEDBACK_VEC_TX_USER_DATA, 
        //             data, 
        //             custom_size, 
        //             FEEDBACK_PEER_8, 
        //             FEEDBACK_DST_HIGGS,
        //             timeslot,
        //             epoc
        //             );

        //     t->inStreamAppend("cs20in", packet);
        // }

        t->tick(500);
    }

    if(!found1) {
        cout << "found1 was NOT set, something is wrong" << endl;
    } else {
        cout << "found1 was set" << endl;
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







void test2(int argc, char** argv, char** env) {

}





void test3(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



    preReset(top);

    t->reset(40);

    postReset(top);

    // tb inputs starts here
    // user can tick the clock for a period
    // append data to input streams, and look at output streams
    // modify negClock() and posClock() above
    // you can also insert for check streams from those functins()

    // delay between sending inputs

    int us = 120;

    uint32_t amount = 5;
    uint32_t direction = 1;
    int custom_size;

    // std::vector<uint32_t> vin = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

    bool found1 = false;

    // dump is here because we require size 1 due to a bug
    std::vector<uint32_t> dumb;
    dumb.resize(1);
    auto check_awake = feedback_vector_packet(
                FEEDBACK_VEC_STATUS_REPLY,
                dumb,
                FEEDBACK_PEER_8, FEEDBACK_DST_HIGGS);

    auto check_initial_settings = 90;

    auto go_1 = 60;

    bool start_with_check = true;

    bool mm_found_0, mm_found_1, mm_found_2;
    mm_found_0 = mm_found_1 = mm_found_2 = false;
    
    uint32_t mm_enabled, mm_start, mm_end;
    mm_enabled = mm_start = mm_end = 0;

    for(unsigned int i = 0; i < us; i++) {

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
                    found1 = true; // look to see if fbbus is alive
                }
            }
        }

        if(start_with_check &&  i == go_1 ) {
            t->inStreamAppend("cs20in",check_awake);
        }

        if(start_with_check &&  i == (go_1+28) ) {
            cout << "at firt time " << (go_1+28) << endl;
            for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
                cout << "0x" << HEX_STRING(*it) << endl;
                if( *it == FEEDBACK_ALIVE ) {
                    found1 = true; // look to see if fbbus is alive
                }
            }
        }

        if( i == 70 ) {
            t->inStreamAppend(
                "ringbusin",
                ringbus_udp_packet(RING_ADDR_ETH, REQUEST_MAPMOV_REPORT)
            );
        }

        if( i == 84 ) {
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

        t->tick(500);
    }

    if(!found1) {
        cout << "found1 was NOT set, something is wrong" << endl;
    } else {
        cout << "found1 was set" << endl;
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




void test4(int argc, char** argv, char** env) {

}


void test5(int argc, char** argv, char** env) {

}


void test6(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



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
    int inject_2nd_ok_check = 295; // can be 0 to 16 probably

    auto inject_mapmov_at = 84;

    auto inject_corruption = 270; // 165

    int adjust_offset = -10; // negative numbers means the packet will be sent later

    bool start_with_check = true;

    bool inject_recover_zeros = 275;

    bool do_recover = true;
    bool do_corrupt = false;

    bool mm_found_0, mm_found_1, mm_found_2;
    mm_found_0 = mm_found_1 = mm_found_2 = false;
    
    uint32_t mm_enabled, mm_start, mm_end;
    mm_enabled = mm_start = mm_end = 0;

    for(unsigned int i = 0; i < us; i++) {


        t->tick(500);
    }


    t->allStreamDump();

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



void test7(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



    preReset(top);

    t->reset(40);

    postReset(top);

    // tb inputs starts here
    // user can tick the clock for a period
    // append data to input streams, and look at output streams
    // modify negClock() and posClock() above
    // you can also insert for check streams from those functins()

    // delay between sending inputs

    int us = 450;

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
    int inject_2nd_ok_check = 295; // can be 0 to 16 probably

    auto inject_mapmov_at = 84;

    auto inject_corruption = 270; // 165

    int adjust_offset = -10; // negative numbers means the packet will be sent later

    bool start_with_check = true;

    bool inject_recover_zeros = 275;

    bool do_recover = true;
    bool do_corrupt = false;

    // int enabled_subcarriers = 128;

    bool mm_found_0, mm_found_1, mm_found_2;
    mm_found_0 = mm_found_1 = mm_found_2 = false;
    
    uint32_t mm_enabled, mm_start, mm_end;
    mm_enabled = mm_start = mm_end = 0;

    for(unsigned int i = 0; i < us; i++) {

        if(i == go_1 ) {
            std::vector<uint32_t> data;

            // cout << "data: " << endl;
            // for(auto it = data.begin(); it != data.end(); it++) {
            //     cout << "0x" << HEX_STRING(*it) << endl;
            // }
            // cout << endl << endl;;

            // data.resize(8);

            const double enabled_subcarriers = 128;


            new_subcarrier_data_sync2(&data, 0xbeefbabe, enabled_subcarriers);
            new_subcarrier_data_sync2(&data, 32, enabled_subcarriers);


            uint32_t val;

            std::vector<uint32_t> mydata;
            for(int i = 0; i < 32; i++) {
                data.push_back(i);
            }

            // // dataSync(&data, 0xbeefbabe, 16);
            // dataSync(&data, 0xdeadbeef, 16);
            // dataSync(&data, 0xdeadbeef, 16);
            // dataSync(&data, 0xdeadbeef, 16);
            // dataSync(&data, 0xdeadbeef, 16);
            // dataSync(&data, 0xdeadbeef, 16);
            // dataSync(&data, 0xdeadbeef, 16);
            // dataSync(&data, 0xdeadbeef, 16);
            // dataSync(&data, 0xdeadbeef, 16);

            // cout << "data: " << endl;
            // for(auto it = data.begin(); it != data.end(); it++) {
            //     cout << "0x" << HEX_STRING(*it) << endl;
            // }
            // cout << endl << endl;;


            //dataSync(&data, 0xabcddead, 16);

            // feedback_vector_packet_mapmov

            const int header = 16;

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


        t->tick(500);
    }

    // if(!found1) {
    //     cout << "found1 was NOT set, something is wrong" << endl;
    // } else {
    //     cout << "found1 was set" << endl;
    // }

    // for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    //     if( *it == FEEDBACK_ALIVE ) {
    //         found2 = true; // look to see if fbbus is alive
    //     }
    // }

    // if(!found2) {
    //     cout << "found2 was NOT set, something is wrong" << endl;
    // } else {
    //     cout << "found2 was set" << endl;
    // }

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

    assert(found1);
    assert(found2);

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



void test8(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 



    preReset(top);

    t->reset(40);

    postReset(top);


    // tb inputs starts here
    // user can tick the clock for a period
    // append data to input streams, and look at output streams
    // modify negClock() and posClock() above
    // you can also insert for check streams from those functins()

    // delay between sending inputs

    int us = 270;

    std::vector<uint32_t> allRb = {
        // ETH_GET_STATUS_CMD
    };

    std::vector<uint32_t> pack;

    // pack = op("get", 0, 0, GENERIC_OPERATOR_CMD);
    // allRb.push_back(pack[0]);
    // allRb.push_back(pack[1]);


    pack = op("get", 15, 0, GENERIC_OPERATOR_CMD);
    allRb.push_back(pack[0]);
    allRb.push_back(pack[1]);

    pack = op("set", 15, 0xdeadcccc, GENERIC_OPERATOR_CMD);
    allRb.push_back(pack[0]);
    allRb.push_back(pack[1]);

    // pack = op("get", 1, 0, GENERIC_OPERATOR_CMD);
    // allRb.push_back(pack[0]);
    // allRb.push_back(pack[1]);


    pack = op("get", 15, 0, GENERIC_OPERATOR_CMD);
    allRb.push_back(pack[0]);
    allRb.push_back(pack[1]);


    pack = op("sub", 15, 0xcccc, GENERIC_OPERATOR_CMD);
    allRb.push_back(pack[0]);
    allRb.push_back(pack[1]);

    pack = op("get", 15, 0, GENERIC_OPERATOR_CMD);
    allRb.push_back(pack[0]);
    allRb.push_back(pack[1]);

    pack = op("add", 15, 0xf00f, GENERIC_OPERATOR_CMD);
    allRb.push_back(pack[0]);
    allRb.push_back(pack[1]);

    pack = op("get", 15, 0, GENERIC_OPERATOR_CMD);
    allRb.push_back(pack[0]);
    allRb.push_back(pack[1]);


    cout << "\n\n";
    for(auto w : allRb) {
        cout << HEX_STRING(w) << "\n";
    }



    auto injector = meteredRingbusSendUni<top_t>(t, allRb, 90, RING_ADDR_CS20, 11);
    // injectReport(i);

    

    for(unsigned int i = 0; i < us; i++) {

        injector(i);

        // t->getRingbusDropped(RING_ENUM_CS20);

        t->tick(500);
    }

    cout << "Ringbus got out" << endl;
    for(const auto w : t->outs["ringbusout"]->data ) {
        cout << "0x" << HEX_STRING(w) << endl;
    }

    cout << "\n\n";


    DispatchGeneric dispatch;

    int calls = 0;

    dispatch.setCallback([&calls](const uint32_t _sel, const uint32_t _val) {
        cout << "Call: " << calls << " Variable index #" << _sel << " has value " << HEX32_STRING(_val) << "\n";

        assert(_sel == 15);

        switch(calls) {
            case 0:
                assert(_val == 0x42);
                break;
            case 1:
                assert(_val == 0xdeadcccc);
                break;
            case 2:
                assert(_val == 0xdead0000);
                break;
            case 3:
                assert(_val == 0xdeadf00f);
                break;
        }

        calls++;
    }, GENERIC_READBACK_PCCMD);




    cout << "Generic results:" << endl;
    for(const auto w : t->outs["ringbusout"]->data ) {
        dispatch.gotWord(w);
    }


    t->allStreamDump();
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
        case 1:
            test1(argc, argv, env);
            break;
        case 2:
            test2(argc, argv, env);
            break;
        case 3:
            test3(argc, argv, env);
            break;
        case 4:
            test4(argc, argv, env);
            break;
        case 5:
            test5(argc, argv, env);
            break;
        case 6:
            test6(argc, argv, env);
            break;
        case 7:
            test7(argc, argv, env);
            break;
        case 8:
            test8(argc, argv, env);
            break;
        default:
            cout << "Invalid test selected" << endl;
            exit(1);
            break;
    }

}