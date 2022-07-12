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

#define GARBAGE_ADDR       (4094*NSLICES)
#define SCRATCH_ADDR       (4095*NSLICES)

#include "higgs_helper.hpp"



using namespace std;


typedef Vtb_higgs_top top_t;
typedef HiggsHelper<top_t> helper_t;

#include "piston_c_types.h"
#include "vmem_types.h"




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




std::vector<uint32_t> data_udp_packet() {
  std::vector<uint32_t> out;
  out.push_back(0xdeadbeef);
  out.push_back(0x12345678);
  out.push_back(0x0000ffff);
  return out;
}

std::vector<uint32_t> file_udp_packet(string filename) {
  std::vector<uint32_t> out;
  uint32_t rtn;    
  cout << "opening " << filename << endl;
  std::ifstream inFile(filename, ios::binary|ios::in);
  // out.push_back(0xdeadbeef);
  // out.push_back(0x12345678);
  // out.push_back(0x0000ffff);
  streampos pos;

  if(inFile.is_open()){
    while(!inFile.eof()){
      inFile.read(reinterpret_cast<char *>(&rtn), sizeof(rtn));
      // cout << rtn << endl;
      out.push_back(rtn);
    }
    inFile.close();
  } else {
      cout << "Error opening input file" << endl ;
  }


  return out;
}


uint32_t reverse_bits(uint32_t input)
{    
    uint32_t output = 0;

    for (uint32_t mask = 1; mask > 0; mask <<= 1)
    {
        output <<= 1;

        if (input & mask)
            output |= 1;
    }

    return output;
}




std::vector<uint32_t> swizzle(uint32_t a, uint32_t b) {
  // cout << "a " << HEX_STRING(a) << " b " << HEX_STRING(b) << endl;
  std::vector<uint32_t> out = {0,0};

  uint32_t aa = 0;
  uint32_t bb = 0;
  uint32_t b0 = 0;
  uint32_t b1 = 0;
  for(unsigned int i = 0; i < 32; i++) {
    if( i < 16 ) {
      b0 = a & (1<<(2*i));
      b1 = a & (1<<((2*i)+1));
    } else {
      b0 = b & (1<<(2*(i-16)));
      b1 = b & (1<<(2*(i-16)+1));
    }

    b0 = (b0)?1:0;
    b1 = (b1)?1:0;

    aa |= b0 << i;
    bb |= b1 << i;
  }

  // cout << HEX_STRING(aa) << endl << HEX_STRING(bb) << endl;

  out[0] = aa;
  out[1] = bb;

  return out;
}

std::vector<uint32_t> inverse_swizzle(uint32_t a, uint32_t b) {
  std::vector<uint32_t> out = {0,0};

  uint32_t aa = 0;
  uint32_t bb = 0;
  uint32_t b0 = 0;
  uint32_t b1 = 0;
  for(unsigned int i = 0; i < 32; i++) {
    b0 = a & (1<<(i));
    b1 = b & (1<<(i));
    b0 = (b0)?1:0;
    b1 = (b1)?1:0;

    if( i < 16 ) {
      aa |= b0 << (i*2)  |  b1 << ((i*2)+1);
    } else {
      bb |= b0 << ((i-16)*2)  |  b1 << (((i-16)*2)+1);
    }

  }

  // cout << HEX_STRING(aa) << endl << HEX_STRING(bb) << endl;

  out[0] = aa;
  out[1] = bb;

  return out;
}


std::vector<uint32_t> swizzle_reverse_all(std::vector<uint32_t> ins) {
  std::vector<uint32_t> out;
  std::vector<uint32_t> tmp = {0,0};

  bool do_swizzle = true;
  bool inverse = true; // is swizzle inverse?

  for(unsigned int i = 0; i < ins.size()-1; i+=2) {

    if( do_swizzle ) {
      if( inverse ) {
        tmp = inverse_swizzle(
          ins[i],
          ins[i+1]
          );
      } else {
        tmp = swizzle(
          ins[i],
          ins[i+1]
          );
      }
    } else {
      tmp[0] = ins[i];
      tmp[1] = ins[i+1];
    }


    VEC_R_APPEND2(out, tmp);
  }
  return out;
}

void p_c(uint32_t a, uint32_t b) {
  std::cout << "a , at" << endl;
  std::cout << "b , bt" << endl;
  
  
  uint32_t at,bt;

  std::vector<uint32_t> out = swizzle(a,b);

  at = out[0];
  bt = out[1];


  cout << std::bitset<32>(a) << ", " << std::bitset<32>(at) << std::endl;
  cout << std::bitset<32>(b) << ", " << std::bitset<32>(bt) << std::endl;
}

