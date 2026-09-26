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
#include "Veth_frame_router.h"
#include "Veth_frame_router__Syms.h"


#include <verilated_vcd_c.h>


#define RESET i_rxmac_srst

#include "eth_frame_router_helper.hpp"


using namespace std;


typedef Veth_frame_router top_t;
typedef EthFrameRouterHelper<top_t> helper_t;



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
  top->i_rxmac_clk = 0;
}

void postReset() {
  // top->i_ringbus = 1;
  // top->i_data_eth = 0;
  // top->i_data_adc = 0;
  // top->i_o_ready_eth = 1;
  // top->i_o_ready_dac = 1;
  // top->i_data_valid_eth = 1;
  // top->i_data_valid_adc = 1;
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
  EthFrameRouterHelper<top_t>* t = new EthFrameRouterHelper<top_t>(top,&main_time,tfp);

  // attach handlers
  t->negClock = &negClock;
  t->posClock = &posClock;


  // attach streams
  // here we build a stream object which has poiners into top
  // and then we just pass this into the EthFrameRouterHelper.
  // after we do this, we can append data to these streams at any time, either here
  // or in posClock/negClock above

  // 0th input
  Port8In ethmacin;
  ethmacin.t_data = &(top->i_rx_dbout);
  ethmacin.t_valid = &(top->i_rx_write);
  ethmacin.t_last_byte = &(top->i_rx_eof);
  ethmacin.valid_meter = 0;
  ethmacin.valid_state = 0;
  ethmacin.count_up = 0;
  t->ins.push_back(ethmacin);

  // // 1st input
  // Port32In ringbusin;
  // ringbusin.t_data = &(top->ringbus_in_data);
  // ringbusin.t_valid = &(top->ringbus_in_data_vld);
  // ringbusin.valid_meter = 0;
  // t->ins.push_back(ringbusin);

  // // 2nd input (ADC DATA)
  // Port32In cs00in;
  // cs00in.t_data = &(top->adc_data_out);
  // cs00in.t_valid = &(top->adc_data_out_valid);
  // cs00in.valid_meter = 4;
  // cs20in.valid_state = 0;
  // t->ins.push_back(cs00in);

  // // 0th output
  // Port32Out ringbusout;
  // ringbusout.i_data = &(top->ringbus_out_data);
  // ringbusout.i_valid = &(top->ringbus_out_data_vld);
  // ringbusout.i_ready = &(top->ring_bus_i0_ready);
  // ringbusout.control_ready = 1;
  // t->outs.push_back(ringbusout);

  // // 1nd output (data output from cs20, bound for 10)
  // Port32Out cs20out;
  // cs20out.i_data = &(top->snap_cs20_riscv_out_data);
  // cs20out.i_valid = &(top->snap_cs20_riscv_out_valid);
  // cs20out.i_ready = &(top->snap_cs20_riscv_out_ready);
  // cs20out.control_ready = 0; // tb does not control ready
  // t->outs.push_back(cs20out);

  // // 2st output (data bound for dac)
  // Port32Out cs10out;
  // cs10out.i_data = &(top->snap_cs10_riscv_out_data);
  // cs10out.i_valid = &(top->snap_cs10_riscv_out_valid);
  // cs10out.i_ready = &(top->snap_cs10_riscv_out_ready);
  // cs10out.control_ready = 0; // tb does not control ready
  // t->outs.push_back(cs10out);

  // // 3
  // Port32Out cs00out;
  // cs00out.i_data = &(top->snap_cs00_riscv_out_data);
  // cs00out.i_valid = &(top->snap_cs00_riscv_out_valid);
  // cs00out.i_ready = &(top->snap_cs00_riscv_out_ready);
  // cs00out.control_ready = 0; // tb does not control ready
  // t->outs.push_back(cs00out);

  // // 4
  // Port32Out cs01out;
  // cs01out.i_data = &(top->snap_cs01_riscv_out_data);
  // cs01out.i_valid = &(top->snap_cs01_riscv_out_valid);
  // cs01out.i_ready = &(top->snap_cs01_riscv_out_ready);
  // cs01out.control_ready = 0; // tb does not control ready
  // t->outs.push_back(cs01out);

  // // 5
  // Port32Out cs11out;
  // cs11out.i_data = &(top->snap_cs11_riscv_out_data);
  // cs11out.i_valid = &(top->snap_cs11_riscv_out_valid);
  // cs11out.i_ready = &(top->snap_cs11_riscv_out_ready);
  // cs11out.control_ready = 0; // tb does not control ready
  // t->outs.push_back(cs11out);

  // // 6
  // Port32Out cs21out;
  // cs21out.i_data = &(top->snap_cs21_riscv_out_data);
  // cs21out.i_valid = &(top->snap_cs21_riscv_out_valid);
  // cs21out.i_ready = &(top->snap_cs21_riscv_out_ready);
  // cs21out.control_ready = 0; // tb does not control ready
  // t->outs.push_back(cs21out);

  // // 7
  // Port32Out cs31out;
  // cs31out.i_data = &(top->snap_cs31_riscv_out_data);
  // cs31out.i_valid = &(top->snap_cs31_riscv_out_valid);
  // cs31out.i_ready = &(top->snap_cs31_riscv_out_ready);
  // cs31out.control_ready = 0; // tb does not control ready
  // t->outs.push_back(cs31out);

  // // 8
  // Port32Out cs30out;
  // cs30out.i_data = &(top->snap_cs30_riscv_out_data);
  // cs30out.i_valid = &(top->snap_cs30_riscv_out_valid);
  // cs30out.i_ready = &(top->snap_cs30_riscv_out_ready);
  // cs30out.control_ready = 0; // tb does not control ready
  // t->outs.push_back(cs30out);

        //   output wire o_data_valid_dac,
        // output wire i_o_ready_dac,    // an output because we are controlling this from verilog not the tb
        // output wire [31:0] o_data_dac,


// auto vec = file_read_hex("prn_input.hex");
//   

  vector<uint8_t> p0 = {
    0x06,
    0x07,
    0x08,
    0x09,
    0x0a,
    0x0b,
    0x00,
    0x05,
    0x1b,
    0xd0,
    0x0a,
    0x8a,
    0x0b,
    0x00,
    0x45,
    0x00,
    0x00,
    0x24,
    0x89,
    0x33,
    0x40,
    0x00,
    0x40,
    0x11,
    0x99,
    0x8b,
    0x0a,
    0x02,
    0x02,
    0x01,
    0x0a,
    0x02,
    0x02,
    0x02,
    0xa9,
    0x72,
    0x4e,
    0x20,
    0x00,
    0x10,
    0xf0,
    0x34,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
  };

  // arp?
  vector<uint8_t> p1 = {
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0x00,
    0x05,
    0x1B,
    0xD0,
    0x0A,
    0x8A,
    0x08,
    0x06,
    0x00,
    0x01,
    0x08,
    0x00,
    0x06,
    0x04,
    0x00,
    0x01,
    0x00,
    0x05,
    0x1B,
    0xD0,
    0x0A,
    0x8A,
    0x0A,
    0x02,
    0x02,
    0x01,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0A,
    0x02,
    0x02,
    0x02,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0
  };
  
  vector<uint8_t> p2 = {
    0x6,
    0x7,
    0x8,
    0x9,
    0xa,
    0xb,
    0x0,
    0x5,
    0x1b,
    0xD0,
    0x0a,
    0x8a,
    0x08,
    0x0,
    0x45,
    0x0,
    0x0,
    0x24,
    0x43,
    0x3d,
    0x40,
    0x00,
    0x40,
    0x11,
    0xdf,
    0x85,
    0x0a,
    0x02,
    0x02,
    0x01,
    0x0a,
    0x2,
    0x2,
    0x2,
    0xdd,
    0x0e,
    0x4e,
    0x20,
    0x00,
    0x10,
    0xd5,
    0xea,
    0x1,
    0x0,
    0x0,
    0x0,
    0x7,
    0xad,
    0xde,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0


  };


  // t->inStreamAppendPacket(0, p0);

//   exit(0);


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

    top->arp_pkt_fifo_full = 0;
    top->arp_pkt_fifo_afull = 0;
    top->ipv4_pkt_fifo_full = 0;
    top->ipv4_pkt_fifo_afull = 0;

  t->reset(40);

  postReset();


  // test always starts with this (fixme no random here)
  t->inStreamAppendPacket(0, p1);

  // with 256 enabled subcarriers, 16 input samples generates 256 words which are loaded into one 1024 frame
  // therefor 8 us (4000 clock cyles) is the correct delay to send data (This will overflow for very long test runs)
  unsigned int adv = 8;

  // int us = 5; // estimate for 64*3*1280 to go through  1962

  // us = 100;

  // int per_cmd = 140;

  // these are actually double ns.
  // so i = 50, really means 100 ns
  unsigned ns_base = 130;

  unsigned int pull_gap_maximum_us = 130;

  unsigned int execute_time = 120 - 60;

  unsigned int next_pull = ns_base + rand() % pull_gap_maximum_us;

  const uint32_t send_types = 2;


  unsigned int lifetime_pulls = 1000*10;
  unsigned int pulls = 0;

  for(unsigned int i = 1; /*empty*/ ; i++) {

    if(top->o_eth_frame_circ_buf_overflow |
       top->o_ipv4_pkt_fifo_overflow|
        top->o_arp_pkt_fifo_overflow ) {
        cout << "HANG CONDITION AT " << i << "(sent " << pulls << ")" << endl;
        // FIXME put this in for random testing
        // assert(false);
    }

    if(top->o_unsupported_eth_type_error) {
        cout << "Invalid Packet AT " << i << endl;
        assert(false);
    }

    if ( i < 700*1000){
      if( i % 1000 == 0 ) {
          top->arp_pkt_fifo_afull = 1;
          top->ipv4_pkt_fifo_afull = 1;
      }

      if( i % 1300 == 0 ) {
          top->arp_pkt_fifo_afull = 0;
          top->ipv4_pkt_fifo_afull = 0;
      }      
    } else {
          top->arp_pkt_fifo_afull = 0;
          top->ipv4_pkt_fifo_afull = 0;     
    }


    if( i == next_pull && pulls == (lifetime_pulls+1) ) {
        cout << "Did last pull " << (pulls-1) << " at time " << i << endl;
        break;
    }

    if( i == next_pull ) {
        uint32_t which = rand() % send_types;

        switch(which) {
            case 0:
                t->inStreamAppendPacket(0, p1);
                break;
            case 1:
                t->inStreamAppendPacket(0, p2);
                break;
            default:
                assert(false && "switch broken");
                break;
        }
        next_pull += execute_time + (rand() % pull_gap_maximum_us);

        pulls++;
    }


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
