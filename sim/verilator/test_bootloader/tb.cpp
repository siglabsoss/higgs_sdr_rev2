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



VerilatedVcdC* tfp = NULL;
// Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
top_t* top = new top_t; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper
// Current simulation time (64-bit unsigned)
uint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp () {
  return main_time; // Note does conversion to real, to match SystemC
}




// See higgs_helper.hpp

void bootload_fpga(helper_t *t, unsigned int ttl, unsigned int *data, unsigned int size) {
  t->inStreamAppend("ringbusin", ringbus_udp_packet(ttl, BOOTLOADER_CMD|size ) );
  t->tick((12+30+30+95+6)*500);

  // unsigned int delay;

  for(int i = 0; i < size; i++) {
    t->inStreamAppend("ringbusin", ringbus_udp_packet(ttl, data[i] ) );

    t->tick(9*500);
  }

}




int main(int argc, char** argv, char** env) {

  STANDARD_TB_START();

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

unsigned int cs20mem[] =
{
0x0240006f,
0x00000000,
0x00000000,
0x00000000,
0x00000000,
0x00000000,
0x00000000,
0x00000000,
0x30200073,
0x000067b7,
0x40878793,
0x00006737,
0x54c70713,
0x02f70463,
0x0007a023,
0x00478793,
0xfee79ce3
};

const unsigned int cs20sz = ARRAY_SIZE(cs20mem);



    unsigned got_asm = 0;

    unsigned correct = 0;


    uint32_t asmState;
    
    t->registerRb([&](const uint32_t word) {
        const uint32_t dmode = (word & 0x00ff0000)>>16;
        const uint64_t data =  word & 0x0000ffff;

        switch(dmode) {
            case 0:
                asmState = data;
                break;
            case 1:
                asmState |= (data << 16);
                cout << " Asm: " << HEX32_STRING(asmState) << "\n";

                if( got_asm == 0 && asmState == 0xdeadbeef) {
                    correct++;
                }
                if( got_asm == 1 && asmState == 0xfeedbabe) {
                    correct++;
                }

                got_asm++;
                break;
            default:
                cout << "UNKNOWN rb callback\n";
                break;
        }
    }, EDGE_EDGE_OUT);



    unsigned us = 60;


    for(int j = 0; j < 2; j++) {

        if( j == 0 ) {
            cs20mem[7] = 0xdeadbeef;
        }

        if( j == 1 ) {
            cs20mem[7] = 0xfeedbabe;
        }

        // boot the processors
        t->tick(10*500);


        // bootload_fpga(t, 0, recent_eth, 1024); // erases 0x7e04
        bootload_fpga(t, RING_ADDR_CS20, cs20mem, cs20sz); // seems ok?
        // bootload_fpga(t, 0, debug_counter, 8);

        t->tick(10*500);
    
        for( int i = 0; i < us; i++) {

            // if( i == 5 ) {
            //     t->send_ring(RING_ADDR_CS20, EDGE_EDGE_IN | 0x01);
            // }

            if( i == 10 ) {
                t->send_ring(RING_ADDR_CS20, EDGE_EDGE_OUT | (32-4) );
            }

            t->tick(500);
        }
    }


    t->print_ringbus_out();
    t->allStreamDump();


    // Final model cleanup
    top->final();

    // Close trace if opened

    if (tfp) { tfp->close(); }

    // Destroy model
    delete top; top = NULL;


    assert(correct == 2);

    cout << "All Tests passed\n";

    //print_vector(output_vector);
    // Fin
    exit(0);
}
