#ifndef __HIGGS_HELPER__
#define __HIGGS_HELPER__

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <assert.h>
#include <fstream>
#include <algorithm>
#include <map>
#include <iterator>
#include <functional>
#include <cstdint>
#include <cmath>
#include "piston_c_types.h"
#include "feedback_bus_tb.hpp"
#include "pass_fail.h"
#include "tb_inject_mask.h"


// Include common routines
//#include <verilated.h>

#include <verilated_vcd_c.h>

#ifndef VERILATE_TESTBENCH
#define VERILATE_TESTBENCH
#endif
#include "ringbus.h"
#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_PC
#include "ringbus2_post.h"

#include "convert.hpp"
#include "FileUtils.hpp"
#include "cpp_utils.hpp"
#include "CmdRunner.hpp"
#include "random.h"

// #include "tb_ringbus.hpp"

// since higgs_helper is providing everything, why not grab this
using namespace std;
using namespace siglabs::file;

using namespace std::placeholders; // for _1, _2 etc.

// forward declares
std::vector<uint32_t> ringbus_udp_packet(uint8_t ttl, uint32_t data);
uint32_t _feedback_hash(const feedback_frame_t *v);

#include "vector_helpers.hpp"

#define NSLICES (16)
#ifndef RESET
#define RESET MIB_MASTER_RESET
#endif

#include "SerialParser.hpp"

// An input to the DUT (technically an output from the perspective of the TB)
class Port32In {
public:
    unsigned int *t_data;
    unsigned char *t_valid;
    unsigned char *t_ready;
    std::vector<uint32_t> data; // pending data
    std::vector<uint32_t> history; // History of all data added, used for dumping files at the end
    unsigned int valid_meter = 0; // 0 for always valid, 1 for every other
    unsigned int valid_state = 0;
    unsigned int random_valid = 0; // 0 for always valid, [1-999] for a chance out of 1000
    unsigned int random_valid_maximum = 1000;
    bool drive_port = true;
    bool respect_ready = false; // if false test will not wait for ready, and always advance input data (think of adc)

    void Deserialize(const std::vector<uint8_t>& r) {
        const uint32_t* const load = (const uint32_t*)r.data();
        *t_data = *load;

        *t_valid = r[4];
    }

    ///
    /// Output of this should be passed to Port32Out::Deserialize
    ///
    std::vector<uint8_t> Serialize(void) const {
        std::vector<uint8_t> r;
        r.push_back(*t_ready);
        return r;
    }
};

// An output form the DUT (technically an input from the perspective of the TB)
class Port32Out {
public:
    unsigned int *i_data;
    unsigned char *i_ready;
    unsigned char *i_valid;
    std::vector<uint32_t> data; // captured data
    bool control_ready; // you should probably never set this
    unsigned int random_ready = 0; // 0 for always ready, 1 for random
    bool drive_port = true;

    ///
    /// Output of this should be passed to Port32In::Deserialize
    ///
    std::vector<uint8_t> Serialize(void) const {
        std::vector<uint8_t> r;

        const uint8_t* const load = (const uint8_t*)i_data;
        r.assign(load, load+4);

        r.push_back(*i_valid);
        return r;
    }

    void Deserialize(const std::vector<uint8_t>& r) {
        *i_ready = r[0];
    }
};

// An input to the DUT, used for ethernet mac.  We need a "last" signal for the mac
class Port8In {
public:
    uint8_t *t_data;
    unsigned char *t_valid;
    unsigned char *t_last_byte;
    std::vector<std::vector<uint8_t>> data; // pending data
    unsigned int valid_meter; // 0 for always valid, 1 for every other
    unsigned int valid_state;
};

// specalized for eth, can be made more general later
class Port8Out {
public:
    uint8_t *i_data;
    unsigned char *i_ready;
    unsigned char *i_valid;
    unsigned char *i_last_byte;
    bool i_last_byte_p = false; // delayed value of i_last_byte
    std::vector<std::vector<uint8_t>> data; // captured data
    bool control_ready; // you should probably never set this
    unsigned int random_ready; // 0 for always ready, 1 for random
};

typedef std::map<std::string, Port8Out*> outs8_map_t;
typedef std::map<std::string, Port8In*> ins8_map_t;
typedef std::map<std::string, SerialParser*> uart_map_t;
typedef std::map<std::string, Port32In*> ins_map_t;
typedef std::map<std::string, Port32Out*> outs_map_t;
typedef std::map<std::string, Port32Out*> monitor_map_t;
// typedef std::map<std::string, uint32_t (Vtb_higgs_top::*)(void)> vex_map_t;
typedef std::map<std::string, uint32_t> vex_map_t;
typedef std::pair<size_t, bool> dac_valid_t;

struct PC_functor
{
     template <class T>
     uint32_t operator()(T *top, uint32_t as_int) 
     {
        switch(as_int) {
#ifdef TB_USE_CS11
          case RING_ENUM_CS11:
              return top->tb_higgs_top->cs11_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
              break;
#endif     
#ifdef TB_USE_CS01
          case RING_ENUM_CS01:
              return top->tb_higgs_top->cs01_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
              break;
#endif
#ifdef TB_USE_CS31
          case RING_ENUM_CS31:
              return top->tb_higgs_top->cs31_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
              break;
#endif
#ifdef TB_USE_CS32
          case RING_ENUM_CS32:
              return top->tb_higgs_top->cs32_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
              break;
#endif
#ifdef TB_USE_CS22
          case RING_ENUM_CS22:
              return top->tb_higgs_top->cs22_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
              break;
#endif 
#ifdef TB_USE_CS21
          case RING_ENUM_CS21:
              return top->tb_higgs_top->cs21_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
              break;
#endif 
#ifdef TB_USE_CS20
          case RING_ENUM_CS20:
              return top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
              break;
#endif 
#ifdef TB_USE_CS12
          case RING_ENUM_CS12:
              return top->tb_higgs_top->cs12_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
              break;
#endif 
#ifdef TB_USE_CS02
          case RING_ENUM_CS02:
              return top->tb_higgs_top->cs02_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
              break;
#endif 
          default:
            assert(0 && "get_ibus can't find requested fpga");
        }             
     }
};

std::string fpgaForOutStream(const std::string out_stream) {
    if(out_stream == "cs31out") {
        return "cs31";
    }
    if(out_stream == "cs11out") {
        return "cs11";
    }
    if(out_stream == "cs01out") {
        return "cs01";
    }
    if(out_stream == "cs32out") {
        return "cs32";
    }
    if(out_stream == "cs22out") {
        return "cs22";
    }
    if(out_stream == "cs21out") {
        return "cs21";
    }
    if(out_stream == "cs20out") {
        return "cs20";
    }
    if(out_stream == "cs12out") {
        return "cs12";
    }
    if(out_stream == "cs02out") {
        return "cs02";
    }
    if(out_stream == "ethout") {
        return "eth";
    }
    return "";
}


void preReset(Vtb_higgs_top* top) {
  // Initialize inputs
  top->RESET = 1;
  top->clk = 0;
}

void postReset(Vtb_higgs_top* top) {
  // top->i_ringbus = 1;
  // top->i_data_eth = 0;
  top->i_data_adc = 0;
  // top->i_o_ready_eth = 1;
  // top->i_o_ready_dac = 1;
  // top->i_data_valid_eth = 1;
  top->i_data_valid_adc = 1;

}


