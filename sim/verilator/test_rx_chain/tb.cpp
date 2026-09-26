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

void tx_turnstile(top_t *top, helper_t *t, unsigned int samples,
                                           unsigned int base){
    for(unsigned int i = 0; i < samples; i++){
        top->tx_turnstile_data_in = base + i;
        top->tx_turnstile_data_valid = 1;
        t->tick(2);
    }
}

void rx_from_adc(top_t *top, helper_t *t, unsigned int samples,
                                           unsigned int base){
    for(unsigned int i = 0; i < samples; i++){
        top->i_data_adc = base + i;
        t->tick(1);    
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

void enable_rx_data(top_t *top, helper_t *t, unsigned int samples,
                                             unsigned int base){
    unsigned int packet_data;

    t->tick(1000);
    packet_data = DMA_IN_CMD|(samples);
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, packet_data));

    t->tick(2039);
    rx_from_adc(top, t, samples, base);
}

void receive_data(top_t *top, helper_t *t, unsigned int samples,
                                           unsigned int base){
    unsigned int packet_data;

    enable_rx_data(top, t, samples, base);

    t->tick(3000);
    packet_data = DMA_IN_CMD|(samples);
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS01, packet_data));

    t->tick(1000);
    packet_data = DMA_OUT_CMD|(samples);
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, packet_data));

    t->tick(2000);
    packet_data = PASS_DATA_CMD|(samples);
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, packet_data));
}

void receive_data_chunk(top_t *top, helper_t *t, unsigned int samples){
    unsigned int packet_data;

    packet_data = DMA_OUT_PACKET_CMD|(samples);
    t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, packet_data));
}   


int main(int argc, char** argv, char** env) {
    int gap = 1000;

    STANDARD_TB_START();

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

    // tb inputs starts here
    // user can tick the clock for a period
    // append data to input streams, and look at output streams
    // modify negClock() and posClock() above
    // you can also insert for check streams from those functins()

    // boot the processors
    t->tick(1000);
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