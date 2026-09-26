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




// See higgs_helper.hpp

std::vector<uint32_t> data_udp_packet() {
  std::vector<uint32_t> out;
  out.push_back(0xdeadbeef);
  out.push_back(0x12345678);
  out.push_back(0x0000ffff);
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

  STANDARD_TB_START();

  HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 


  srand(320948);

  preReset(top);

  t->reset(40);

  postReset(top);

  // tb inputs starts here
  // user can tick the clock for a period
  // append data to input streams, and look at output streams
  // modify negClock() and posClock() above
  // you can also insert for check streams from those functins()


  // delay between sending inputs

  // with 256 enabled subcarriers, 16 input samples generates 256 words which are loaded into one 1024 frame
  // therefor 8 us (4000 clock cyles) is the correct delay to send data (This will overflow for very long test runs)
  unsigned int adv = 8;

  int us = 300;

  unsigned us_base = 14;

  for(unsigned int i = 0; i < us; i++) {

    if( i > us_base && ((us_base + i) % adv) == 0) {
      t->inStreamAppend("cs20in", swizzle_reverse_all(counter_stream(16)) );
    }

    t->tick(500);

    unsigned int cs20pc = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();

    if(t->pc_exicted_main(cs20pc)) {
      cout << "Exiting early due to main() returning us(" << i << ") " << main_time  << endl;
      break;
    }
  }

  // std::vector<uint32_t> enabled = {111, 122, 133, 144, 866, 877, 888, 899};

  // consider_output(t->outs[2].data, enabled);


  // cs30_node_t* cs30_node = top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  cs00_node_t* cs00_node = top->tb_higgs_top->cs00_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  cs20_node_t* cs20_node = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  cs10_node_t* cs10_node = top->tb_higgs_top->cs10_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;
  cs01_node_t* cs01_node = top->tb_higgs_top->cs01_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME;

  // file_dump_T<cs30_node_t>(cs30_node, "cs30.out");
  file_dump_T<cs00_node_t>(cs00_node, "cs00_vmem.out");
  file_dump_T<cs20_node_t>(cs20_node, "cs20_vmem.out");
  file_dump_T<cs10_node_t>(cs10_node, "cs10_vmem.out");
  file_dump_T<cs01_node_t>(cs01_node, "cs01_vmem.out");

  // hexdump_T<cs20_node_t>(cs20_node,"CS20",0, 32);
  // hexdump_T<cs10_node_t>(cs10_node,"CS10",0, 1024*4);

  cout << "Ringbus got out" << endl;
  for(auto it = t->outs["ringbusout"]->data.begin(); it != t->outs["ringbusout"]->data.end(); it++) {
    cout << "0x" << HEX_STRING(*it) << endl;
  }

  // cout << "CS10 sent to dac:" << endl;
  // for(auto it = t->outs[1].data.begin(); it != t->outs[1].data.end(); it++) {
  //   cout << "0x" << HEX_STRING(*it) << endl;
  // }


  // file_dump_vec(t->outs["cs20out"]->data, "cs20_out.hex");
  // file_dump_vec(t->outs["cs10out"]->data, "cs10_out.hex");
  // file_dump_vec(t->outs["cs00out"]->data, "cs00_out.hex");
  // file_dump_vec(t->outs["cs01out"]->data, "cs01_out.hex");
  // file_dump_vec(t->outs["cs11out"]->data, "cs11_out.hex");
  // file_dump_vec(t->outs["cs21out"]->data, "cs21_out.hex");
  // file_dump_vec(t->outs["cs31out"]->data, "cs31_out.hex");
  // file_dump_vec(t->outs["cs30out"]->data, "cs30_out.hex");

  t->allStreamDump();

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
