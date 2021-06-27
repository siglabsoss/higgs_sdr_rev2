#ifndef __HIGGS_HELPER__
#define __HIGGS_HELPER__

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <assert.h>
#include <fstream>
#include <algorithm>
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

#include "cpp_utils.hpp"

// #include "tb_ringbus.hpp"

// since higgs_helper is providing everything, why not grab this
using namespace std;

// forward declares
std::vector<uint32_t> ringbus_udp_packet(uint8_t ttl, uint32_t data);

#include "vector_helpers.hpp"

#define NSLICES (16)



// An input to the DUT (technically an output from the perspective of the TB)
class Port8In {
public:
    uint8_t *t_data;
    unsigned char *t_valid;
    unsigned char *t_last_byte;
    std::vector<uint8_t> data; // pending data
    std::vector<size_t> chunks;
    unsigned int valid_meter; // 0 for always valid, 1 for every other
    unsigned int valid_state;
    unsigned int count_up;
};

// An output form the DUT (technically an input from the perspective of the TB)
class Port8Out {
public:
    uint8_t *i_data;
    unsigned char *i_ready;
    unsigned char *i_valid;
    std::vector<uint8_t> data; // captured data
    bool control_ready; // you should probably never set this
    unsigned int random_ready; // 0 for always ready, 1 for random
};


template <class T>
class EthFrameRouterHelper {

public:
    T *top;
    uint64_t* main_time;
    VerilatedVcdC* tfp = NULL;
    unsigned char *clk;
    std::vector<Port8In> ins;
    std::vector<Port8Out> outs;
    unsigned int pc_done[3] = {0xE4, 0xE8, 0xEC};

    void (*negClock)(EthFrameRouterHelper<T>*);
    void (*posClock)(EthFrameRouterHelper<T>*);
    
    EthFrameRouterHelper(T *top, uint64_t *main_time, VerilatedVcdC *tfp) {
        this->top=top;
        this->main_time=main_time;
        this->tfp=tfp;
        clk=&(top->i_rxmac_clk);
    }