std::vector<uint32_t> get_counter(const int start, const int stop) {
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

void setup_random(uint32_t fixed_seed) {

    unsigned int seed_start = std::time(0);
    
    if(const char* env_p = std::getenv("TEST_SEED")) {
        unsigned int env_seed = atoi(env_p);
        cout << "environment variable TEST_SEED was set to: " << env_seed << endl;
        fixed_seed = env_seed;
    }

    if(fixed_seed != 0) {
        seed_start = fixed_seed;
        cout << "starting with hard-coded seed " << seed_start << endl;
    } else {
        cout << "starting with random seed " << seed_start << endl;
    }

    srand(seed_start);
}

template <class T>
uint32_t vmem_T(T * node, uint32_t r_addr){  
        uint32_t addr = r_addr/NSLICES;
        switch(r_addr & 0xF){
            case 0 : return node->mem_slice_0->dpram_inst->rtn_mem(addr);
            case 1 : return node->mem_slice_1->dpram_inst->rtn_mem(addr);
            case 2 : return node->mem_slice_2->dpram_inst->rtn_mem(addr);
            case 3 : return node->mem_slice_3->dpram_inst->rtn_mem(addr);
            case 4 : return node->mem_slice_4->dpram_inst->rtn_mem(addr);
            case 5 : return node->mem_slice_5->dpram_inst->rtn_mem(addr);
            case 6 : return node->mem_slice_6->dpram_inst->rtn_mem(addr);
            case 7 : return node->mem_slice_7->dpram_inst->rtn_mem(addr);
            case 8 : return node->mem_slice_8->dpram_inst->rtn_mem(addr);
            case 9 : return node->mem_slice_9->dpram_inst->rtn_mem(addr);
            case 10 : return node->mem_slice_10->dpram_inst->rtn_mem(addr); 
            case 11 : return node->mem_slice_11->dpram_inst->rtn_mem(addr);
            case 12 : return node->mem_slice_12->dpram_inst->rtn_mem(addr);
            case 13 : return node->mem_slice_13->dpram_inst->rtn_mem(addr);
            case 14 : return node->mem_slice_14->dpram_inst->rtn_mem(addr);
            case 15 : return node->mem_slice_15->dpram_inst->rtn_mem(addr);
            default: std::cout << "illegal argument to vmem_T " << r_addr << endl; return 0;
        }
}

/// fetch vmem words, can be run at any time to grab vectors
/// @params [in] node  : verilated type
/// @params [in] start_dma : dma address to start with
/// @params [in] length : how many words
template <class T>
std::vector<uint32_t> readVmemUnode(T * node, const unsigned start_dma, const unsigned length) {
    const unsigned end = start_dma+length;

    constexpr unsigned maximum = (4096*NSLICES);

    if( start_dma > maximum ) {
        cout << "invalid arguments for read_vmem()\n";
        return {};
    }
    if( end > maximum ) {
        cout << "invalid arguments for read_vmem()\n";
        return {};
    }
    if( length > maximum ) {
        cout << "invalid arguments for read_vmem()\n";
        return {};
    }
    

    std::vector<uint32_t> out;
    out.reserve(length);

    for(unsigned int i = start_dma; i < end; i++) {
        out.push_back( vmem_T<T>(node, i) );
    }

    return out;
}


typedef std::function<void(const uint32_t)> ring_cb_t;

typedef std::pair<uint32_t, ring_cb_t>  ring_pair_cb_t;

typedef std::function<void(void)> fb_error_cb_t;

typedef std::function<void(const feedback_frame_t *v)> fb_raw_cb_t;
typedef std::function<void(uint32_t, uint32_t, const std::vector<uint32_t>&, const std::vector<uint32_t>&)> fb_cb_t;


template <class T>
class HiggsHelper {

public:
    T *top;
    uint64_t* main_time;
    VerilatedVcdC* tfp = NULL;
    unsigned char *clk;
    ins8_map_t ins8;
    outs8_map_t outs8;
    uart_map_t uarts;
    ins_map_t ins;
    outs_map_t outs;
    monitor_map_t monitors;
    vex_map_t vexs;
    PC_functor functor;
    const unsigned int pc_done[3] = {0xE4, 0xE8, 0xEC};
    std::vector<dac_valid_t> dac_valid_history;
    bool dac_valid_p = false;
    bool monitor_adc_in = false;
    bool monitor_dac_valid = true;
    bool adc_status_suppress = false;
    bool warn_ringbus_erased = true;
    unsigned mega_wrapper_parse_progress = 0;

    bool parse_feedback_bus = false;
    unsigned fbp_progress = 0;
    int fbp_jamming = -1;
    unsigned fbp_error_count = 0;
    fb_error_cb_t fbp_error_cb = 0;
    fb_raw_cb_t fbp_raw_cb = 0;
    fb_cb_t fbp_cb = 0;
    bool fbp_print_jamming = true;
    bool fbp_dump = false;
    std::map<std::string, std::ofstream*> fbp_file_map;
    std::string prefix;
    std::string path_prefix;
    bool is_fork = false;
    uint32_t fork_id = 0;
    uint32_t cached_pf1 = 0;

    // dispatch ringbus
    std::vector<ring_pair_cb_t> rb_callbacks;
    std::vector<uint32_t> rb_previous;

    const std::vector<std::string> enabled_fpgas;
    constexpr std::vector<std::string> GetEnabledFPGAS(void);


    uint32_t (Vtb_higgs_top::*get_iBus_cmd_payload_pc)(void);

    HiggsHelper(T *_top, uint64_t *_main_time, VerilatedVcdC *_tfp):
    top(_top)
    ,main_time(_main_time)
    ,tfp(_tfp)
    ,clk(&(_top->clk))
    ,enabled_fpgas(GetEnabledFPGAS())
    {
        this->initialize_ports();
        SetupIsVerilatorInject();
    }

    void SetupMultipleHiggs(const uint32_t _fork_id) {
        is_fork = true;
        fork_id = _fork_id;
        prefix = std::string("Higgs ") + std::to_string(fork_id) + std::string(": ");


        path_prefix = std::string("higgs_") + std::to_string(fork_id) + std::string("/");

        mkdir(path_prefix.c_str(), 0775);


        cached_pf1 |= ((_fork_id+1)&TBI_HIGGS_ID_MASK)<<TBI_HIGGS_ID_SHIFT;
        SetupIsVerilatorInject();
    }

    void SetupIsVerilatorInject(void) {
        cached_pf1 |= (0x1<<TBI_IS_VERILATOR_SHIFT);
        InjectCachedPf1();
    }


    void InjectCachedPf1(void) {
        const uint32_t pf1_byte = (uint64_t)(void*)pass_fail_1;
        const uint32_t pf1_word = pf1_byte / 4;

        // cout << "got seed " << seed << " and " <<  pf0_word << "\n";

        for(const auto fpga : enabled_fpgas) {
            this->writeImemWord(fpga, pf1_word, cached_pf1);
        }
    }


    uint64_t us(void) const {
        return *main_time / 1000;
    }

    void enableParseFeedbackBus(bool enable = true) {
        parse_feedback_bus = enable;
    }
    void enableDumpFbBus(bool enable = true) {
        parse_feedback_bus = true;
        fbp_dump = enable;
    }

    std::string parse_feedback_bus_fpga = "cs20out";

    void customParseFeedbackBusEndpoint(std::string x) {
        parse_feedback_bus_fpga = x;
    }

    // seems like we have a bug, if there are no trailing zeros
    // doParseFeedbackBus() won't parse the last packet
    // this hack just adds zeros which you must do at the right time in order
    // to get the last packet
    void customParseFeedbackPadZeros(const unsigned count = 1) {
        auto &data = this->outs[parse_feedback_bus_fpga]->data;
        for(unsigned i = 0; i < count; i++) {
            data.push_back(0);
        }
    }

    ///
    /// fbp prefix means "feedback bus parse"
    void doParseFeedbackBus(void) {
        auto &data = this->outs[parse_feedback_bus_fpga]->data;

        const feedback_frame_t* v;


        // cout << "i: " << data.size() << " - " << fbp_progress ;
        unsigned i;
        for(i = fbp_progress; i < data.size(); /*empty*/ ) {
            const auto word = data[i];
            // cout << i << ",";
            // cout << HEX32_STRING(w) << "\n";

            v = (feedback_frame_t*) (((uint32_t*)data.data())+i);

            bool error = false;
            uint32_t advance = feedback_word_length((const feedback_frame_t*)v, &error);

            // cout << "i: " << i << " sz " << data.size() << " adv " << advance  << "\n";

            if(word != 0) {

                if((i+1)+advance > data.size()) {
                    // std::cout << "Breaking loop at word #" << i
                    //   << " because header goes beyond received words\n";
                    break;
                }

                
                // cout << "\n";
                // print_feedback_generic((const feedback_frame_t*)v);
                if( !error ) {
                    dispatchFb((const feedback_frame_t*)v);
                }
                if(fbp_jamming != -1) {
                    if( fbp_print_jamming ) {
                        cout << "Was Jamming was for " << fbp_jamming << "\n";
                    }
                    fbp_jamming = -1;
                }
            } else {
                if(fbp_jamming == -1) {
                    fbp_jamming = 1;
                } else {
                    fbp_jamming++;
                }
            }



            if( error ) {
                std::cout << "Hard fail when parsing word #" << i << "\n";
                advance = 1;
                fbp_error_count++;
                if( fbp_error_cb ) {
                    fbp_error_cb();
                }
            }

            if( advance != 1 ) {
                // If zero we want to print because that's wrong
                // If 1 we don't want to spam during a flushing section
                // If larger we want to print because they are few
                // cout << "Advance " << advance << "\n";
            }

            // cout << "Adv: " << advance << ",";

            i += advance;
        }

        // cout << "\n";

        // fbp_progress += data.size() - fbp_progress;

        if( fbp_progress < data.size() ) {
            // only run this if the loop above entered
            // we use the same condition as the loop for if()
            fbp_progress += (i) - fbp_progress;
        }
    }

    /// calls all of the correct user functions based on what's enabled
    ///
    void dispatchFb(const feedback_frame_t *v) {
        const uint32_t* p = (const uint32_t*)v;
        int blen = v->length - FEEDBACK_HEADER_WORDS;

        std::vector<uint32_t> header;
        header.assign(p, p+FEEDBACK_HEADER_WORDS);

        // cout << "llen: " << blen << "\n";

        std::vector<uint32_t> body;

        if( blen > 0 ) {
            body.assign(p+FEEDBACK_HEADER_WORDS, p+FEEDBACK_HEADER_WORDS+blen);
        }

        if( fbp_raw_cb ) {
            fbp_raw_cb(v);
        }

        uint32_t type0 = v->type;
        uint32_t type1 = ((const feedback_frame_vector_t*)v)->vtype;

        if( fbp_cb ) {
            fbp_cb(type0, type1, header, body);
        }

        if( fbp_dump ) {
            doDumpFb(type0, type1, header, body);
        }
    }

    void doDumpFb(uint32_t t0, uint32_t t1, const std::vector<uint32_t>& header, const std::vector<uint32_t>& body) {

        constexpr unsigned digits = 2;

        std::string number0 = std::to_string(t0);

        while(number0.size() < digits) {
            number0 = "0" + number0;
        }

        std::string number1 = std::to_string(t1);

        while(number1.size() < digits) {
            number1 = "0" + number1;
        }

        std::string name = "frame_" + number0 + "_" + number1 + ".hex";
        // std::map<std::string, std::ofstream*> fileMap;

        std::ofstream* fstream; 

        if( fbp_file_map.find(name) != fbp_file_map.end() ) {
            // cout << "found\n";
            fstream = fbp_file_map[name];
        } else {
            // cout << "not found\n";
            fstream = new std::ofstream(name);
            fbp_file_map[name] = fstream;
        }

        if( !fstream->is_open() ) {
            cout << "Error: doDumpFb() couldn't open '" << name << "'\n";
        }

        for(const auto w : header) {
            (*fstream) << HEX32_STRING(w) << "\n";
        }

        for(const auto w : body) {
            (*fstream) << HEX32_STRING(w) << "\n";
        }

        fstream->flush();

        // cout << "would have written " << name << "\n";

    }

    void registerRawFbCb(fb_raw_cb_t cb) {
        parse_feedback_bus = true;
        fbp_raw_cb = cb;
    }

    void registerFbCb(fb_cb_t cb) {
        parse_feedback_bus = true;
        fbp_cb = cb;
    }
    void registerFbError(fb_error_cb_t cb) {
        fbp_error_cb = cb;
    }

    void registerRb(ring_cb_t cb, uint32_t w) {
        rb_callbacks.push_back(ring_pair_cb_t(w,cb));
    }

    void dispatchSingleRbCallback(const uint32_t found) {
        const unsigned int type = found & 0xff000000;
        const unsigned int data = found & 0x00ffffff;

        for(const auto row : rb_callbacks) {
            uint32_t a;
            ring_cb_t cb;
            std::tie(a,cb) = row;

            if( a == type ) {
                cb(data);
            }
        }
    }

    void checkDispatchRbCallbacks(std::vector<uint32_t>& rb, std::vector<uint32_t>& now) {
        if( rb == now ) {
            return;
        }
        if( rb.size() > now.size() ) {
            if( warn_ringbus_erased ) {
                cout << "Warning dispatch_ringbus saw 'now' shrink\n";
            }
            rb = now;
            return;
        }

        unsigned foundnew = now.size() - rb.size();

        if( foundnew == 0 ) {
            cout << "Warning dispatch_ringbus got same size different values, aborting\n";
            return;
        }

        // cout << "\n\n";
        for(int i = rb.size(); i < now.size(); i++ ) {
            const auto w = now[i];
            // cout << "new rb: " << HEX32_STRING(w) << "\n";
            dispatchSingleRbCallback(w);
        }

        rb = now;
        // for(const auto w : now ) {
        //     cout << HEX32_STRING(w) << "\n";
        // }
    }


    void handleMonitorDacValid() {

        if( !monitor_dac_valid ) {
            return;
        }

        bool force = dac_valid_history.size() == 0;

        // std::cout << *main_time << '\n';

        // read the current value and cast
        bool dac_valid = (bool)top->o_data_valid_dac;
        
        if( (dac_valid_p != dac_valid) || force ) {
            dac_valid_history.emplace_back(dac_valid, *main_time);

            std::string msg = "invalid";
            if(dac_valid) {
                msg = "valid  ";
            }

            if( !force ) {
                std::cout << "CS01 output to DAC became " << msg << " at " << *main_time << "ns\n";
            }
        }
        //     o_data_valid_dac
        
        // std::cout << dac_valid << '\n';

        dac_valid_p = dac_valid;
    }

    // pass number of clock cycles to reset, must be even number and greater than 4
    void reset(unsigned count=40) {
        count += count % 2; // make even number

        // bound minimum
        if(count < 4) {
                count += 4 - count;
        }

        *clk = 0;
        top->RESET = 1;
        for(auto i = 0; i < count; i++) {

                if(i+2 >= count)
                {
                        top->RESET = 0;
                }

                (*main_time)++;
                *clk = !*clk;
                if(tfp) {tfp->dump(*main_time);}
                top->eval();
        }
        top->RESET = 0;
    }

    // previously took an index, now just takes a pointer to the obj
    // handles data going into DUT
    void handleDataInNegIndex(Port32In* port, std::string key) {
        if( !port->drive_port ) {
            return;
        }



        using namespace std;
        unsigned int meter_flag = 0;
        if( port->valid_meter != 0 ) {
            // do meter
            if( port->valid_state == 0 )
            {
                meter_flag = 1;
            }

            port->valid_state = (port->valid_state+1) % port->valid_meter;
        } else {
            meter_flag = 1;
        }



        if( port->random_valid == 0 ) {
            if( meter_flag && port->data.size() ) {
                *(port->t_data) = port->data.back();
                *(port->t_valid) = 1;
            } else {
                *(port->t_valid) = 0;
            }
        } else {
            int pull = rand() % port->random_valid_maximum;

            if( port->data.size() && (pull <= port->random_valid) ) {
                *(port->t_data) = port->data.back();
                *(port->t_valid) = 1;
            } else {
                *(port->t_valid) = 0;
            }
        }
        
    }

    // handles data going into DUT
    void handleDataInPosIndex(Port32In* port, std::string key) {
        if( !port->drive_port ) {
            return;
        }

        unsigned int meter_flag = 0;
        if( port->valid_meter != 0 && port->data.size()) {
            if( port->valid_state == 0 ) {
                meter_flag = 1;
            }
        } else {
            meter_flag = 1;
        }


        // WIP enable pushback from DUT to tb on inputs
        // works for ringbus only right now
        if( key == "ringbusin" ) {

            if(meter_flag && port->data.size() &&  (*(port->t_ready) == 1)  ) {
                port->data.pop_back();
            }else if (*(port->t_ready) != 1) {
                // if(key == "ringbusin") {
                //     cout << "tb->" << key << " was not ready " << "at main_time " << (*this->main_time) << endl; 
                // }
            }
        } else {

            // old behaviour

            if( port->respect_ready ) {
                // normal valid ready
                if(port->data.size() && *(port->t_ready) && *(port->t_valid) ) {
                    port->data.pop_back();
                }
            } else {
                // adc behavior
                if( port->data.size() && *(port->t_valid) ) {
                    port->data.pop_back();
                }
            }

            
        } // if key
    } // handleDataInPosIndex


    void handleDataOutNegIndex(Port32Out* port) {
        if( !port->drive_port ) {
            return;
        }
        if( port->control_ready ) {

          if(port->random_ready) {
            uint32_t pick = rand();
            bool ready = (pick % 1000) > 500;
            // std::cout << "pick " << pick << std::endl;
            // std::cout << "ready " << ready << std::endl;
            *(port->i_ready) = ready;
          } else {
            *(port->i_ready) = 1;
          }
        }
    }

    void handleDataOutPosIndex(Port32Out* port) {
        if( !port->drive_port ) {
            return;
        }
        if( *(port->i_ready) && *(port->i_valid) ) {
            port->data.push_back( *(port->i_data) );
        }
    }


    void handleData8OutPos(Port8Out* port) {
        if( *(port->i_ready) && *(port->i_valid) || *(port->i_last_byte) ) {
            port->data.back().push_back( *(port->i_data) );
        }
    }

    void handleData8OutNeg(Port8Out* port) {
        if( port->control_ready ) {
            if(port->random_ready) {
                // uint32_t pick = rand();
                // bool ready = (pick % 1000) > 500;
                // // std::cout << "pick " << pick << std::endl;
                // // std::cout << "ready " << ready << std::endl;
                // *(port->i_ready) = ready;
            } else {


                if( false ) {
                    cout <<
                                (int)*(port->i_valid) 
                     << ", " << (int) *(port->i_last_byte)
                     << ", " << (int) port->i_last_byte_p
                     << ", " << HEX8_STRING((int)*(port->i_data))
                     << "\n";
                 }

                if( *(port->i_valid) ) {
                    if( *(port->i_last_byte) ) {
                        *(port->i_ready) = 0;
                    } else {
                        *(port->i_ready) = 1;
                    }
                } else {
                    *(port->i_ready) = 0;
                }
                if( port->i_last_byte_p ) {
                    port->data.emplace_back();
                }
            }
        }

        port->i_last_byte_p = *(port->i_last_byte);
    }
    void handle_uart_neg(SerialParser *serial) {

        // handle uart in the direction of tb -> riscv
        // this direction is TBD, so we just set the line high
        *(serial->line_in) = (uint8_t)1;


        if ((serial->start_bit) >= serial->UART_START_BIT_LEN) {
            serial->start_bit++;
            if (!(serial->start_bit % serial->UART_START_BIT_LEN)) {
                serial->value |= (*(serial->line_out) << \
                                  (serial->start_bit / serial->UART_START_BIT_LEN - 2));
            }
            if (serial->start_bit == serial->UART_START_BIT_LEN * serial->UART_PKT_LEN) {
                if( serial->print_chars ) {
                    std::cout << serial->fpga << " Sent UART data: "
                              << HEX8_STRING( (uint32_t) serial->value) << "\n";
                }
                serial->parser(serial->value);
                serial->start_bit = 0;
                serial->value = 0;
                // std::cout << std::hex << std::showbase << "Sent UART data: "
                //           << (uint32_t) serial->uart_data.back() << "\n";
            }
        } else if (!*(serial->line_out)) {
            serial->start_bit++;
        }
    }

    // previously took an index, now just takes a pointer to the obj
    // handles data going into DUT
    // FLIPPING CONSUMPTION ORDER from other ports as others were broken
    void handleData8InNeg(Port8In* port) {
        if( port->data.size() && port->data.begin()->size() )
            {
                *(port->t_data) = *port->data.begin()->begin();
                *(port->t_valid) = 1;
            } else {
                *(port->t_valid) = 0;
            }
    }

    // handles data going into DUT
    void handleData8InPos(Port8In* port) {

        // old behaviour
        if( port->data.size() && port->data.begin()->size() > 1 ) {
            port->data.begin()->erase(port->data.begin()->begin());
            // port->data.begin()->pop_back();
        } else {

            if( port->data.size() && port->data.begin()->size() <= 1 ) {

                if( *(port->t_last_byte) ) {
                    *(port->t_last_byte) = 0;
                    port->data.erase(port->data.begin());
                } else {
                    *(port->t_last_byte) = 1;
                    port->data.begin()->erase(port->data.begin()->begin());
                }

                // t_last_byte
            }
        }
        
    } // handleDataInPosIndex



    void handleDataNeg() {
        for (auto it = this->ins.begin(); it != this->ins.end(); ++it){
            this->handleDataInNegIndex(it->second, it->first);
        }
        for (auto it = this->outs.begin(); it != this->outs.end(); ++it){
            this->handleDataOutNegIndex(it->second);
        }
        for (auto it = this->monitors.begin(); it != this->monitors.end(); ++it){
            this->handleDataOutNegIndex(it->second);
        }
        for (auto it = this->outs8.begin(); it != this->outs8.end(); ++it){
            this->handleData8OutNeg(it->second);
        }
        for (auto it = this->ins8.begin(); it != this->ins8.end(); ++it){
            this->handleData8InNeg(it->second);
        }
        for (auto it = this->uarts.begin(); it != this->uarts.end(); ++it) {
            this->handle_uart_neg(it->second);
        }
    }

    void handleDataPos() {
        for(auto it = this->ins.begin(); it != this->ins.end(); ++it) {
            this->handleDataInPosIndex(it->second, it->first);
        }
        for(auto it = this->outs.begin(); it != this->outs.end(); ++it) {
            this->handleDataOutPosIndex(it->second);
        }
        for(auto it = this->monitors.begin(); it != this->monitors.end(); ++it) {
            this->handleDataOutPosIndex(it->second);
        }
        for (auto it = this->outs8.begin(); it != this->outs8.end(); ++it){
            this->handleData8OutPos(it->second);
        }
        for (auto it = this->ins8.begin(); it != this->ins8.end(); ++it){
            this->handleData8InPos(it->second);
        }
    }

    // because we are using data.back() and data.pop_back() to insert data into the DUT
    // we need to add it in this backwards weird way.
    void inStreamAppend(std::string name, std::vector<uint32_t> din) {
        if( this->ins.find(name) == this->ins.end() ) {
            throw std::invalid_argument( std::string("inStreamAppend() did not find fpga: ")+name );
        }

        Port32In * port = this->ins[name];

        // std::cout << "found " << &(port->data) << endl;
        // std::cout << "size " << port->data.size() << endl;

        VEC_R_APPEND(port->data, din);
        VEC_APPEND(port->history, din);

        // std::cout << "size " << port->data.size() << endl;
      }


    void inPacketAppend(std::string name, std::vector<uint8_t> p) {
        assert((this->ins8.find(name) != this->ins8.end()) && "Test");

        Port8In * port = this->ins8[name];

        port->data.emplace_back(p);
    }


    void entered_next_us(uint64_t us) {
        // std::cout << "here " << us << "us" << std::endl;
        print_time(us);

#ifdef TB_USE_CS31
        print_adc_fill(us);
#endif

#ifdef ETH_USE_MEGA_WRAPPER
        parseEthRxPackets();
#endif

        if( parse_feedback_bus ) {
            doParseFeedbackBus();
        }

        checkDispatchRbCallbacks(rb_previous, outs["ringbusout"]->data);

    }

#ifdef TB_USE_CS31
    void print_adc_fill(uint64_t us) {
        if( !monitor_adc_in ) {
            return;
        }

        if( ins["cs31in"]->data.size() == 0 ) {
            if(!adc_status_suppress) {
                cout << "Testbench 'acting as ADC' data ran out at us " << us << endl;
                // report_flag = false;
                adc_status_suppress = true;
            }
            
        } else if ( ins["cs31in"]->data.size() < 1024 ) {
            cout << "Testbench 'acting as ADC' data running low (" << ins["cs31in"]->data.size() << ") at us " << us << endl;
        }

        if( ins["cs31in"]->data.size() > 0 ) {
            if( adc_status_suppress ) {
                cout << "Testbench 'acting as ADC' data filled to ~" << ins["cs31in"]->data.size() << " samples at us " << us << "\n";
            }
            adc_status_suppress = false;
        }
    }
#endif


    void check_us() {
        constexpr int increment = (1000); // 1 us
        constexpr int convert_us = (1000);
        if(*main_time % increment == 0) {
            uint64_t us = *main_time / convert_us;
            entered_next_us(us);
        }
    }




    void print_time(uint64_t us) {
        constexpr int increment = (100); // 100 us
        if(us % increment == 0) {
            int val;
            std::cout << us << "us" << std::endl;
        }
    }

    // pass a fpga, returns a filename without the extension or period
    std::string filenameForFPGA(std::string fpga) {

        if(fpga == "cs31out") {
            return "cs31_out";
        }
        if(fpga == "cs11out") {
            return "cs11_out";
        }
        if(fpga == "cs01out") {
            return "cs01_out";
        }
        if(fpga == "cs32out") {
            return "cs32_out";
        }
        if(fpga == "cs22out") {
            return "cs22_out";
        }
        if(fpga == "cs21out") {
            return "cs21_out";
        }
        if(fpga == "cs21in") {
            return "cs21_in";
        }
        if(fpga == "cs20out") {
            return "cs20_out";
        }
        if(fpga == "cs12out") {
            return "cs12_out";
        }
        if(fpga == "cs02out") {
            return "cs02_out";
        }
        if(fpga == "ringbusout") {
            return "ringbus_out";
        }
        if(fpga == "cs31in") {
            return "cs31_in";
        }
        if(fpga == "mcs31in") {
            return "cs31_in";
        }
        if(fpga == "cs11in") {
            return "cs11_in";
        }
        if(fpga == "mcs11in") {
            return "cs11_in";
        }
        if(fpga == "ringbusin") {
            return "ringbus_in";
        }
        if(fpga == "mmapmovin") {
            return "mapmov_in";
        }
        if(fpga == "ethout") {
            return "eth_out";
        }
        if(fpga == "macout") {
            return "mac_out";
        }
        
        cout << "WARNING: filenameForFPGA() doesn't know about " << fpga << endl;
        return fpga;
    }


  void outStreamDump(std::string name) {

    assert((this->outs.find(name) != this->outs.end()) && "Test");

    std::string filename = path_prefix + filenameForFPGA(name) + ".hex";    

    std::cout << "opening " << filename << " for writing" << std::endl;
    std::ofstream outFile(filename);
    
    Port32Out * port = this->outs[name];

    if (outFile.is_open()){

      for(auto it = port->data.begin(); it != port->data.end(); it++) {
        outFile << HEX32_STRING(*it) << std::endl;
      }
      outFile.close();
    }
  }


// internal usage
template <class Y>
void dumpOneStream(std::string s, Y portin, bool print = true) {
    std::string filename = path_prefix + filenameForFPGA(s) + ".hex";

    if( print ) {
        std::cout << "opening " << filename << " for writing\n";
    }
    std::ofstream outFile(filename);

    Y port = portin;

    if (outFile.is_open()) {
        for(auto it = port->data.begin(); it != port->data.end(); it++) {
              outFile << HEX32_STRING(*it) << std::endl;
        }
        outFile.close();
    }
}

template <class Y>
void dumpOneStreamHistory(std::string s, Y portin, bool print = true) {
    std::string filename = path_prefix + filenameForFPGA(s) + ".hex";

    if( print ) {
        std::cout << "opening " << filename << " for writing\n";
    }
    std::ofstream outFile(filename);

    Y port = portin;

    if (outFile.is_open()) {
        for(auto it = port->history.begin(); it != port->history.end(); it++) {
              outFile << HEX32_STRING(*it) << std::endl;
        }
        outFile.close();
    }
}


void allStreamDump(bool print = true) {

    // this loop could be removed as we are only doing 1
    for (auto it = this->ins.begin(); it != this->ins.end(); ++it) {
        if( it->first == "ringbusin") {
            dumpOneStreamHistory<Port32In*>(it->first, it->second, print);
        }
    }

    for (auto it = this->monitors.begin(); it != this->monitors.end(); ++it) {
        dumpOneStream<Port32Out*>(it->first, it->second, print);
    }

    for (auto it = this->outs.begin(); it != this->outs.end(); ++it) {
        dumpOneStream<Port32Out*>(it->first, it->second, print);
    }
}


// clock from 0 to 1
void tickHigh(void) {
   (*main_time)++;
   *clk = !*clk;
   top->eval();
   if(tfp) {tfp->dump(*main_time);}
   this->handleDataNeg();
}


void tickLow(void) {
    *clk = !*clk;
    (*main_time)++;
    top->eval();
    if(tfp) {tfp->dump(*main_time);}
    this->handleDataPos();

    // watches cs01 output to dac
    this->handleMonitorDacValid();
    this->handleMonitorRbDrop();
    check_us();
}


void tick(const unsigned count = 1){
    for(unsigned i = 0; i < count; i++) {
        tickHigh();
        tickLow();
    }
}

// issue #40
void handleMonitorRbDrop() {
    // bool cs20 = getRingbusDropped(RING_ENUM_CS20);
    // if( cs20 ) {
    //     cout << "CS20 Dropped Ringbus!!!!! at " << *main_time << " (" << *main_time / 1000 << " us)\n";
    // }
}

  // pass a PC from any of the vex to see if we think it' has left main(){}
  // returns true if it program inside vex has finished
  bool pc_exicted_main(std::string fpga) {
    uint32_t check = this->get_ibus(fpga);


    if(
      check == this->pc_done[0] ||
      check == this->pc_done[1] ||
      check == this->pc_done[2]
      ) {
      return true;
    }
    return false;
}

  uint32_t get_ibus(std::string fpga) {

        uint32_t as_int = this->vexs[fpga];

        return this->functor(this->top,as_int);

  }

    /**
     * Sends a ringbus command to disable ADC counter. When disabled, real data
     * will output from the ADC.
     *
     */
    void disable_adc_counter(bool wait = true) {
        inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, DISAB_ADC_COUNTER_CMD) );
    
        if(wait) {
            tick(1000);
        }
    }

    /**
     * Sends a ringbus command to enable ADC counter. When enabled, a counter
     * value will ouput from ADC as opposed to real ADC data.
     *
     */
    void enable_adc_counter(bool wait = true) {
        std::cout << "enable_adc_counter()" << std::endl;
        inStreamAppend("ringbusin", ringbus_udp_packet(RING_ADDR_CS20, EN_ADC_COUNTER_CMD) );
    
        if(wait) {
            tick(1000);
        }
    }

    /**
     * This method sends a ringbus command to ETH to set the DSA attenuation.
     * The permissible attenuation settings range from 0 - 30 dB in steps of
     * 2 dB. 
     *
     * Args:
     *     atten (unsigned int): DSA attenuation value. Input value ranges from
     * 0 - 30 in steps of 2. A valid input is 12.
     *
     */
    void set_dsa_gain(unsigned int atten, bool wait = true) {
        unsigned int dsa_atten = (atten*4)<<3;
        unsigned int packet_data = DSA_GAIN_CMD|dsa_atten;
        inStreamAppend(1, ringbus_udp_packet(RING_ADDR_ETH, packet_data));
        if(wait) {
            tick(500);
        }
    }

    /**
     * Sends a ringbus command to ETH to set VGA attenuation. The permissible
     * attenuation values in the VGA range from 0 - 32 dB in steps of 1 dB.
     *
     * Args:
     *     data (unsigned int): A 24 bit value where the least significant 8
     * bits is the attenuation value (0 - 32). Bits 8 - 15 is the gain address
     * (0x0200). Bits 16 - 23 is the VGA channel (A:0x00010000, B:0x00000000).
     * As an example, to set channel B with an attenuation value of 3, pass the
     * 0x00000000|0x00000200|0x3
     *
     */
    void set_vga_gain(unsigned int data){
        unsigned int packet_data = VGA_GAIN_CMD|data;
        inStreamAppend(1, ringbus_udp_packet(RING_ADDR_ETH, packet_data));
        tick(500);
    }

    /*
     * Sends a ringbus command to ETH to configure DAC. To configure
     * the DAC in Higgs, a series of 24 bit codes is sent to ETH. A list of
     * values to send sequentially can be found in scripts/dac_module.py.
     *
     * Args:
     *     data (unsigned int): 24 bit value to configure DAC. A list of values
     * can be found in scripts/dac_module.py
     */
    void set_dac_module(unsigned int data){
        unsigned int packet_data = CONFIG_DAC_CMD|data;
        inStreamAppend(1, ringbus_udp_packet(RING_ADDR_ETH, packet_data));
        tick(500);
    }

    /**
     * Print outcoming ringbus from Higgs during simulation
     *
     * @param[in] erase: If set true will delete all ringbus messages. Set to
     * false by default
     */
    void print_ringbus_out(bool erase = false) {
        std::cout << prefix << "Ringbus got out "
                  << this->outs["ringbusout"]->data.size() << " items\n";
        for(auto w : this->outs["ringbusout"]->data ) {
            std::cout << prefix << "0x" << HEX32_STRING(w) << std::endl;
        }
        if(erase) {
            this->outs["ringbusout"]->data.resize(0);
        }
    }

    /**
     * Simulates the sending of ringbus commands from PC
     *
     * @params[in] fpga: An ENUM value representing which FPGA to send a
     * message. If ringbus2_pre.h and ringbus2_post.h are included, these are
     * define as RING_ADDR_XXX.
     * @params[in] data: Ringbus data to send to FPGA
     */
    void send_ring(uint32_t fpga, uint32_t data) {
        inStreamAppend("ringbusin", ringbus_udp_packet(fpga, data));
    }

  void initialize_ports(){
      Port32Out* eth_rx = new Port32Out();
      eth_rx->i_data = &(top->o_rx_data_eth);
      eth_rx->i_valid = &(top->o_rx_valid_eth);
      eth_rx->i_ready = &(top->i_rx_ready_eth);
      eth_rx->control_ready = 1;
      this->outs.insert(std::make_pair("ethout", eth_rx));
      
    Port32In* ringbusin = new Port32In();
    ringbusin->t_data = &(top->ringbus_in_data);
    ringbusin->t_valid = &(top->ringbus_in_data_vld);
    ringbusin->t_ready = &(top->ringbus_in_data_ready); // FIXME, broken?
    ringbusin->valid_meter = 0;
    ringbusin->valid_state = 0;
    this->ins.insert(std::make_pair("ringbusin", ringbusin));

    Port32Out* ringbusout = new Port32Out();
    ringbusout->i_data = &(top->ringbus_out_data);
    ringbusout->i_valid = &(top->ringbus_out_data_vld);
    ringbusout->i_ready = &(top->ring_bus_i0_ready);
    ringbusout->control_ready = 1;
    this->outs.insert(std::make_pair("ringbusout", ringbusout));

    SerialParser* eth_serial = new SerialParser();
    eth_serial->line_in = &(top->snap_eth_io_uart_rxd);
    eth_serial->line_out = &(top->snap_eth_io_uart_txd);
    eth_serial->fpga = "eth";
    eth_serial->enable = false;
    this->uarts.insert(std::make_pair("eth", eth_serial));

    auto _read_vmem = [&](const std::string a, const unsigned b, const unsigned c) { return readVmem(a,b,c);};
    auto _read_imem = [&](const std::string a, const unsigned b, const unsigned c) { return readImemWords(a,b,c);};


    #ifdef TB_USE_CS31

    #ifndef TB_USE_ADC
    Port32In* cs31in = new Port32In();
    cs31in->t_data = &(top->adc_data_out);
    cs31in->t_valid = &(top->adc_data_out_valid);
    cs31in->t_ready = &(top->adc_data_out_ready);
    cs31in->valid_meter = 4;
    cs31in->valid_state = 0;
    this->ins.insert(std::make_pair("cs31in", cs31in));
    #endif

    Port32Out* cs31out = new Port32Out();
    cs31out->i_data = &(top->snap_cs31_riscv_out_data);
    cs31out->i_valid = &(top->snap_cs31_riscv_out_valid);
    cs31out->i_ready = &(top->snap_cs31_riscv_out_ready);
    cs31out->control_ready = 0; // tb does not control ready
    this->outs.insert(std::make_pair("cs31out", cs31out));

    // this->vexs.insert(std::make_pair("cs31",0));
    SerialParser* cs31_serial = new SerialParser();
    cs31_serial->line_in = &(top->snap_cs31_io_uart_rxd);
    cs31_serial->line_out = &(top->snap_cs31_io_uart_txd);
    cs31_serial->fpga = "cs31";
    cs31_serial->get_vmem_data = _read_vmem;
    cs31_serial->get_imem_data = _read_imem;
    this->uarts.insert(std::make_pair("cs31", cs31_serial));

    Port32Out* mcs31in = new Port32Out();
    mcs31in->i_data =  &(top->snap_cs31_riscv_in_data);
    mcs31in->i_valid = &(top->snap_cs31_riscv_in_valid);
    mcs31in->i_ready = &(top->snap_cs31_riscv_in_ready);
    mcs31in->control_ready = 0;
    this->monitors.insert(std::make_pair("mcs31in", mcs31in));
    #endif
        
    #ifdef TB_USE_CS11

    Port32In* cs11in = new Port32In();
    cs11in->t_data = &(top->tx_turnstile_data_in);
    cs11in->t_valid = &(top->tx_turnstile_data_valid);
    cs11in->t_ready = &(top->tx_turnstile_data_ready);
    cs11in->valid_meter = 0;
    cs11in->valid_state = 0;
    cs11in->respect_ready = true;
    this->ins.insert(std::make_pair("cs11in", cs11in));


    // m stands for "monitor"
    Port32Out* mcs11in = new Port32Out();
    mcs11in->i_data =  &(top->snap_cs11_riscv_in_data);
    mcs11in->i_valid = &(top->snap_cs11_riscv_in_valid);
    mcs11in->i_ready = &(top->snap_cs11_riscv_in_ready);
    mcs11in->control_ready = 0; // tb does not control ready
    this->monitors.insert(std::make_pair("mcs11in", mcs11in));




    Port32Out* mmapmovin = new Port32Out();
    mmapmovin->i_data =  &(top->snap_mapmov_in_data);
    mmapmovin->i_valid = &(top->snap_mapmov_in_valid);
    mmapmovin->i_ready = &(top->snap_mapmov_in_ready);
    mmapmovin->control_ready = 0; // tb does not control ready
    this->monitors.insert(std::make_pair("mmapmovin", mmapmovin));

    Port32Out* cs11out = new Port32Out();
    cs11out->i_data = &(top->snap_cs11_riscv_out_data);
    cs11out->i_valid = &(top->snap_cs11_riscv_out_valid);
    cs11out->i_ready = &(top->snap_cs11_riscv_out_ready);
    cs11out->control_ready = 0; // tb does not control ready
    this->outs.insert(std::make_pair("cs11out", cs11out));

    // this->vexs.insert(std::make_pair("cs11",&Vtb_higgs_top::Vtb_higgs_top_q_engine__pi12));
    this->vexs.insert(std::make_pair("cs11",RING_ENUM_CS11));
    SerialParser* cs11_serial = new SerialParser();
    cs11_serial->line_in = &(top->snap_cs11_io_uart_rxd);
    cs11_serial->line_out = &(top->snap_cs11_io_uart_txd);
    cs11_serial->fpga = "cs11";
    cs11_serial->get_vmem_data = _read_vmem;
    cs11_serial->get_imem_data = _read_imem;
    this->uarts.insert(std::make_pair("cs11", cs11_serial));
    #endif

    #ifdef TB_USE_CS01

    Port32Out* cs01out = new Port32Out();
#ifdef VERILATE_MULTIPLE_HIGGS
    cs01out->i_data = &(top->o_data_dac);
    cs01out->i_valid = &(top->o_data_valid_dac);
    cs01out->i_ready = &(top->i_o_ready_dac);
#else
    cs01out->i_data = &(top->snap_cs01_riscv_out_data);
    cs01out->i_valid = &(top->snap_cs01_riscv_out_valid);
    cs01out->i_ready = &(top->snap_cs01_riscv_out_ready);
#endif
    cs01out->control_ready = 0; // tb does not control ready
    this->outs.insert(std::make_pair("cs01out", cs01out));

    this->vexs.insert(std::make_pair("cs01",RING_ENUM_CS01));
    SerialParser* cs01_serial = new SerialParser();
    cs01_serial->line_in = &(top->snap_cs01_io_uart_rxd);
    cs01_serial->line_out = &(top->snap_cs01_io_uart_txd);
    cs01_serial->fpga = "cs01";
    cs01_serial->get_vmem_data = _read_vmem;
    cs01_serial->get_imem_data = _read_imem;
    this->uarts.insert(std::make_pair("cs01", cs01_serial));
    #endif

    #ifdef TB_USE_CS32

    Port32Out* cs32out = new Port32Out();
    cs32out->i_data = &(top->snap_cs32_riscv_out_data);
    cs32out->i_valid = &(top->snap_cs32_riscv_out_valid);
    cs32out->i_ready = &(top->snap_cs32_riscv_out_ready);
    cs32out->control_ready = 0; // tb does not control ready
    this->outs.insert(std::make_pair("cs32out", cs32out));

    this->vexs.insert(std::make_pair("cs32",RING_ENUM_CS32));
    SerialParser* cs32_serial = new SerialParser();
    cs32_serial->line_in = &(top->snap_cs32_io_uart_rxd);
    cs32_serial->line_out = &(top->snap_cs32_io_uart_txd);
    cs32_serial->fpga = "cs32";
    cs32_serial->get_vmem_data = _read_vmem;
    cs32_serial->get_imem_data = _read_imem;
    this->uarts.insert(std::make_pair("cs32", cs32_serial));
    #endif

    #ifdef TB_USE_CS22

    Port32Out* cs22out = new Port32Out();
    cs22out->i_data = &(top->snap_cs22_riscv_out_data);
    cs22out->i_valid = &(top->snap_cs22_riscv_out_valid);
    cs22out->i_ready = &(top->snap_cs22_riscv_out_ready);
    cs22out->control_ready = 0; // tb does not control ready
    this->outs.insert(std::make_pair("cs22out", cs22out));

    this->vexs.insert(std::make_pair("cs22",RING_ENUM_CS22));
    SerialParser* cs22_serial = new SerialParser();
    cs22_serial->line_in = &(top->snap_cs22_io_uart_rxd);
    cs22_serial->line_out = &(top->snap_cs22_io_uart_txd);
    cs22_serial->fpga = "cs22";
    cs22_serial->get_vmem_data = _read_vmem;
    cs22_serial->get_imem_data = _read_imem;
    this->uarts.insert(std::make_pair("cs22", cs22_serial));
    // if 11 is present AND 01 has no riscv, tb can drive this interface
    #ifdef CS32_NO_RISCV
    Port32In* cs22in = new Port32In();
    cs22in->t_data = &(top->inject_cs22_riscv_in_data);
    cs22in->t_valid = &(top->inject_cs22_riscv_in_valid);
    cs22in->t_ready = &(top->inject_cs22_riscv_in_ready);
    cs22in->drive_port = true;
    cs22in->respect_ready = true;
    this->ins.insert(std::make_pair("cs22in", cs22in));
    #endif

    #endif

    #ifdef TB_USE_CS21
    // 6
    Port32Out* cs21out = new Port32Out();
    cs21out->i_data = &(top->snap_cs21_riscv_out_data);
    cs21out->i_valid = &(top->snap_cs21_riscv_out_valid);
    cs21out->i_ready = &(top->snap_cs21_riscv_out_ready);
    cs21out->control_ready = 0; // tb does not control ready
    this->outs.insert(std::make_pair("cs21out", cs21out));

    this->vexs.insert(std::make_pair("cs21",RING_ENUM_CS21));
    SerialParser* cs21_serial = new SerialParser();
    cs21_serial->line_in = &(top->snap_cs21_io_uart_rxd);
    cs21_serial->line_out = &(top->snap_cs21_io_uart_txd);
    cs21_serial->fpga = "cs21";
    cs21_serial->get_vmem_data = _read_vmem;
    cs21_serial->get_imem_data = _read_imem;
    this->uarts.insert(std::make_pair("cs21", cs21_serial));

    // if 21 is present AND 22 has no riscv, tb can drive this interface
    #ifdef CS22_NO_RISCV
    Port32In* cs21in = new Port32In();
    cs21in->t_data = &(top->inject_cs21_riscv_in_data);
    cs21in->t_valid = &(top->inject_cs21_riscv_in_valid);
    cs21in->t_ready = &(top->inject_cs21_riscv_in_ready);
    cs21in->drive_port = true;
    cs21in->respect_ready = true;
    this->ins.insert(std::make_pair("cs21in", cs21in));
    #endif


    #endif

    #ifdef TB_USE_CS20
    Port32Out* cs20out = new Port32Out();
    cs20out->i_data = &(top->snap_cs20_riscv_out_data);
    cs20out->i_valid = &(top->snap_cs20_riscv_out_valid);
    cs20out->i_ready = &(top->snap_cs20_riscv_out_ready);
    cs20out->control_ready = 0; // tb does not control ready
    this->outs.insert(std::make_pair("cs20out", cs20out));

    this->vexs.insert(std::make_pair("cs20",RING_ENUM_CS20));
    SerialParser* cs20_serial = new SerialParser();
    cs20_serial->line_in = &(top->snap_cs20_io_uart_rxd);
    cs20_serial->line_out = &(top->snap_cs20_io_uart_txd);
    cs20_serial->fpga = "cs20";
    cs20_serial->get_vmem_data = _read_vmem;
    cs20_serial->get_imem_data = _read_imem;
    this->uarts.insert(std::make_pair("cs20", cs20_serial));
    #endif

    #ifdef TB_USE_CS12
    Port32Out* cs12out = new Port32Out();
    cs12out->i_data = &(top->snap_cs12_riscv_out_data);
    cs12out->i_valid = &(top->snap_cs12_riscv_out_valid);
    cs12out->i_ready = &(top->snap_cs12_riscv_out_ready);
    cs12out->control_ready = 0; // tb does not control ready
    this->outs.insert(std::make_pair("cs12out", cs12out));

    this->vexs.insert(std::make_pair("cs12",RING_ENUM_CS12));
    SerialParser* cs12_serial = new SerialParser();
    cs12_serial->line_in = &(top->snap_cs12_io_uart_rxd);
    cs12_serial->line_out = &(top->snap_cs12_io_uart_txd);
    cs12_serial->fpga = "cs12";
    cs12_serial->get_vmem_data = _read_vmem;
    cs12_serial->get_imem_data = _read_imem;
    this->uarts.insert(std::make_pair("cs12", cs12_serial));
    #endif

    #ifdef TB_USE_CS02
    Port32Out* cs02out = new Port32Out();
    cs02out->i_data = &(top->snap_cs02_riscv_out_data);
    cs02out->i_valid = &(top->snap_cs02_riscv_out_valid);
    cs02out->i_ready = &(top->snap_cs02_riscv_out_ready);
    cs02out->control_ready = 0; // tb does not control ready
    this->outs.insert(std::make_pair("cs02out", cs02out));

    this->vexs.insert(std::make_pair("cs02",RING_ENUM_CS02));
    SerialParser* cs02_serial = new SerialParser();
    cs02_serial->line_in = &(top->snap_cs02_io_uart_rxd);
    cs02_serial->line_out = &(top->snap_cs02_io_uart_txd);
    cs02_serial->fpga = "cs02";
    cs02_serial->get_vmem_data = _read_vmem;
    cs02_serial->get_imem_data = _read_imem;
    this->uarts.insert(std::make_pair("cs02", cs02_serial));
    #endif

    #ifdef ETH_USE_MEGA_WRAPPER

    Port8In* ethByteIn = new Port8In();
    ethByteIn->t_data = &(top->MAC_RX_FIFODATA);
    ethByteIn->t_valid = &(top->MAC_RX_WRITE);
    ethByteIn->t_last_byte = &(top->MAC_RX_EOF);
    ethByteIn->valid_meter = 0;
    // ethByteIn->data.resize(1);
    this->ins8.insert(std::make_pair("ethbytein", ethByteIn));


    static unsigned int  dummy_mac_data = 0;
    static unsigned char dummy_mac_valid = 0;
    static unsigned char dummy_mac_ready = 0;
    Port32Out* macout = new Port32Out();
    macout->i_data = &(dummy_mac_data);
    macout->i_valid = &(dummy_mac_valid);
    macout->i_ready = &(dummy_mac_ready);
    macout->control_ready = 0; // tb does not control ready
    this->outs.insert(std::make_pair("macout", macout));


    Port8Out* ethByteOut = new Port8Out();
    ethByteOut->i_data      = &(top->MAC_TX_FIFODATA);
    ethByteOut->i_valid     = &(top->MAC_TX_FIFOAVAIL);
    ethByteOut->i_ready     = &(top->MAC_TX_MACREAD);
    ethByteOut->i_last_byte = &(top->MAC_TX_FIFOEOF);
    ethByteOut->i_last_byte_p = 0;
    ethByteOut->control_ready = 1;
    ethByteOut->random_ready = 0;
    ethByteOut->data.resize(1);

    this->outs8.insert(std::make_pair("ethbyteout", ethByteOut));

    #endif
  }

  std::function<uint32_t(uint32_t)> readImemFunction(const std::string f) {
    //////////////////////////////  TX  //////////////////////////////
    #ifdef TB_USE_CS11
    #ifndef CS11_NO_RISCV
    if( f == "cs11" ) {
        return [&](uint32_t x){return top->tb_higgs_top->cs11_top->vex_machine_top_inst->q_engine_inst->mem->get_imem(x);};
    }
    #endif
    #endif

    #ifdef TB_USE_CS12
    #ifndef CS12_NO_RISCV
    if( f == "cs12" ) {
        return [&](uint32_t x){return top->tb_higgs_top->cs12_top->vex_machine_top_inst->q_engine_inst->mem->get_imem(x);};
    }
    #endif
    #endif

    #ifdef TB_USE_CS02
    #ifndef CS02_NO_RISCV
    if( f == "cs02" ) {
        return [&](uint32_t x){return top->tb_higgs_top->cs02_top->vex_machine_top_inst->q_engine_inst->mem->get_imem(x);};
    }
    #endif
    #endif

    #ifdef TB_USE_CS01
    #ifndef CS01_NO_RISCV
    if( f == "cs01" ) {
        return [&](uint32_t x){return top->tb_higgs_top->cs01_top->vex_machine_top_inst->q_engine_inst->mem->get_imem(x);};
    }
    #endif
    #endif


    //////////////////////////////  RX  //////////////////////////////
    #ifdef TB_USE_CS31
    #ifndef CS31_NO_RISCV
    if( f == "cs31" ) {
        return [&](uint32_t x){return top->tb_higgs_top->cs31_top->vex_machine_top_inst->q_engine_inst->mem->get_imem(x);};
    }
    #endif
    #endif

    #ifdef TB_USE_CS32
    #ifndef CS32_NO_RISCV
    if( f == "cs32" ) {
        return [&](uint32_t x){return top->tb_higgs_top->cs32_top->vex_machine_top_inst->q_engine_inst->mem->get_imem(x);};
    }
    #endif
    #endif

    #ifdef TB_USE_CS22
    #ifndef CS22_NO_RISCV
    if( f == "cs22" ) {
        return [&](uint32_t x){return top->tb_higgs_top->cs22_top->vex_machine_top_inst->q_engine_inst->mem->get_imem(x);};
    }
    #endif
    #endif

    #ifdef TB_USE_CS21
    #ifndef CS21_NO_RISCV
    if( f == "cs21" ) {
        return [&](uint32_t x){return top->tb_higgs_top->cs21_top->vex_machine_top_inst->q_engine_inst->mem->get_imem(x);};
    }
    #endif
    #endif

    #ifdef TB_USE_CS20
    #ifndef CS20_NO_RISCV
    if( f == "cs20" ) {
        return [&](uint32_t x){return top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->mem->get_imem(x);};
    }
    #endif
    #endif

    return 0;
  }


    ///
    /// Returns a std::function that is capable of injecting into imem/dmem.
    /// Will return 0 if fpga is not not included in the Makefile or not a valid 
    /// @param[in] f - the fpga as a string
    ///
    std::function<void(const uint32_t, const uint32_t)> writeImemFunction(const std::string f) {
        //////////////////////////////  TX  //////////////////////////////
        #ifdef TB_USE_CS11
        #ifndef CS11_NO_RISCV
        if( f == "cs11" ) {
            return [&](const uint32_t a, const uint32_t d){return top->tb_higgs_top->cs11_top->vex_machine_top_inst->q_engine_inst->mem->set_imem(a,d);};
        }
        #endif
        #endif

        #ifdef TB_USE_CS12
        #ifndef CS12_NO_RISCV
        if( f == "cs12" ) {
            return [&](const uint32_t a, const uint32_t d){return top->tb_higgs_top->cs12_top->vex_machine_top_inst->q_engine_inst->mem->set_imem(a,d);};
        }
        #endif
        #endif

        #ifdef TB_USE_CS02
        #ifndef CS02_NO_RISCV
        if( f == "cs02" ) {
            return [&](const uint32_t a, const uint32_t d){return top->tb_higgs_top->cs02_top->vex_machine_top_inst->q_engine_inst->mem->set_imem(a,d);};
        }
        #endif
        #endif

        #ifdef TB_USE_CS01
        #ifndef CS01_NO_RISCV
        if( f == "cs01" ) {
            return [&](const uint32_t a, const uint32_t d){return top->tb_higgs_top->cs01_top->vex_machine_top_inst->q_engine_inst->mem->set_imem(a,d);};
        }
        #endif
        #endif


        //////////////////////////////  RX  //////////////////////////////
        #ifdef TB_USE_CS31
        #ifndef CS31_NO_RISCV
        if( f == "cs31" ) {
            return [&](const uint32_t a, const uint32_t d){return top->tb_higgs_top->cs31_top->vex_machine_top_inst->q_engine_inst->mem->set_imem(a,d);};
        }
        #endif
        #endif

        #ifdef TB_USE_CS32
        #ifndef CS32_NO_RISCV
        if( f == "cs32" ) {
            return [&](const uint32_t a, const uint32_t d){return top->tb_higgs_top->cs32_top->vex_machine_top_inst->q_engine_inst->mem->set_imem(a,d);};
        }
        #endif
        #endif

        #ifdef TB_USE_CS22
        #ifndef CS22_NO_RISCV
        if( f == "cs22" ) {
            return [&](const uint32_t a, const uint32_t d){return top->tb_higgs_top->cs22_top->vex_machine_top_inst->q_engine_inst->mem->set_imem(a,d);};
        }
        #endif
        #endif

        #ifdef TB_USE_CS21
        #ifndef CS21_NO_RISCV
        if( f == "cs21" ) {
            return [&](const uint32_t a, const uint32_t d){return top->tb_higgs_top->cs21_top->vex_machine_top_inst->q_engine_inst->mem->set_imem(a,d);};
        }
        #endif
        #endif

        #ifdef TB_USE_CS20
        #ifndef CS20_NO_RISCV
        if( f == "cs20" ) {
            return [&](const uint32_t a, const uint32_t d){return top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->mem->set_imem(a,d);};
        }
        #endif
        #endif

        return 0;
    }



    ///
    /// Write a single word to imem/dmem inside riscv.
    /// @param[in] start_word - must be word addressed, not byte addressed.  To get this, take the cpu pointer and divide by 4
    void writeImemWord(const std::string f, const unsigned start_word, const uint32_t word) {
        auto a = writeImemFunction(f);
        if( a == 0 ) {
            throw std::invalid_argument( std::string("writeImemFunction() did not find fpga: ")+f );
            return;
        }

        a(start_word, word);
    }


  std::vector<uint32_t> readImemWords(const std::string f, const unsigned start_word, const unsigned length) {
    auto a = readImemFunction(f);
    if( a == 0 ) {
        throw std::invalid_argument( std::string("readImemWords() did not find fpga: ")+f );
        return {};
    }

    const unsigned end = start_word + length;

    std::vector<uint32_t> out;
    out.reserve(length);
    for(unsigned i = start_word; i < end; i++ ) {
        out.push_back(a(i));
    }
    return out;
  }


  std::vector<uint32_t> readVmem(const std::string f, const unsigned start_dma, const unsigned length) {


    //////////////////////////////  TX  //////////////////////////////
    #ifdef TB_USE_CS11
    #ifndef CS11_NO_RISCV
    if( f == "cs11" ) {
        return readVmemUnode(top->tb_higgs_top->cs11_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME, start_dma, length);
    }
    #endif
    #endif

    #ifdef TB_USE_CS12
    #ifndef CS12_NO_RISCV
    if( f == "cs12" ) {
        return readVmemUnode(top->tb_higgs_top->cs12_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME, start_dma, length);
    }
    #endif
    #endif

    #ifdef TB_USE_CS02
    #ifndef CS02_NO_RISCV
    if( f == "cs02" ) {
        return readVmemUnode(top->tb_higgs_top->cs02_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME, start_dma, length);
    }
    #endif
    #endif

    #ifdef TB_USE_CS01
    #ifndef CS01_NO_RISCV
    if( f == "cs01" ) {
        return readVmemUnode(top->tb_higgs_top->cs01_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME, start_dma, length);
    }
    #endif
    #endif


    //////////////////////////////  RX  //////////////////////////////
    #ifdef TB_USE_CS31
    #ifndef CS31_NO_RISCV
    if( f == "cs31" ) {
        return readVmemUnode(top->tb_higgs_top->cs31_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME, start_dma, length);
    }
    #endif
    #endif

    #ifdef TB_USE_CS32
    #ifndef CS32_NO_RISCV
    if( f == "cs32" ) {
        return readVmemUnode(top->tb_higgs_top->cs32_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME, start_dma, length);
    }
    #endif
    #endif

    #ifdef TB_USE_CS22
    #ifndef CS22_NO_RISCV
    if( f == "cs22" ) {
        return readVmemUnode(top->tb_higgs_top->cs22_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME, start_dma, length);
    }
    #endif
    #endif

    #ifdef TB_USE_CS21
    #ifndef CS21_NO_RISCV
    #ifndef CS21_IS_DENGINE
    if( f == "cs21" ) {
        return readVmemUnode(top->tb_higgs_top->cs21_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME, start_dma, length);
    }
    #endif
    #endif
    #endif

    #ifdef TB_USE_CS20
    #ifndef CS20_NO_RISCV
    if( f == "cs20" ) {
        return readVmemUnode(top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->piston_inst->UNODE_NAME, start_dma, length);
    }
    #endif
    #endif

    if( f == "eth" ) {
        return readVmemUnode(top->tb_higgs_top->eth_top->q_engine_inst->piston_inst->UNODE_NAME, start_dma, length);
    }

    throw std::invalid_argument( std::string("readVmem() did not find fpga: ")+f );
    return {};
}




    std::vector<std::string> GetEnabledFPGAS(void) {
        std::vector<std::string> r;


        #ifdef TB_USE_CS11
        #ifndef CS11_NO_RISCV
        r.push_back("cs11");
        #endif
        #endif

        #ifdef TB_USE_CS12
        #ifndef CS12_NO_RISCV
        r.push_back("cs12");
        #endif
        #endif

        #ifdef TB_USE_CS02
        #ifndef CS02_NO_RISCV
        r.push_back("cs02");
        #endif
        #endif

        #ifdef TB_USE_CS01
        #ifndef CS01_NO_RISCV
        r.push_back("cs01");
        #endif
        #endif


        //////////////////////////////  RX  //////////////////////////////
        #ifdef TB_USE_CS31
        #ifndef CS31_NO_RISCV
        r.push_back("cs31");
        #endif
        #endif

        #ifdef TB_USE_CS32
        #ifndef CS32_NO_RISCV
        r.push_back("cs32");
        #endif
        #endif

        #ifdef TB_USE_CS22
        #ifndef CS22_NO_RISCV
        r.push_back("cs22");
        #endif
        #endif

        #ifdef TB_USE_CS21
        #ifndef CS21_NO_RISCV
        r.push_back("cs21");
        #endif
        #endif

        #ifdef TB_USE_CS20
        #ifndef CS20_NO_RISCV
        r.push_back("cs20");
        #endif
        #endif

        return r;
    }



