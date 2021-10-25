// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2017 by Wilson Snyder.
//======================================================================

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
#define BOOT_SIZE (27)
unsigned int boot_array[BOOT_SIZE] = {0x0040006f, 0x000077b7, 0xc0878793,
                                      0x00007737, 0xc0870713, 0x00f70863,
                                      0x0007a023, 0x00478793, 0xfee79ce3,
                                      0x00000713, 0x00000793, 0x00008137,
                                      0xbfc10113, 0x008000ef, 0x0000006f,
                                      0xff010113, 0x00812623, 0x01010413,
                                      0xdeadc7b7, 0xeef78893, 0x888897b7,
                                      0x88878893, 0x00000793, 0x00078513,
                                      0x00c12403, 0x01010113, 0x00008067};

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





void bootload_fpga(unsigned int fpga, helper_t *t){
    unsigned int boot_size = 27;
    unsigned int packet_data = BOOTLOADER_CMD|BOOT_SIZE;
    unsigned int boot_array[boot_size] = {0x0040006f, 0x000077b7, 0xc0878793,
                                          0x00007737, 0xc0870713, 0x00f70863,
                                          0x0007a023, 0x00478793, 0xfee79ce3,
                                          0x00000713, 0x00000793, 0x00008137,
                                          0xbfc10113, 0x008000ef, 0x0000006f,
                                          0xff010113, 0x00812623, 0x01010413,
                                          0xdeadc7b7, 0xeef78893, 0x888897b7,
                                          0x88878893, 0x00000793, 0x00078513,
                                          0x00c12403, 0x01010113, 0x00008067};
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, packet_data));
    t->tick(500);

    for(unsigned int i = 0; i < boot_size; i++){
        t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, boot_array[i]));
        t->tick(500);    
    }
}

void set_dac_module(helper_t *t, unsigned int data){
    unsigned int packet_data = CONFIG_DAC_CMD|data;
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_ETH, packet_data));
    t->tick(500);
}

void tx_turnstile(top_t *top, helper_t *t, unsigned int samples,
                                           unsigned int base){
    for(unsigned int i = 0; i < samples; i++){
        top->tx_turnstile_data_in = base + i;
        top->tx_turnstile_data_valid = 1;
        t->tick(2);
    }
}

void transmit_data(top_t *top, helper_t *t, unsigned int samples,
                                            unsigned int base){
    unsigned int packet_data;
    
    t->tick(1000);
    packet_data = DMA_IN_CMD|(samples);
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, packet_data));
    
    t->tick(1000);
    tx_turnstile(top, t, samples, base);

    t->tick(1000);
    packet_data = DMA_IN_CMD|(samples);
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS10, packet_data));

    t->tick(1000);
    packet_data = DMA_OUT_CMD|(samples);
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, packet_data));
}

void test_ring(helper_t *t, unsigned int fpga, unsigned int message){
    unsigned int packet_data;
    packet_data = RING_TEST_CMD|message|(8 - fpga);
    t->inStreamAppend("ringbusin", ringbus_udp_packet(fpga, packet_data));
    t->tick(1000);
}


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
    unsigned int packet_data;

    t->tick(5000);
    packet_data = DMA_OUT_PACKET_CMD|(0x5bc);
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, packet_data));

    t->tick(gap*60);

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