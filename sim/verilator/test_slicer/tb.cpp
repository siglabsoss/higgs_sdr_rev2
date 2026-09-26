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

  STANDARD_TB_START();

  // This helper is what I built to make this function easy
  // this handles reset.  You can register an arbitrary number of inputs
  // and outputs.
  // calling things like `inStreamAppend()` allows user to easily specify queue
  // input data which will be ticked over when tick is called
  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);

  // // attach handlers
  // t->negClock = &negClock;
  // t->posClock = &posClock;


  // attach streams
  // here we build a stream object which has poiners into top
  // and then we just pass this into the HiggsHelper.
  // after we do this, we can append data to these streams at any time, either here
  // or in posClock/negClock above

  // // 0th input
  // Port32In cs20in;
  // cs20in.t_data = &(top->tx_turnstile_data_in);
  // cs20in.t_valid = &(top->tx_turnstile_data_valid);
  // cs20in.valid_meter = 0;
  // cs20in.valid_state = 0;
  // t->ins.push_back(cs20in);

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


//   exit(0);


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
  auto ideal_input = file_read_hex("sc_16_144_880_1008_bpsk.hex");
  

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
  VEC_APPEND(input_looped, ideal_input);
  VEC_APPEND(input_looped, ideal_input);
  VEC_APPEND(input_looped, ideal_input);

  // cut vector (work backwards so numbers work out)


  float samples_per_us = 124.9; //  0.008006405124099279

  /////////////////////
  // List sample # which you want to create a glitch at
  // 2nd argument is the # to glitch
  // cut_vector(input_looped, 85*1280, 300);
  // cut_vector(input_looped, 65*1280, 270);

  // input looped, cut samples to CS00's input dma


  // t->inStreamAppend("cs00in", input_looped);

  std::vector<uint32_t> long_counter = get_counter(0,1024*1024);
  t->inStreamAppend("cs00in", long_counter);

  // std::vector<uint32_t> tv = get_counter(0,12);

  // for(uint32_t i = 0; i < tv.size(); i++)
  // {
  //   cout << tv[i] << endl;
  // }
    
  //   cut_vector(tv, 2, 3);


  // for(uint32_t i = 0; i < tv.size(); i++)
  // {
  //   cout << tv[i] << endl;
  // }

  // cut_vector(input_looped, 1280*40, 1000);

  // std::vector<uint32_t> a = {1,2,3,3};
  // std::vector<uint32_t> b = {7,7};

  
  // VEC_R_APPEND2(input_looped, b);


// if(0) {
//       t->inStreamAppend(2, a);
//       t->inStreamAppend(2, b);  
//     } else {
//         VEC_APPEND(input_looped, a);
//         VEC_APPEND(input_looped, b);
//         t->inStreamAppend(2, input_looped);
// }

    // auto counts = t->ins["cs00in"]->data.size();
    //   for(uint32_t i = 0; i < counts; i++)
    //   {
    //     cout << t->ins[2].data.back() << endl;
    //     t->ins[2].data.pop_back();
      // }


  // for(uint32_t i = 0; i < input_looped.size(); i++)
  // {
  //   cout << input_looped[i] << endl;
  // }

  // exit(0);

  
  // int glitch0 = 0; // how many samples to trim off the beginning
  // int glitch1 = 0; // how many samples to trim out of the middle (set to 0 for continuous input)
  // int glitch1_at = 65*1024; // how many samples in does glitch 1 occur ?

  // std::vector<uint32_t> chunk_0(
  //   ideal_input.begin() + glitch0,
  //   ideal_input.begin() + glitch1_at + glitch0
  //   );


  // std::vector<uint32_t> chunk_1(
  //   ideal_input.begin() + glitch1_at + glitch0 + glitch1,
  //   ideal_input.end()
  //   );


  // // t->inStreamAppend(2, ideal_input); // give all data for adc

  // t->inStreamAppend(2, chunk_0);
  // t->inStreamAppend(2, chunk_1);

  // for(unsigned int i = 0; i < ideal_input.size(); i++) {
  //   cout << HEX32_STRING(ideal_input[i]) << endl;
  // }