// template <class T>
bool getRingbusDropped(uint32_t as_int) {
    bool v0;
    switch(as_int) {
#ifdef TB_USE_CS20
#ifndef CS20_QENGINE_LITE
      case RING_ENUM_CS20:
          v0 = top->tb_higgs_top->cs20_top->vex_machine_top_inst->q_engine_inst->ring_bus_inst->get_o_rd_of();
          return v0;
          break;
#endif
#endif
// #ifdef TB_USE_CS10
//       case RING_ENUM_CS10:
//           return top->tb_higgs_top->cs10_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
//           break;
// #endif
// #ifdef TB_USE_CS00
//       case RING_ENUM_CS00:
//           return top->tb_higgs_top->cs00_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
//           break;
// #endif
// #ifdef TB_USE_CS01
//       case RING_ENUM_CS01:
//           return top->tb_higgs_top->cs01_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
//           break;
// #endif
// #ifdef TB_USE_CS11
//       case RING_ENUM_CS11:
//           return top->tb_higgs_top->cs11_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
//           break;
// #endif 
// #ifdef TB_USE_CS21
//       case RING_ENUM_CS21:
//           return top->tb_higgs_top->cs21_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
//           break;
// #endif 
// #ifdef TB_USE_CS31
//       case RING_ENUM_CS31:
//           return top->tb_higgs_top->cs31_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
//           break;
// #endif 
// #ifdef TB_USE_CS30
//       case RING_ENUM_CS30:
//           return top->tb_higgs_top->cs30_top->vex_machine_top_inst->q_engine_inst->get_iBus_cmd_payload_pc();
//           break;
// #endif 
      default:
        // assert(0 && "get_ibus can't find requested fpga");
        return false;
    }
}
    

    void megaWrapperLoadRingbus(const std::vector<uint8_t>& x) {
         // cout << "rb match\n";

        // cout << HEX_STRING((int)x[47])
        // << HEX_STRING((int)x[48])
        // << HEX_STRING((int)x[49])
        // << HEX_STRING((int)x[50]) << endl;

        uint32_t val = (x[50]<<24) | (x[49] << 16) | (x[48] << 8) | x[47];

        this->outs["ringbusout"]->data.push_back(val);

        // cout << HEX32_STRING(val) << '\n';

    }

    // skips over sequence number
    void megaWrapperLoadFeedbackBus(const std::vector<uint8_t>& x) {

        uint32_t data_len = x.size() - 47;

        // cout << "fb len " << data_len << "\n";

        for(uint32_t i = 0; i < data_len; i+=4) {
            uint32_t word = (x[50+i]<<24) | (x[49+i] << 16) | (x[48+i] << 8) | x[47+i];

            this->outs["macout"]->data.push_back(word);
            // cout << "word: " << HEX32_STRING(word) << "\n";
        }

        // uint32_t word = (x[50]<<24) | (x[49] << 16) | (x[48] << 8) | x[47];

        // cout << "first: " << HEX32_STRING(fval) << "\n";
    }



    // converts packets that the test bench has received during ETH_USE_MEGA_WRAPPER
    void parseEthRxPackets(void) {
        // if( this->outs["ringbusout"]->data.size() != 0 ) {
        //     cout << "parseEthRxPackets() ran but there were already ringbus packets\n";
        // }

        const auto sz = this->outs8["ethbyteout"]->data.size();

        for(unsigned i = mega_wrapper_parse_progress; i < sz - 1; i++) {
            const auto& x = this->outs8["ethbyteout"]->data[i];
        // }
        // for(auto x : this->outs8["ethbyteout"]->data ) {

            // cout << "try parse " << i << "\n";
            if( x.size() >= 50 ) {
                // cout << "did parse " << i << "\n";

                constexpr int rb_port = 10001;
                constexpr uint8_t rb_port_0 = rb_port & 0xff;
                constexpr uint8_t rb_port_1 = (rb_port>>8) & 0xff;

                constexpr uint8_t fb_port_0 = 156;
                constexpr uint8_t fb_port_1 = 65;



                // cout << (int) x[37] << ", " << (int) x[38] << endl;
                // cout << (int) rb_port_1 << ", " << (int) rb_port_0 << "        m" << endl;
                if( x[37] == rb_port_1 && x[38] == rb_port_0 ) {
                    megaWrapperLoadRingbus(x);
                } else if( x[37] == fb_port_0 && x[38] == fb_port_1) {
                    megaWrapperLoadFeedbackBus(x);
                }

                // x[37]
                // x[38]
                mega_wrapper_parse_progress++;
            }


            // for( auto y : x ) {
            //     cout << HEX_STRING((int)y) << ",";
            // }
            // cout << endl;
        }
    }

    // still in class body
    uint64_t printPowerTemporary;

    // only call once, calling twice will double print
    void printPowerEstimates(bool print_all = false, bool head_nl = true) {
        
        auto gotPower = [](const uint64_t p, bool _head_nl) {
            bool sat = (p & 0x8000000000000000) == 0x8000000000000000;
            uint64_t pw = p & (~0xffff000000000000);
            unsigned shift = (p >> 48) & 0xff;

            double mag2 = (pw << shift);
            
            std::string msg = sat?" SATURATED!!" : "";
            if( _head_nl ) {
                cout << "\n\n";
            }
            cout << "Got power: " << HEX64_STRING(pw) << "  (" << pw << ") " << "  with a shift of " << shift << msg << "\n";
            cout << "or       : " << mag2 << "\n\n";

            double mag = sqrt(mag2);

            cout << "mag2: " << mag2  << "\n";

            double p1 = 30000.0; // an arbitrary reference

            double db = 10*std::log10(mag/p1);

            cout << "db : " << db  << " db\n";

        };

        // upper 0xff are already masked off
        // capture everything by reference, except the bools, which we capture by value
        this->registerRb([&, head_nl, print_all](const uint32_t word) {
            const uint32_t dmode = (word & 0x00ff0000)>>16;
            const uint64_t data =  word & 0x0000ffff;

            switch(dmode) {
                case 0:
                    printPowerTemporary = data;
                    break;
                case 1:
                    printPowerTemporary |= (data << 16);
                    break;
                case 2:
                    printPowerTemporary |= (data << 32);
                    break;
                case 3:
                    printPowerTemporary |= (data << 48);
                    gotPower(printPowerTemporary, head_nl);
                    break;

                default:
                    cout << "UNKNOWN POWER RESULT in rb callback\n";
                    break;
            }
            if( print_all ) {
                cout << "it is now " << this->us() << "\n";
                cout << "POWER callback with " << HEX32_STRING(data) << "\n";
            }
        }, POWER_RESULT_PCCMD);
    }


    // the default argument should be ok if us == 15
    void wakeupSelfSync(const uint32_t timer = 0x4e00) {
        send_ring(RING_ADDR_CS01, SELF_SYNC_CMD | (timer>>8) );
        send_ring(RING_ADDR_CS02, SELF_SYNC_CMD | (timer>>8) );
        send_ring(RING_ADDR_CS11, SELF_SYNC_CMD | (timer>>8) );
        send_ring(RING_ADDR_CS12, SELF_SYNC_CMD | (timer>>8) );
        send_ring(RING_ADDR_CS20, SELF_SYNC_CMD | (timer>>8) );
        //  t->send_ring(RING_ADDR_CS21, SELF_SYNC_CMD | (timer>>8) ); // d-engine
        send_ring(RING_ADDR_CS22, SELF_SYNC_CMD | (timer>>8) );
        send_ring(RING_ADDR_CS31, SELF_SYNC_CMD | (timer>>8) );
        send_ring(RING_ADDR_CS32, SELF_SYNC_CMD | (timer>>8) );
    }

    void setCookedDataMode(const uint32_t mode = 2) {
        send_ring(RING_ADDR_RX_0, COOKED_DATA_TYPE_CMD | mode);
        send_ring(RING_ADDR_RX_1, COOKED_DATA_TYPE_CMD | mode);
        send_ring(RING_ADDR_RX_2, COOKED_DATA_TYPE_CMD | mode);
        send_ring(RING_ADDR_RX_4, COOKED_DATA_TYPE_CMD | mode);
        send_ring(RING_ADDR_TX_0, COOKED_DATA_TYPE_CMD | mode);
        send_ring(RING_ADDR_TX_1, COOKED_DATA_TYPE_CMD | mode);
        send_ring(RING_ADDR_TX_2, COOKED_DATA_TYPE_CMD | mode);
    }


    ///
    /// Inject into dmem at the location of pass_fail_0
    /// riscv code should include "tb_inject_mem.h" and call get_random_seed()
    ///
    void InjectTestSeed(const uint32_t seed) {
        const uint32_t pf0_byte = (uint64_t)(void*)pass_fail_0;
        const uint32_t pf0_word = pf0_byte / 4;

        // cout << "got seed " << seed << " and " <<  pf0_word << "\n";

        for(const auto fpga : enabled_fpgas) {
            this->writeImemWord(fpga, pf0_word, seed);
        }
    }


    ///
    /// If you need shared random pulls, do them before calling this
    /// Call this if you need different random pulls per worker-fork
    /// If so, call this, and then call rand() afterwards
    void ReseedAfterFork(const uint32_t w_id) {

        // burn a different number of rand per worker
        for(uint32_t i = 0; i < w_id; i++) {
            rand();
        }

        // self-seed so that workers are now on different seeds
        srand(rand());
    }


}; // class HiggsHelper


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

