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
#include <verilated_vcd_c.h>
// #include "dbus.hpp"

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))


#define RESET MIB_MASTER_RESET

#include "higgs_helper.hpp"


using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

// typedef Vtb_higgs_top_q_engine__pi3 q_engine_t;



VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top = new top_t;// Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
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
    // top->i_ringbusout = 1;
    // top->i_data_eth = 0;
    top->i_data_adc = 0;
    // top->i_o_ready_eth = 1;
    // top->i_o_ready_dac = 1;
    // top->i_data_valid_eth = 1;
    top->i_data_valid_adc = 1;
    cout << "after main time: " << main_time << endl;
}


int main(int argc, char** argv, char** env) {

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);


    srand(1);

    preReset(top);

    t->reset(40);

    postReset(top);

    unsigned int us = 1000;
    // t->tick(500*300);

    // loop for 300 us or until cs20 is done
    for(unsigned int i = 0; i < us; i++) {
        t->tick(500); // tick 1 us
        unsigned int cs20_pc = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();

        if(t->pc_exicted_main("cs20")) {
            cout << "Breaking at " << main_time << "(" << i << " us) PC was " << cs20_pc << endl;
            break;
        }
    }

    // flush ringbusout
    t->tick(500*6); // set to 500*100 if you are dumping many ringbus with ring_block_send_eth()

    assert( t->outs["ringbusout"]->data.size() != 0 && "didn't get any output");

    cout << "Ring got out " << t->outs["ringbusout"]->data.size() << " items." << endl;
    for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
        cout << "" << HEX_STRING(*it) << endl;
    }


    const uint32_t expect = 0x000000ff;

    assert(
        t->outs["ringbusout"]->data[t->outs["ringbusout"]->data.size()-1] == expect
        && "Some tests did not pass"
        );

    cout << "All Tests Passed" << endl;

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

