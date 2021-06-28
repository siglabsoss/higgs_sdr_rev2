#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <bitset>
// Include common routines

#include <assert.h>
#include <verilated.h>

#include <sys/stat.h>  // mkdir

#include <fstream>

// Include model header, generated from Verilating "top.v"
#include "Vmapper_mover.h"
#include "Vmapper_mover__Syms.h"


#include <verilated_vcd_c.h>


#define RESET rst

#include "helper.hpp"


using namespace std;


typedef Vmapper_mover top_t;
typedef Helper<top_t> helper_t;



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
    top->rst = 1;
    top->clk = 0;
}

void postReset() {
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



int main(int argc, char** argv, char** env) {

    cout << "tb.cpp main()" << endl;

    // This is a more complicated example, please also see the simpler examples/hello_world_c.

    // Prevent unused variable warnings
    if (0 && argc && argv && env) {}
    // Pass arguments so Verilated code can see them, e.g. $value$plusargs
    Verilated::commandArgs(argc, argv);

    // Set debug level, 0 is off, 9 is highest presently used
    Verilated::debug(0);

    // Randomization reset policy
    Verilated::randReset(2);

    const char* flag = Verilated::commandArgsPlusMatch("trace");
    if (flag && 0==strcmp(flag, "+trace")) {
	Verilated::traceEverOn(true);  // Verilator must compute traced signals
	cout << "Enabling waves into wave_dump.vcd...\n" << endl;
	tfp = new VerilatedVcdC;
	top->trace(tfp, 99);  // Trace 99 levels of hierarchy
	// mkdir("logs", 0777);
	tfp->open("wave_dump.vcd");  // Open the dump file
    } else {
	cout << "WILL NOT WRITE .vcd WAVE FILE" << endl;
	cout << "  \"make show\" will be stale " << endl << endl;
    }

    // This helper is what I built to make this function easy
    // this handles reset.  You can register an arbitrary number of inputs
    // and outputs.
    // calling things like `inStreamAppend()` allows user to easily specify queue
    // input data which will be ticked over when tick is called
    Helper<top_t>* t = new Helper<top_t>(top,&main_time,tfp);

    // attach handlers
    t->negClock = &negClock;
    t->posClock = &posClock;


    // attach streams
    // here we build a stream object which has poiners into top
    // and then we just pass this into the EthFrameRouterHelper.
    // after we do this, we can append data to these streams at any time, either here
    // or in posClock/negClock above

    // 0th input
    PortIn din;
    din.t_data = &(top->t_data);
    din.t_valid = &(top->t_valid);
    din.t_ready = &(top->t_ready);
    //  din.t_last_byte = &(top->t_ready);
    din.valid_meter = 0;
    din.valid_state = 0;
    din.count_up = 0;
    t->ins.push_back(din);

    // // 1st input

    // // 3
    // Port32Out cs00out;
    // cs00out.i_data = &(top->snap_cs00_riscv_out_data);
    // cs00out.i_valid = &(top->snap_cs00_riscv_out_valid);
    // cs00out.i_ready = &(top->snap_cs00_riscv_out_ready);
    // cs00out.control_ready = 0; // tb does not control ready
    // t->outs.push_back(cs00out);


    vector<uint32_t> p0 = {
	0x02, // 0
	0x1D6, // 1
	0x08, // 2
	0x09, // 3
	0x05, // 4 vector type
	0x0B,  // 5
	0x00, // 6
	0x10, // 7 pkt size
	0x01 //constellation type
	
    };


    unsigned int seed_start = std::time(0);

    unsigned int fixed_seed = 0; // set to non zero to use

    // fixed_seed = 1529688167;

    if(fixed_seed != 0) {
	seed_start = fixed_seed;
	cout << "starting with hard-coded seed " << seed_start << endl;
    } else {
	cout << "starting with random seed " << seed_start << endl;
    }


    srand(seed_start);

    unsigned int pick1, pick2;

    // pick1 = rand() & 0x00ff;
    // pick2 = rand() & 0x00ff;

    // cout << endl;

    // cout << "Pick 1,2:  0x" << HEX_STRING(pick1) << ", 0x" << HEX_STRING(pick2) << endl;

    preReset();

    //top->arp_pkt_fifo_full = 0;

    t->reset(40);

    top->i_ready = 1;
    for(int i = 0; i<32; i++){
	top->mover_active[i] = 0xAAAAAA0A;
    }
    top->trim_start = 4;
    top->trim_end = 4+16;

    postReset();

    for(int i =0; i< 22;i++)
	p0.push_back(i);
  
    // test always starts with this (fixme no random here)
    t->inStreamAppendPacket(0, p0);

  
    const uint32_t send_types = 2;


    unsigned int lifetime_pulls = 1000*10;
    unsigned int pulls = 0;

    for(unsigned int i = 1; i < 1000 ; i++) {

	t->tick(1);
    }


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
