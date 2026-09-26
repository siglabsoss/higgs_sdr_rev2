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
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"

#include "cpp_utils.hpp"

// also gets feedback_bus.h from riscv
#include "feedback_bus_tb.hpp"



#include <verilated_vcd_c.h>


#define RESET MIB_MASTER_RESET

#include "higgs_helper.hpp"



using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

#include "vmem_types.h"
#include "convert.hpp"




VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
  return main_time; // Note does conversion to real, to match SystemC
}


#include "test_1.cpp"

int main(int argc, char** argv, char** env) {
    uint32_t test_select = 1;

    if(const char* env_p = std::getenv("TEST_SELECT")) {
        unsigned int env_test_select = atoi(env_p);
        cout << "environment variable TEST_SELECT was set to: " << env_test_select << endl;
        test_select = env_test_select;
    }


    unsigned int fixed_seed = 0; // set to non zero to use
    setup_random(fixed_seed);

    switch(test_select) {
        case 1:
            test1(argc, argv, env);
            break;
        default:
            cout << "Invalid test selected" << endl;
            exit(1);
            break;
    }
}