// imem and dmem together aka scalar mem
// template <class T>
void hexprint_scalarmem(std::function<uint32_t(uint32_t)> inspect, std::string msg, unsigned int start = 0, unsigned int len = 999999, bool print_addr = true) {
    constexpr unsigned imem_words = 8192u;

    unsigned end = min(imem_words*4, len);
    end /= 4;
    start /= 4;

    for(unsigned i = start; i < end; i++) {
        auto imod = i % 16;
        auto memword = inspect(i);

        if( print_addr && imod == 0 ) {
            cout << "0x" << HEX32_STRING(i*4) << ": ";
        }

        cout << HEX32_STRING(memword) << " ";
        if( imod == 15) {
            cout << "\n";
        }
    }
}


template <class T>
void hexdump_vmem(T * node, std::string msg, unsigned int start, unsigned int len, bool bump = true) {
    using namespace std;
    unsigned int vmem_start = start/NSLICES;
    cout << msg << endl;
    cout << "hexdump... " << endl;
    cout << "start : 0x" << HEX32_STRING(start) << endl;
    cout << "length: 0x" << HEX32_STRING(len) << endl;
    auto bump_val = 0;
    if(bump) {
        bump_val = 0x40000;
    }

    for (auto i = vmem_start; i < vmem_start + len; i++){
        cout << "0x" << HEX32_STRING(((i*NSLICES)<<2)+bump_val) << ": ";
        cout << HEX32_STRING( node->mem_slice_0->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_1->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_2->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_3->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_4->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_5->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_6->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_7->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_8->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_9->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_10->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_11->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_12->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_13->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_14->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_15->dpram_inst->rtn_mem(i) );
        cout << endl;
    }
    cout << endl;
}

