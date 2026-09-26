#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <iomanip>

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"



// 



// // If "verilator --trace" is used, include the tracing class
#include <verilated_vcd_c.h>
// #include "dbus.hpp"

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))


#define RESET MIB_MASTER_RESET

#include "higgs_helper.hpp"

#include "Vtb_higgs_top_tb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"
#define NSLICES (16)


using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

#include "vmem_types.h"
#include "piston_c_types.h"
// #include "Vtb_higgs_top_vmem_dat_6_5__pi14.h"

// typedef Vtb_higgs_top_q_engine__pi3 q_engine_t;

// eth
// typedef Vtb_higgs_top_vmem_dat_6_5__pi8 eth_node_t;

VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
    return main_time; // Note does conversion to real, to match SystemC
}

void preReset() {
    // Initialize inputs
    top->RESET = 1;
    top->clk = 0;
}

void postReset() {
    // top->i_ringbus = 1;
    // top->i_data_eth = 0;
    top->i_data_adc = 0;
    // top->i_o_ready_eth = 1;
    // top->i_o_ready_dac = 1;
    // top->i_data_valid_eth = 1;
    top->i_data_valid_adc = 1;
    cout << "after main time: " << main_time << endl;
}


void negClock(helper_t *t) {
    // you must call this, or else data will not stream in/out
    t->handleDataNeg();
    // cout << "time: " << main_time << endl;
}

void posClock(helper_t *t) {
    // you must call this, or else data will not stream in/out
    t->handleDataPos();

}

std::vector<uint32_t> get_counter(int start, int stop) {
    std::vector<uint32_t> out;
    for(int i = start; i < stop; i++) {
        out.push_back(i);
    }

    if(0) {
        cout << "get_counter( " << start << ", " << stop << ")" << endl;
        for(auto it = out.begin(); it < out.end(); it++) {
            cout << *it << endl;
        }
    }
    return out;
}



void enable_power_estimation(helper_t *t, unsigned int data){
    unsigned int packet_data = POWER_ESTIMATION_CMD|data;
    t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS00, packet_data));
    t->tick(500);
}

void rx_from_adc(top_t *top, helper_t *t, unsigned int samples,
                                          unsigned int base){
    for(unsigned int i = 0; i < samples; i++){
        top->i_data_adc = base + i;
        t->tick(1);    
    }
}

void test_agc(helper_t *t, unsigned int data){
    unsigned int packet_data = AGC_TEST_CMD|data;
    t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS00, packet_data));
    t->tick(500);
}

int main(int argc, char** argv, char** env) {

    STANDARD_TB_START()

    // This helper is what I built to make this function easy
    // this handles reset.  You can register an arbitrary number of inputs
    // and outputs.
    // calling things like `inStreamAppend()` allows user to easily specify queue
    // input data which will be ticked over when tick is called
    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);



    // attach streams
    // here we build a stream object which has poiners into top
    // and then we just pass this into the HiggsHelper.
    // after we do this, we can append data to these streams at any time, either here
    // or in posClock/negClock above

    // 0th input
    // Port32In cs20in;
    // cs20in.t_data = &(top->tx_turnstile_data_in);
    // cs20in.t_valid = &(top->tx_turnstile_data_valid);
    // t->ins.push_back(cs20in);

    // // 1st input
    // Port32In ringbusin;
    // ringbusin.t_data = &(top->ringbus_in_data);
    // ringbusin.t_valid = &(top->ringbus_in_data_vld);
    // t->ins.push_back(ringbusin);

    // // 0th output
    // Port32Out ringbusout;
    // ringbusout.i_data = &(top->ringbus_out_data);
    // ringbusout.i_valid = &(top->ringbus_out_data_vld);
    // ringbusout.i_ready = &(top->ring_bus_i0_ready);
    // ringbusout.control_ready = 1;
    // t->outs.push_back(ringbusout);


    srand(1);

    preReset(top);

    t->reset(40);


    postReset(top);

    unsigned int pc_done[3] = {0xE4, 0xE8, 0xEC};

    int us;
    us = 20000;
    // us = 5000;

    for(unsigned int i = 0; i < us; i++) {
        t->tick(500); // tick 1 us

        unsigned int cs30_pc = top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
        if(
            cs30_pc == pc_done[0] ||
            cs30_pc == pc_done[1] ||
            cs30_pc == pc_done[2]
            ) {
            cout << "exiting early at us " << i << endl;
            break;
        }
    }

    // enable_power_estimation(t, 0x3c0);
    test_agc(t, 0x0);
    t->tick(1000*60); // flush


    cs30_node_t* cs30_node = top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;

    file_dump_T<cs30_node_t>(cs30_node, "cs30_vmem.out");
    cout << "Ring got out " << t->outs["ringbusout"]->data.size() << " items." << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
    }

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