    // pass number of clock cycles to reset, must be even number and greater than 4
    void reset(unsigned count) {
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

    void handleDataNegIndex(unsigned i) {
        using namespace std;
        unsigned int meter_flag = 0;
        if( this->ins[i].valid_meter != 0 ) {
            // do meter
            if( this->ins[i].valid_state == 0 )
            {
                meter_flag = 1;
            }

            this->ins[i].valid_state = (this->ins[i].valid_state+1) % this->ins[i].valid_meter;
        } else {
            meter_flag = 1;
        }

        if( meter_flag && this->ins[i].data.size() )
            {
                *(this->ins[i].t_data) = this->ins[i].data.back();
                *(this->ins[i].t_valid) = 1;

                // cout << "comp "  << this->ins[i].chunks.front()-1 << " and " << this->ins[i].count_up << endl;
                if( this->ins[i].chunks.front()-1 == this->ins[i].count_up) {
                    // chunks
                    *(this->ins[i].t_last_byte) = 1;
                    this->ins[i].count_up = 0;
                } else {
                    *(this->ins[i].t_last_byte) = 0;
                    this->ins[i].count_up++;
                }


            } else {
                *(this->ins[i].t_valid) = 0;
                *(this->ins[i].t_last_byte) = 0;
            }
    }

    void handleDataPosIndex(unsigned i) {
        unsigned int meter_flag = 0;
        if( this->ins[i].valid_meter != 0 && this->ins[i].data.size()) {
            if( this->ins[i].valid_state == 0 )
                    {
                        meter_flag = 1;
                    }
        } else {
            meter_flag = 1;
        }    
        if(meter_flag && this->ins[i].data.size() ) {
            this->ins[i].data.pop_back();
            // cout << "count_up was " << this->ins[i].count_up << endl;
            // cout << "count_up was " << this->ins[i].chunks.front() << endl;

        }
    }

    void handleDataOutNegIndex(unsigned i) {
        if( this->outs[i].control_ready ) {

            if(this->outs[i].random_ready) {
                uint32_t pick = rand();
                bool ready = (pick % 1000) > 500;
                *(this->outs[i].i_ready) = ready;
            } else {
                *(this->outs[i].i_ready) = 1;
            }
        }
    }

    void handleDataOutPosIndex(unsigned i) {
        if( *(this->outs[i].i_ready) && *(this->outs[i].i_valid) ) {
            this->outs[i].data.push_back( *(this->outs[i].i_data) );
        }
    }



    void handleDataNeg() {
        for(auto i = 0; i < ins.size(); i++) {
            this->handleDataNegIndex(i);
        }
        for(auto i = 0; i < outs.size(); i++) {
            this->handleDataOutNegIndex(i);
        }
    }

    void handleDataPos() {
        for(auto i = 0; i < ins.size(); i++) {
            this->handleDataPosIndex(i);
        }
        for(auto i = 0; i < outs.size(); i++) {
            this->handleDataOutPosIndex(i);
        }
    }

    // because we are using data.back() and data.pop_back() to insert data into the DUT
    // we need to add it in this backwards weird way.
    void inStreamAppendPacket(unsigned index, std::vector<uint8_t> din) {
        assert(index <= ins.size());

        VEC_R_APPEND(ins[index].data, din);
        ins[index].chunks.push_back(din.size());
    }

    void print_time() {
        constexpr int increment = (100*1000); // 100 us
        constexpr int convert_us = (1000); // 100 us
        if(*main_time % increment == 0) {
            int val;
            std::cout << *main_time / convert_us << "us" << std::endl;
        }
    }

    void tick(unsigned count = 1){
        for(unsigned i = 0; i < count; i++) {
            print_time();
            (*main_time)++;
            *clk = !*clk;
            top->eval();
            if(tfp) {tfp->dump(*main_time);}
            negClock(this);
            
            *clk = !*clk;
            (*main_time)++;
            top->eval();
            if(tfp) {tfp->dump(*main_time);}
            posClock(this);
        }
    }


}; // class EthFrameRouterHelper


template <class T>
void hexdump_vmem(T * node, std::string msg, unsigned int start, unsigned int len) {
    using namespace std;
    unsigned int vmem_start = start/NSLICES;
    cout << msg << endl;
    cout << "hexdump... " << endl;
    cout << "start : 0x" << HEX32_STRING(start) << endl;
    cout << "length: 0x" << HEX32_STRING(len) << endl;
    for (auto i = vmem_start; i < vmem_start + len; i++){
        cout << "0x" << HEX32_STRING(((i*NSLICES)<<2)+0x40000) << ": ";
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
        }
}

template <class T>
std::vector<uint32_t> file_dump_T(T * node, std::string filename) {
    std::vector<uint32_t> out;
    uint32_t rtn;    
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


    return out;
}

// reads every line in a file that has 32 bit hex values (WITHOUT 0x)
// this pairs with file_dump_vec()
std::vector<uint32_t> file_read_hex(std::string filename) {
        fstream infile;
        infile.open(filename, fstream::in|fstream::out|fstream::app);

        std::vector<uint32_t> out;
        uint32_t a;
        infile >> std::hex;
        while (infile >> a)
        {
                out.push_back(a);
        }
        return out;
}

// dumps out ascii HEX values, 8 characters per line (32 bits)
void file_dump_vec(std::vector<uint32_t> din, std::string filename) {
    uint32_t rtn;
    std::cout << "opening " << filename << " for writing" << std::endl;
    std::ofstream outFile(filename);

    if (outFile.is_open()){

        for(auto it = din.begin(); it != din.end(); it++) {
            outFile << HEX32_STRING(*it) << std::endl;
        }
        outFile.close();
    }
}

void file_dump_csv(std::vector<uint32_t> din, std::string filename) {
        int real, imag;
        int16_t ureal, uimag;

        std::cout << "opening " << filename << " for writing" << std::endl;
        std::ofstream outFile(filename);

        if (outFile.is_open()){

                for(auto it = din.begin(); it != din.end(); it++) {
                        ureal = *it & 0xffff;
                        uimag = (*it>>16) & 0xffff;

                        outFile << ureal << ", " << uimag << endl;

                        // outFile << HEX32_STRING(*it) << std::endl;
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


#endif