template <class T>
void hexdump_T(T * node, std::string msg, unsigned int start, unsigned int rows) {
    unsigned int vmem_start = start/NSLICES;
    using namespace std;
    cout << msg << endl;
    cout << "hexdump... " << endl;
    cout << "start : 0x" << HEX32_STRING(start) << endl;
    cout << "rowsgth: 0x" << HEX32_STRING(rows) << endl;
    for (auto i = vmem_start; i < vmem_start + rows; i++){
        cout << "0x" << HEX32_STRING(i*NSLICES) << ": ";
        cout << HEX32_STRING( node->mem_slice_0->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_1->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_2->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_3->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_4->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_5->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_6->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_7->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_8->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_9->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_10->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_11->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_12->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_13->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_14->dpram_inst->rtn_mem(i) ) << " ";
        cout << HEX32_STRING( node->mem_slice_15->dpram_inst->rtn_mem(i) ) << " ";
        cout << endl;
    }
    cout << endl;
}



template <class T>
void file_dump_T(T * node, const std::string filename) {
    std::cout << "opening " << filename << std::endl;
    std::ofstream outFile(filename);

    if (outFile.is_open()){
        for(unsigned int i = 0; i < 4096; i++){
            for(unsigned int j = 0; j < NSLICES; j++){
                outFile << HEX32_STRING(vmem_T<T>(node, i*NSLICES | j)) << " ";
            }
            outFile << std::endl;
        }
        outFile.close();
    }

}






