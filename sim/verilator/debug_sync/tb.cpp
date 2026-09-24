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

  uint32_t odfm_offset = 0;

  if(const char* env_p = std::getenv("OFDM_OFFSET")) {
      unsigned int env_test_select = atoi(env_p);
      cout << "environment variable OFDM_OFFSET was set to: " << env_test_select << endl;
      odfm_offset = env_test_select;
  }

  STANDARD_TB_START();

  // This helper is what I built to make this function easy
  // this handles reset.  You can register an arbitrary number of inputs
  // and outputs.
  // calling things like `inStreamAppend()` allows user to easily specify queue
  // input data which will be ticked over when tick is called
  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);


  srand(320948);

  preReset(top);

  t->reset(40);

  postReset(top);

  // put a counter at the beginning of the ADC Data
  // since the FPGA's take a bit to boot, some of this will be dropped
  std::vector<uint32_t> initial_counter = get_counter(0,1024*1);
  // t->inStreamAppend(2, initial_counter ); // give data for adc

  // read in some generated data we made. this will come after the counter
  // there are 117 ofdm frames in here
  auto ideal_input = file_read_hex("sc_16_144_880_1008_bpsk_mod.hex");
  

  // remove remainder for a full number of samples (ie dividible by 1280)
  uint32_t sample_of_last_full_frame = ideal_input.size() - (ideal_input.size()%(1280*16));

  // print while .size() is old value
  cout << "got back (" << ideal_input.size() << ") (using " << sample_of_last_full_frame << "): " << endl;

  // resize (shrink) so vector is new size
  ideal_input.resize(sample_of_last_full_frame);

  // t->inStreamAppend(2, ideal_input);
  // t->inStreamAppend(2, ideal_input);

  std::vector<uint32_t> input_looped = {};
  // unless changed, each of these has 64 * 1280 in them
  for(int i = 0; i < 1; i++) {
    VEC_APPEND(input_looped, ideal_input);
  }

  // trim off the front
  auto trimback = 15000 + 0;
  auto trimfront = 50 + odfm_offset;

  // trim front
  input_looped.erase(input_looped.begin(), input_looped.begin()+trimfront);

  // trim end
  input_looped.resize(input_looped.size()-trimback);
  

  cout << "input_looped size " << input_looped.size() << " which was after trim of " << trimback << endl;

  // cut vector (work backwards so numbers work out)


  float samples_per_us = 124.9; //  0.008006405124099279

  /////////////////////
  // List sample # which you want to create a glitch at
  // 2nd argument is the # to glitch
  // cut_vector(input_looped, 85*1280, 300);
  // cut_vector(input_looped, 65*1280, 270);

  // input looped, cut samples to CS00's input dma


  // t->inStreamAppend("cs00in", input_looped);

  // std::vector<uint32_t> long_counter = get_counter(0,1024*1024);
  t->inStreamAppend("cs00in", input_looped);


  // with 256 enabled subcarriers, 16 input samples generates 256 words which are loaded into one 1024 frame
  // therefor 8 us (4000 clock cyles) is the correct delay to send data (This will overflow for very long test runs)
  unsigned int adv = 8;

  int us = 220*2; // estimate for 64*3*1280 to go through  1962

  // us = 100;

  // int per_cmd = 140;

  // unsigned us_base = 14;

  bool report_flag = true;

  for(unsigned int i = 0; i < us; i++) {

    if( t->ins["cs00in"]->data.size() == 0 ) {
        if(report_flag) {
            cout << "Adc data ran out at us " << i << endl;
            report_flag = false;
        }
    } else if ( t->ins["cs00in"]->data.size() < 1024 ) {
        cout << "Adc data running low (" << t->ins["cs00in"]->data.size() << ") at us " << i << endl;
    }

    if( i == 3 ) {
        // t->enable_adc_counter(false);
        // t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, SFO_PERIODIC_ADJ_CMD |  0x3 ) ); // amount
    }

    if( i == 25 ) {
      t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, SYNCHRONIZATION_CMD | 1 ) ); // Ask for a on-demand sync
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



  cout << "Ringbus got out" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }

  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    if( ((*it) & 0xff000000) == COARSE_SYNC_PCCMD ) {
      std::string filename = "coarse_log.txt";

      std::cout << "opening " << filename << " for writing" << std::endl;
      std::ofstream outFile(filename, std::ofstream::out | std::ofstream::app);

      if (outFile.is_open()) {
          outFile << "odfm_offset,"<< odfm_offset << "," << HEX32_STRING(*it) << std::endl;
          outFile.close();
      } else { 
        cout << "File did not open" << endl;
      }

    }
  }

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
  // file_dump_vec(t->outs[8].data, "cs30_out.hex");

    t->allStreamDump();

  // file_dump_csv(t->outs[3].data, "cs00_out.csv");

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
