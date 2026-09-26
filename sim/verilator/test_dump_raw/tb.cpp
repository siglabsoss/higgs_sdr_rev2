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
  auto ideal_input = file_read_hex("sc_16_144_880_1008_bpsk.hex");
  

  // remove remainder for a full number of samples (ie dividible by 1280)
  // uint32_t sample_of_last_full_frame = ideal_input.size() - (ideal_input.size()%(1280*16));

  // print while .size() is old value
  // cout << "got back (" << ideal_input.size() << ") (using " << sample_of_last_full_frame << "): " << endl;

  // resize (shrink) so vector is new size
  // ideal_input.resize(sample_of_last_full_frame);

  // t->inStreamAppend(2, ideal_input);
  // t->inStreamAppend(2, ideal_input);

  // std::vector<uint32_t> input_looped = {};
  // unless changed, each of these has 64 * 1280 in them
  // VEC_APPEND(input_looped, ideal_input);
  // VEC_APPEND(input_looped, ideal_input);
  // VEC_APPEND(input_looped, ideal_input);

  // cut vector (work backwards so numbers work out)


  // float samples_per_us = 124.9; //  0.008006405124099279

  /////////////////////
  // List sample # which you want to create a glitch at
  // 2nd argument is the # to glitch
  // cut_vector(input_looped, 85*1280, 300);
  // cut_vector(input_looped, 65*1280, 270);

  // input looped, cut samples to CS00's input dma


  // t->inStreamAppend("cs00in", input_looped);

  std::vector<uint32_t> long_counter = get_counter(0,1024*1024);
  // t->inStreamAppend("cs00in", long_counter);

  auto qam16_input = file_read_hex("../../data/cs10_out_qam16_rotated.hex");

  uint32_t sample_of_last_full_frame = qam16_input.size() - (qam16_input.size()%(1280));

  qam16_input.resize(sample_of_last_full_frame);

  cout << qam16_input.size() << endl;


  std::vector<uint32_t> zrs;
  zrs.resize(802-1);
  t->inStreamAppend("cs00in", zrs);


  t->inStreamAppend("cs00in", qam16_input);
  // exit(0);

  // for(auto w :  qam16_input ) {
  //   cout << HEX32_STRING(w) << endl;
  // }



  // with 256 enabled subcarriers, 16 input samples generates 256 words which are loaded into one 1024 frame
  // therefor 8 us (4000 clock cyles) is the correct delay to send data (This will overflow for very long test runs)
  unsigned int adv = 8;

  int us = 2500; // estimate for 64*3*1280 to go through  1962

  // us = 300;
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

    // if( i == 70 ) {
    //     t->inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS00, SYNCHRONIZATION_CMD | 1));
    // }

    t->tick(500);

    // unsigned int cs20pc = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();

    // if(t->pc_exicted_main(cs20pc)) {
    //   cout << "Exiting early due to main() returning us(" << i << ") " << main_time  << endl;
    //   break;
    // }
  }

    int jamming = -1;

    int error_count = 0;
    std::vector<uint32_t> stream_out;

    auto &data = t->outs["cs21out"]->data;
    uint32_t j = 0;

    for(uint32_t i = 0; i < data.size(); /*empty*/ ) {
        uint32_t word = data[i];

        // cout << "I: " << i;

        // cout << "  (" << HEX32_STRING(word) <<  ")" <<endl;

        feedback_frame_t *v;
        feedback_frame_vector_t *vec;

        // use uint32_t pointer arithmatic to and then do final cast before asignign
        v = (feedback_frame_t*) (((uint32_t*)data.data())+i);
        vec = (feedback_frame_vector_t*)v;

        uint32_t* vs = (uint32_t*) v;

        // cout << HEX32_STRING(vs) << endl;
        // cout << HEX32_STRING(vs +1) << endl;
        // cout << HEX32_STRING(vs +2) << endl;
        // cout << HEX32_STRING(*vs) << endl;
        // cout << HEX32_STRING(*(vs +1)) << endl;
        // cout << HEX32_STRING(*(vs +2)) << endl;

        bool error = false;
        
        if(word != 0) {

            if((i+1)+16 > data.size()) {
                cout << "Breaking loop at word #" << i << " because header goes beyond received words" << endl;
                break;
            }

            // cout << endl;
            // print_feedback_generic((feedback_frame_t*)v);
            error = false;
            uint32_t this_len = feedback_word_length((feedback_frame_t*)v, &error);
            // cout << "J: " << j << " " << this_len << " e: " << error << endl;
            if( !error ) {
                unsigned num_words = v->length - FEEDBACK_HEADER_WORDS;
                unsigned int* fbdata = ((unsigned int *)v)+FEEDBACK_HEADER_WORDS;

                if((i+1)+16+num_words > data.size()) {
                    cout << "Breaking loop at word #" << i << " because body goes beyond received words" << endl;
                    break;
                }

                if( vec->type == FEEDBACK_TYPE_STREAM && vec->vtype == 1 ) {
                    // cout << "got len " << num_words << endl;
                    for(size_t k = 0; k < num_words; k++) {
                        stream_out.push_back(fbdata[k]);
                    }
                }


            }

            j++;
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

        error = false;
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
            // cout << "Advance " << advance << endl;
        }

        i += advance;
    }



    dumpVector32("fb_stream.hex", stream_out);



    for( int i = 0; i < stream_out.size(); i++ ) {
        cout << HEX32_STRING(stream_out[i]) << ",";
        if( i%64==63 ) {
            cout << endl;
        }
    }


    
    cout << "Hashes of stream frames:" << endl;


    size_t hash = 0;
    for( int i = 0; (i+64) < stream_out.size(); i+=64 ) {
        std::size_t sent_a = i;
        std::size_t sent_b = i + 64;

        std::vector<uint32_t> split_a(stream_out.begin() + sent_a, stream_out.begin() + sent_b);

        cout << "0x" << HEX_STRING(hashVector32(split_a)) << endl;

        // cout << HEX32_STRING(stream_out[i]) << ",";
        // if( i%64==63 ) {
        //     cout << endl;
        // }
    }

    cout << endl;


    // Hash of stream_out 0xbe6ebe7cfe40dcbb
    // Hash of stream_out 0x8f53701aa8bb1d5d  (after removing tail)
    cout << "Hash of total: 0x" << HEX_STRING(hashVector32(stream_out)) << endl;



















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

  cout << "Ringbus got out" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
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