// returns a std vector representing a udp packet
std::vector<uint32_t> ringbus_udp_packet(uint8_t ttl, uint32_t data) {
    std::vector<uint32_t> out;
    out.push_back((uint32_t)ttl);
    out.push_back(data);

    return out;
}

/////////
// does an in-place modify of vector (could be a macro)
//
//
// x starts out as:
// 0
// 1
// 2
// 3
// 4
// 5
// 6
// 7
// 8
// 9
// 10
// 11
//  
//   cut_vector(x, 2, 3);
//
//
// 0
// 1
// 5
// 6
// 7
// 8
// 9
// 10
// 11
//
//
void cut_vector(std::vector<uint32_t> &x, uint32_t start, uint32_t length) {
        x.erase(x.begin()+start, x.begin()+start+length);
}


std::vector<uint32_t> counter_stream(unsigned int pull, unsigned reset = 0) {
  std::vector<uint32_t> out;
  static unsigned int start = 0;
  if(reset) {
    start = 0;
  }
  for(unsigned int i = 0; i < pull; i++) {
    // cout << start << endl;
    out.push_back(start++);
  }
  return out;
}

///
/// In the macro TB_START_FILENAME we would like to accept both std::string and char
/// this overload is required to make that happen
///
void _tfp_write_file_overload(VerilatedVcdC* const _tfp, const char* const filename) {
    _tfp->open(filename);
}