void p_ci(uint32_t a, uint32_t b) {
  std::cout << "a , at" << endl;
  std::cout << "b , bt" << endl;
  
  
  uint32_t at,bt;

  std::vector<uint32_t> out = inverse_swizzle(a,b);

  at = out[0];
  bt = out[1];


  cout << std::bitset<32>(a) << ", " << std::bitset<32>(at) << std::endl;
  cout << std::bitset<32>(b) << ", " << std::bitset<32>(bt) << std::endl;
}

// void consider_output(std::vector<uint32_t> samples, std::vector<uint32_t> subcarriers) {
//   t->outs[2].data
// }

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
  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);

  // attach handlers
  t->negClock = &negClock;
  t->posClock = &posClock;


  // attach streams
  // here we build a stream object which has poiners into top
  // and then we just pass this into the HiggsHelper.
  // after we do this, we can append data to these streams at any time, either here
  // or in posClock/negClock above

  // 0th input
  Port32In cs20in;
  cs20in.t_data = &(top->tx_turnstile_data_in);
  cs20in.t_valid = &(top->tx_turnstile_data_valid);
  cs20in.valid_meter = 0;
  cs20in.valid_state = 0;
  t->ins.push_back(cs20in);

  // 1st input
  Port32In ringbusin;
  ringbusin.t_data = &(top->ringbus_in_data);
  ringbusin.t_valid = &(top->ringbus_in_data_vld);
  ringbusin.valid_meter = 0;
  t->ins.push_back(ringbusin);

  // 0th output
  Port32Out ringbusout;
  ringbusout.i_data = &(top->ringbus_out_data);
  ringbusout.i_valid = &(top->ringbus_out_data_vld);
  ringbusout.i_ready = &(top->ring_bus_i0_ready);
  ringbusout.control_ready = 1;
  t->outs.push_back(ringbusout);

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

  // 8
  Port32Out cs30out;
  cs30out.i_data = &(top->snap_cs30_riscv_out_data);
  cs30out.i_valid = &(top->snap_cs30_riscv_out_valid);
  cs30out.i_ready = &(top->snap_cs30_riscv_out_ready);
  cs30out.control_ready = 0; // tb does not control ready
  t->outs.push_back(cs30out);

        //   output wire o_data_valid_dac,
        // output wire i_o_ready_dac,    // an output because we are controlling this from verilog not the tb
        // output wire [31:0] o_data_dac,



  unsigned int seed_start = std::time(0);

  unsigned int fixed_seed = 0; // set to non zero to use

  // fixed_seed = 1528613966;  // first seed value after rng was working to give issue
  fixed_seed = 1528640076;

  if(fixed_seed != 0) {
    seed_start = fixed_seed;
    cout << "starting with hard-coded seed " << seed_start << endl;
  } else {
    cout << "starting with random seed " << seed_start << endl;
  }

  srand(seed_start);



  preReset();

  t->reset(40);

  postReset();

  // tb inputs starts here
  // user can tick the clock for a period
  // append data to input streams, and look at output streams
  // modify negClock() and posClock() above
  // you can also insert for check streams from those functins()


  int us = 120;

  // how many us to wait before first input
  unsigned us_base = 10;


  // seed that is fed to dut
  uint32_t dut_seed = rand() & 0xffffff;

  for(unsigned int i = 0; i < us; i++) {

    if(i == 2) {
        // cs30 will wait for this seed before going
        t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS30, EDGE_EDGE_IN | dut_seed ) );
    }


    t->tick(500);

    // unsigned int cs20pc = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();

    // if(t->pc_exicted_main(cs20pc)) {
    //   cout << "Exiting early due to main() returning us(" << i << ") " << main_time  << endl;
    //   break;
    // }
  }


    int jamming = -1;

    int error_count = 0;

    for(uint32_t i = 0; i < t->outs[1].data.size(); /*empty*/ ) {
        uint32_t word = t->outs[1].data[i];

        // cout << "I: " << i;

        // cout << "  (" << HEX32_STRING(word) <<  ")" <<endl;

        feedback_frame_vector_t *v;

        // use uint32_t pointer arithmatic to and then do final cast before asignign
        v = (feedback_frame_vector_t*) (((uint32_t*)t->outs[1].data.data())+i);

        uint32_t* vs = (uint32_t*) v;

        // cout << HEX32_STRING(vs) << endl;
        // cout << HEX32_STRING(vs +1) << endl;
        // cout << HEX32_STRING(vs +2) << endl;
        // cout << HEX32_STRING(*vs) << endl;
        // cout << HEX32_STRING(*(vs +1)) << endl;
        // cout << HEX32_STRING(*(vs +2)) << endl;

        
        if(word != 0) {

            if((i+1)+16 > t->outs[1].data.size()) {
                cout << "Breaking loop at word #" << i << " because header goes beyond received words" << endl;
                break;
            }

            cout << endl;
            print_feedback_generic(v);
            if(jamming != -1) {
                cout << "Was Jamming was for " << jamming << endl;
                jamming = -1;
            }
        } else {
            if(jamming == -1) {
                jamming = 1;
            } else {
                jamming++;
            }
        }

        bool error = false;

        uint32_t advance = feedback_word_length((feedback_frame_t*)v, &error);

        if( error ) {
            cout << "Hard fail when parsing word #" << i << endl;
            advance = 1;
            error_count++;
        }

        if( advance != 1 ) {
            // if zero we want to print because that's wrong
            // if 1 we don't want to spam during a flushing section
            // if larger we want to print because they are few
            cout << "Advance " << advance << endl;
        }

        i += advance;
    }

    file_dump_vec(t->outs[1].data, "cs30_out.hex");


    cout << "Ringbus got out" << endl;
    for(auto it = t->outs[0].data.begin(); it != t->outs[0].data.end(); it++) {
        cout << "0x" << HEX_STRING(*it) << endl;
    }


    assert(error_count == 0 && "Errors while parsing stream from cs30");

   
  // std::vector<uint32_t> enabled = {111, 122, 133, 144, 866, 877, 888, 899};

  // consider_output(t->outs[2].data, enabled);


  // cs30_node_t* cs30_node = top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  // cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  // cs10_node_t* cs10_node = top->tb_higgs_top->cs10_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;

  // file_dump_T<cs30_node_t>(cs30_node, "cs30.out");
  // file_dump_T<cs20_node_t>(cs20_node, "cs20_vmem.out");
  // file_dump_T<cs10_node_t>(cs10_node, "cs10_vmem.out");

  // hexdump_T<cs20_node_t>(cs20_node,"CS20",0, 32);
  // hexdump_T<cs10_node_t>(cs10_node,"CS10",0, 1024*4);



  // cout << "CS10 sent to dac:" << endl;
  // for(auto it = t->outs[1].data.begin(); it != t->outs[1].data.end(); it++) {
  //   cout << "0x" << HEX_STRING(*it) << endl;
  // }


  // file_dump_vec(t->outs[1].data, "cs20_out.hex");
  // file_dump_vec(t->outs[2].data, "cs10_out.hex");
  // file_dump_vec(t->outs[3].data, "cs00_out.hex");
  // file_dump_vec(t->outs[4].data, "cs01_out.hex");
  // file_dump_vec(t->outs[5].data, "cs11_out.hex");
  // file_dump_vec(t->outs[6].data, "cs21_out.hex");
  // file_dump_vec(t->outs[7].data, "cs31_out.hex");
  

  // cout << "All Tests Passed" << endl;


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












  // // note to put an ENUM here not ADDR
    // if( i == 10 ) {
      
    // }

    // if( i == 20 ) {
    //   t->inStreamAppend(0, feedback_ringbus_packet(
    //   RING_ENUM_CS10,
    //   EDGE_EDGE_IN | 0xf0,
    //   RING_ENUM_CS10,
    //   EDGE_EDGE_IN | 0xf1 )
    //    );
    // }

    // if( i == 22 ) {
    //   t->inStreamAppend(0, feedback_ringbus_packet(
    //   RING_ENUM_CS10,
    //   EDGE_EDGE_IN | 0xf2 )
    //    );
    // }




    // if( i == 35 ) {
    //   std::vector<uint32_t> arbdata = {32768, 65536, 131072, 0x42, 0x10, 0, 0xf0000000,1,2,3};
    //   t->inStreamAppend(0, feedback_vector_packet(
    //   0xff,
    //   arbdata)
    //    );
    // }

    // 512 samples takes 38 us to xor
    // if( i == 35 ) {
    //   std::vector<uint32_t> streamdata = counter_stream(512);
    //   t->inStreamAppend(0, feedback_stream_packet(
    //   0,
    //   streamdata)
    //    );
    // }

    // if( i == 40 ) {
    //   t->inStreamAppend(0, feedback_ringbus_packet(
    //   RING_ENUM_CS10,
    //   EDGE_EDGE_IN | 0xd0,
    //   RING_ENUM_CS10,
    //   EDGE_EDGE_IN | 0xd1 )
    //    );
    // }

    // if( i == 12 ) {
    //   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS10, EDGE_EDGE_IN | 0x35 ) ); // call this up to 1024 times to store data
    //   // t->tick(per_cmd);
    // }

    // if( i == 14 ) {
    //   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS10, EDGE_EDGE_IN | 0x36 ) ); // call this up to 1024 times to store data
    //   // t->tick(per_cmd);
    // }

    // if( i == 16 ) {
    //   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS10, EDGE_EDGE_IN | 0x37 ) ); // call this up to 1024 times to store data
    //   // t->tick(per_cmd);
    // }




    // if( i > us_base && ((us_base + i) % adv) == 0) {
    //   t->inStreamAppend(0, swizzle_reverse_all(counter_stream(16)) );
    // }