// std::vector<uint32_t> dumbseq;
// for(unsigned int i = 0; i < 10; i++) {
//     int16_t r = i*2;
//     int16_t q = -1;
//     uint32_t num = ((q&0xffff)<<16) | (r&0xffff);
//   // pick = rand() % 0xffffffff;
//   // cout << pick << endl;
//   dumbseq.push_back(num);
// }

//     file_dump_csv(dumbseq, "dumb.csv");
//     exit(0);


  // for(unsigned int i = 0; i < vinswiz.size(); i++ ) {
  //   cout << std::bitset<32>(vinswiz[i]) << endl;
  //   if(i%2==1) {
  //     cout << endl;
  //   }
  // }



  // std::vector<uint32_t> vinswiz = swizzle_reverse_all(vin);

  // cout << endl;

  // for(unsigned int i = 0; i < vinswiz.size(); i++ ) {
  //   cout << std::bitset<32>(vinswiz[i]) << endl;
  //   if(i%2==1) {
  //     cout << endl;
  //   }
  // }

  // unsigned int enabled_subcarriers = 256;

  // delay between sending inputs

  // with 256 enabled subcarriers, 16 input samples generates 256 words which are loaded into one 1024 frame
  // therefor 8 us (4000 clock cyles) is the correct delay to send data (This will overflow for very long test runs)
  unsigned int adv = 8;

  int us = 100; // estimate for 64*3*1280 to go through  1962

  // us = 100;

  int per_cmd = 140;

  unsigned us_base = 14;

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

    // if( i == 50 ) {
    //   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS00, STREAM_CMD | 0 ) ); // disable stream
    //   // t->tick(per_cmd);
    // }

    // if( i == 20 ) {
    //   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS00, STREAM_CMD | 1 ) ); // enable stream
    //   // t->tick(per_cmd);
    // }

    if( i == 50 ) {
      t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, SYNCHRONIZATION_CMD | 1 ) ); // Ask for a on-demand sync
      // t->tick(per_cmd);
    }

    // if( i == 600 ) {
    //     t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, SFO_PERIODIC_ADJ_CMD |  0x3 ) ); // amount
    //     t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, SFO_PERIODIC_SIGN_CMD |  0x2 ) ); // sign
    // }



    // first pictured reset was i = 1000

    // first cut is at us666
    // if( i == 1250 ) {
    //   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS00, SYNCHRONIZATION_CMD | 1 ) ); // Ask for a on-demand sync
    //   // t->tick(per_cmd);
    // }


    // if( i == 50 ) {
    //   t->inStreamAppend(1, ringbus_udp_packet(RING_ADDR_CS00, TURNSTILE_CMD | 42 ) ); // advance turnstile
    //   // t->tick(per_cmd);
    // }

    // if( i > us_base && ((us_base + i) % adv) == 0) {
    //   t->inStreamAppend(0, swizzle_reverse_all(counter_stream(16)) );
    // }

    t->tick(500);

    // unsigned int cs20pc = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();

    // if(t->pc_exicted_main(cs20pc)) {
    //   cout << "Exiting early due to main() returning us(" << i << ") " << main_time  << endl;
    //   break;
    // }
  }

  // std::vector<uint32_t> enabled = {111, 122, 133, 144, 866, 877, 888, 899};

  // consider_output(t->outs[2].data, enabled);


  // cs30_node_t* cs30_node = top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  // cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  // cs10_node_t* cs10_node = top->tb_higgs_top->cs10_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;

  // // file_dump_T<cs30_node_t>(cs30_node, "cs30.out");
  // file_dump_T<cs20_node_t>(cs20_node, "cs20_vmem.out");
  // file_dump_T<cs10_node_t>(cs10_node, "cs10_vmem.out");

  // // hexdump_T<cs20_node_t>(cs20_node,"CS20",0, 32);
  // // hexdump_T<cs10_node_t>(cs10_node,"CS10",0, 1024*4);

  // cout << "Ringbus got out" << endl;
  // for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
  //   cout << "0x" << HEX_STRING(*it) << endl;
  // }

  cout << "CS30 sent to dac:" << endl;
  for(auto it = t->outs["cs30out"]->data.begin(); it != t->outs["cs30out"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }

  // cout << "CS31 sent to dac:" << endl;
  // for(auto it = t->outs["cs31out"]->data.begin(); it != t->outs["cs31out"]->data.end(); it++) {
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