///
/// In the macro TB_START_FILENAME we would like to accept both std::string and char
/// this overload is required to make that happen
///
void _tfp_write_file_overload(VerilatedVcdC* const _tfp, const std::string& filename) {
    _tfp->open(filename.c_str());
}



#define TB_START_PRE() \
(void)argc;(void)argv;(void)env; \
Verilated::commandArgs(argc, argv); /*Pass arguments so Verilated code can see them, e.g. $value$plusargs*/ \
Verilated::debug(0); /* Set debug level, 0 is off, 9 is highest presently used */ \
Verilated::randReset(2);

#define TB_START_FILENAME(filename) \
{\
const char* const flag = Verilated::commandArgsPlusMatch("trace"); \
if (flag && 0==strcmp(flag, "+trace")) { \
    Verilated::traceEverOn(true);  \
    cout << "Enabling waves into " << (filename) << "...\n\n"; \
    tfp = new VerilatedVcdC; \
    top->trace(tfp, 99); \
    \
    _tfp_write_file_overload(tfp, (filename)); \
} else { \
    cout << "WILL NOT WRITE .vcd WAVE FILE\n"; \
    cout << "  \"make show\" will be stale\n\n"; \
} \
}

#define STANDARD_TB_START() \
(void)argc;(void)argv;(void)env; \
Verilated::commandArgs(argc, argv); /*Pass arguments so Verilated code can see them, e.g. $value$plusargs*/ \
Verilated::debug(0); /* Set debug level, 0 is off, 9 is highest presently used */ \
Verilated::randReset(2); \
TB_START_FILENAME("wave_dump.vcd");



