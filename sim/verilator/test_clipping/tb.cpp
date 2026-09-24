#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
// Include common routines

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"






// // If "verilator --trace" is used, include the tracing class
# include <verilated_vcd_c.h>
// #include "dbus.hpp"



#define RESET MIB_MASTER_RESET

#include "higgs_helper.hpp"


using namespace std;
typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;



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


void rx_from_adc(top_t *top, helper_t *t, unsigned int samples,
                                          unsigned int base,
                                          unsigned int overflow){
    if(overflow){
        for(unsigned int i = 0; i < samples; i++){
            top->i_data_adc = 0x7fff7fff;
            t->tick(1);    
        }
    }
    else{
        for(unsigned int i = 0; i < samples; i++){
            top->i_data_adc = base + i;
            t->tick(1);    
        }   
    }
}

void enable_rx_data(top_t *top, helper_t *t, unsigned int samples,
                                             unsigned int base,
                                             unsigned int overflow){
    unsigned int packet_data;

    t->tick(1000);
    packet_data = DMA_IN_CMD|(samples);
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, packet_data));

    t->tick(2039);
    rx_from_adc(top, t, samples, base, overflow);
}

void saturation_ratio(top_t *top, helper_t *t, unsigned int gain){
    unsigned int packet_data;
    t->tick(1000);
    packet_data = SATURATION_RATIO_CMD|(gain);
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, packet_data));
}

// ring_unit_counters_t dump_print(helper_t *t, uint32_t enumm) {
//     int gap = 1000;
//     uint32_t report_enum;
//     uint32_t report_error;
//     ring_unit_counters_t report_counters;
//     uint32_t addr = ring_unit_lookup_addr(enumm);

//     // feed command in and wait
//     t->inStreamAppend("ringbusin", ringbus_udp_packet(addr, ring_t_enc(4,0,0,0) ) );
//     t->tick(gap*9);

//     cout << "Dump " << get_enum_string(enumm) << endl;
//     // for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
//     //   cout << "    0x" << HEX_STRING(*it) << endl;
//     // }

//     // convert response to an object we can look at
//     report_error = vector_to_ring_unit_counters(t->outs["ringbusout"]->data, &report_enum, &report_counters);

//     assert(report_error == 0);

//     print_ring_unit_counters_t(report_enum, report_counters);

//     cout << endl << endl;

//     // assert(report_enum == enumm);

//     t->outs["ringbusout"]->data.resize(0); // erase what we printed

//     return report_counters;
// }



int main(int argc, char** argv, char** env) {
    int gap = 1000;

    STANDARD_TB_START()

    // This helper is what I built to make this function easy
    // this handles reset.  You can register an arbitrary number of inputs
    // and outputs.
    // calling things like `inStreamAppend()` allows user to easily specify queue
    // input data which will be ticked over when tick is called
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

    // boot the processors
    t->tick(1000);
    rx_from_adc(top, t, 0x400, 0x0, 0x1);
    rx_from_adc(top, t, 0x400, 0x0, 0x0);
    saturation_ratio(top, t, 0x00000000|0x0003c0);
    saturation_ratio(top, t, 0x00000000|0x000300);
    t->tick(30000);
    saturation_ratio(top, t, 0x00000000|0x000140);
    t->tick(gap*80);

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