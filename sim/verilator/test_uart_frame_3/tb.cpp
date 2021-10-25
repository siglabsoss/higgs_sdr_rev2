#define VERILATE_TESTBENCH

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <assert.h>
#include <verilated.h>
#include <sys/stat.h>
#include <fstream>
#include <math.h>
#include <cstdint>
#include "Vtb_higgs_top.h"
#include "Vtb_higgs_top__Syms.h"
#include "cpp_utils.hpp"
#include <verilated_vcd_c.h>
#include "higgs_helper.hpp"
#include "piston_c_types.h"
#include "vmem_types.h"
#include "schedule.h"
#include "feedback_bus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"

typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
// Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
top_t* top = new top_t; 
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
    // Note does conversion to real, to match SystemC
    return main_time;
}

int main(int argc, char** argv, char** env) {
    unsigned int fixed_seed = 0; // set to non zero to use

    setup_random(fixed_seed);

    STANDARD_TB_START();

    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);

    preReset(top);
    t->reset(40);
    postReset(top);


    t->uarts["cs11"]->print = true;
    // t->uarts["cs11"]->print_chars = true;

    int simulation_time_us = 40;


    

    const std::string base = "counter_";
#ifdef asdf
    typedef std::map<uint8_t, unsigned> dump_map_t;
    t->uarts["cs11"]->registerVmemImemCb([&](const uint8_t tag, const uint32_t addr, const std::vector<uint32_t>& data) {
        
        static dump_map_t mymap;

        auto it = mymap.find(tag);
        if (it == mymap.end()) {
            mymap.insert(std::make_pair(tag,0));
            it = mymap.find(tag);
        }

        unsigned count = it->second;

        // if( count == 0 ) {
            cout << "[0] = " << data[0] << "\n";
            cout << "[1] = " << data[1] << "\n";
        // }

        std::string fname = base + std::to_string(count) + ".hex";
        cout << "fname: " << fname << "\n";


        cout << "repeat: " <<  it->second << "\n";
        it->second++;

       // std::cout << "TAG: " << int(tag) << std::endl;
       // for(auto& it : *data) {
       //     std::cout << std::hex << it << std::endl;
       // }
    });
#endif

    // t->uarts["cs11"]->vmemTagToHexFile(0, "counter_");
    // t->uarts["cs11"]->vmemTagToHexFile(1, "small_");

    unsigned fail = 0;
    unsigned calls_1 = 0;
    constexpr unsigned expected_calls = 5;

    t->uarts["cs11"]->registerVmemImemTagCb([&](const uint32_t addr, const std::vector<uint32_t>& data) {
        
        cout << "                                  callback " << calls_1 << "\n";
        if( calls_1 == 0) {
            if( data.size() != 1 ) {
                cout << "ERROR: size wrong in call " << calls_1 << "\n";
                fail++;
            }
            if( data[0] != 0x0000ffe0 ) {
                cout << "ERROR: value wrong in call " << calls_1 << "\n";
                fail++;
            }
        }

        if( calls_1 == 1) {
            if( data.size() != 1 ) {
                cout << "ERROR: size wrong in call " << calls_1 << "\n";
                fail++;
            }
            if( data[0] != 0xdeadf00d ) {
                fail++;
            }
        }

        if( calls_1 == 2 || calls_1 == 4 || calls_1 == 5 ) {
            if( data.size() != 2 ) {
                cout << "ERROR: size wrong in call " << calls_1 << "\n";
                fail++;
            }
            if( data[0] != 0xffffffff ) {
                cout << "ERROR: value wrong in call " << calls_1 << "\n";
                fail++;
            }
            if( data[1] != 0xeeeeeeee ) {
                cout << "ERROR: value wrong in call " << calls_1 << "\n";
                fail++;
            }
        }


        if( calls_1 == 3) {
            if( data.size() != 2 ) {
                cout << "ERROR: size wrong in call " << calls_1 << "\n";
                fail++;
            }
            if( data[0] != 0xeeeeeeee ) {
                cout << "ERROR: value wrong in call " << calls_1 << "\n";
                fail++;
            }
            if( data[1] != 0x0 ) {  // assumes that vmem is init with zeros
                cout << "ERROR: value wrong in call " << calls_1 << "\n";
                fail++;
            }
        }

        if( calls_1 > expected_calls ) {
            fail++;
        }

        calls_1++;
       
    }, 1);



    unsigned calls_imem = 0;

    t->uarts["cs11"]->registerImemWordsTagCb([&](const uint32_t addr, const std::vector<uint32_t>& data) {
        
        cout << "                            imem  callback " << calls_imem << "\n";

        for( const auto w : data ) {
            cout << HEX32_STRING(w) << "\n";
        }
        if( calls_imem == 0 ) {
            std::vector<uint32_t> ideala = {0xffaaee0d, 0xffaaee1d, 0xffaaee2d, 0xffaaee3d};
            if( data != ideala ) {
                cout << "ERROR: imem value wrong in call " << calls_imem << "\n";
                fail++;
            }
        }


        if( calls_imem == 1 ) {
            if( data[0] != 0xfeed0000 ) {
                cout << "ERROR: imem value wrong in call " << calls_imem << "\n";
                fail++;
            }
        }

        if( calls_imem == 2 ) {
            if( data[0] != 0x12345678 ) {
                cout << "ERROR: imem value wrong in call " << calls_imem << "\n";
                fail++;
            }
        }

        if( calls_imem == 3 ) {
            if( data[0] != 0x12345678 || data[1] != 0xff00ee00 ) {
                   cout << "ERROR: imem value wrong in call " << calls_imem << "\n";
                fail++;
            }
        }

        if( calls_imem == 4 || calls_imem == 5 || calls_imem == 6 || calls_imem == 7 ) {
            if( data[0] != 0x22334455 || data.size() != 1 ) {
                   cout << "ERROR: imem value wrong in call " << calls_imem << "\n";
                fail++;
            }
        }

        cout << "\n\n\n";

        calls_imem++;
       
    }, 2);


    for(unsigned int i = 0; i < simulation_time_us; i++) {
        t->tick(500);
    }

    // std::function<uint32_t(uint32_t)> a = [](uint32_t x){return top->tb_higgs_top->cs11_top->vex_machine_top_inst->q_engine_inst->mem->get_imem(x);};
    // std::function<uint32_t(uint32_t)> a = t->readImemFunction("cs11");

    // if( a == 0 ) {
    //     cout << "was 0\n" << endl;
    //     usleep(10);
    // }

    // for(unsigned i = 0; i < 32; i++ ) {
    //     // auto val = top->tb_higgs_top->cs11_top->vex_machine_top_inst->q_engine_inst->mem->get_imem(i);
    //     auto val = a(i);
    //     cout << HEX32_STRING(val) << "\n";
    // }

    // auto res = t->readImemWords("cs11", 6400, 10);
    // for( const auto w : res ) {
    //     cout << HEX32_STRING(w) << "\n";
    // }



    t->print_ringbus_out();

    top->final();

    if (tfp) { tfp->close(); }

    assert(fail == 0);
    assert(calls_1 == (expected_calls+1) && "did not get correct number of calls");

    delete top; top = NULL;
} 