// pass a std vector, who to send it to, when to send, and this returns a lambda
// call the lambda once per every us and this will send ringbus on time.
template <class T>
std::function<void(unsigned int)> meteredRingbusSendUni(HiggsHelper<T>* t, const std::vector<uint32_t> vals, unsigned int start, uint32_t who, unsigned int usPerSend = 10) {
    std::function<void(unsigned int)> cb = [vals,t,who,start,usPerSend](unsigned int us) {
        int progress = (int)us - (int)start;
        if( progress < 0 ) {
            // cout << "skipping at time " << us << endl;
            return;
        }
        if( progress % usPerSend == 0 ) {
            unsigned idx = progress / usPerSend;
            if( idx < vals.size() ) {
                // cout << "in lambda " << vals[idx] << " at " << us << " idx " << idx << endl;
                t->inStreamAppend(
                    "ringbusin",
                    ringbus_udp_packet(who, vals[idx])
                    );
            } // if
        } // if mod
    }; // lambda
    return cb;
} // meteredRingbusSendUni


std::vector<uint8_t> wrapEthPacket(const std::vector<uint32_t> &d, const uint16_t port) {
    constexpr uint16_t header_length = 28;
    auto header = file_read_hex_stream("../../data/udp_packet_template.hexstream");

    uint16_t len = (d.size()*4) + header_length;

    header[16] = (len>>8) & 0xff;
    header[17] = len & 0xff;

    header[0x22] = (port>>8) & 0xff;
    header[0x23] = port & 0xff;

    // named header but this is the header+body now
    for( auto x : d ) {
        header.push_back(x&0xff);
        header.push_back((x>>8)&0xff);
        header.push_back((x>>16)&0xff);
        header.push_back((x>>24)&0xff);
        // cout << HEX_STRING(int(x)) << endl;
    }

    return header;
}

std::vector<std::vector<uint8_t>> wrapEthPacketMulti(const std::vector<uint32_t> &d, const uint16_t port) {

    std::size_t const max_size = 367;

    std::size_t sent_a = 0;
    std::size_t sent_b = 0;

    bool finished = false;

    std::vector<std::vector<uint8_t>> out;

    while(!finished) {
        size_t this_size = max_size;

        sent_b = sent_a + this_size;

        // cout << "sent_a " << sent_a << " sent_b " << sent_b << endl;

        if(sent_b >= (d.size()) ) {
            sent_b = d.size();
            finished = true;
        }

        std::vector<uint32_t> split_a(d.begin() + sent_a, d.begin() + sent_b);

        auto one = wrapEthPacket(split_a, port);

        out.push_back(one);

        sent_a = sent_b;
    }

    return out;
}




std::vector<uint8_t> wrapEthFeedbackPacket(const std::vector<uint32_t> &d) {
    return wrapEthPacket(d, 30000);
}

std::vector<std::vector<uint8_t>> wrapEthFeedbackPacketMulti(const std::vector<uint32_t> &d) {
    return wrapEthPacketMulti(d, 30000);
}

std::vector<uint8_t> wrapEthRingbusPacket(const std::vector<uint32_t> &d) {
    return wrapEthPacket(d, 20000);
}



// position sensative hash out of crappy std::hash
size_t hashVector32(const std::vector<uint32_t> &d) {
    size_t val = 0;
    for(auto w : d ) {
      val += std::hash<std::string>{}(std::to_string(w+val));
    }
    return val;
}


uint32_t _fb_hash_function(const std::vector<uint32_t>& x) {

    uint32_t seed = 0xda27f30c;
    // uint32_t a = 4;
    return xorshift32(seed, x.data(), x.size());
}

uint32_t _feedback_hash(const feedback_frame_t *v) {

    return feedback_hash(v, &_fb_hash_function);
    // auto hf = [&]() {
    //     _dsp->setPartnerTDMA(a,b,c);
    // });



    // xorshift32(seed, )
    // return 4;
}


// uint32_t hashFbVector()




#endif
