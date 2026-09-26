
#include "verilate_multiple.hpp"


// needed for this test, for some reason
std::string lookup_ringbus_enum(unsigned int v, bool upper = false);






void Board0Begin(HiggsHelper<top_t>* const t) {
    cout << t->prefix << "Board0Begin()\n";
}

void Board1Begin(HiggsHelper<top_t>* const t) {
    cout << t->prefix << "Board1Begin()\n";
}


int Board0Loop(HiggsHelper<top_t>* const t, const uint32_t us) {
    // cout << "Board 0: us " << us << "\n";
    return 0;
}

int Board1Loop(HiggsHelper<top_t>* const t, const uint32_t us) {
    // cout << "Board 1: us " << us << "\n";
    return 0;
}


///
/// Called once time after all us have been ticked
/// Return non 0 to fail
int Board0Final(HiggsHelper<top_t>* const t, const uint32_t us) {

    unsigned num_passed = 0;
    unsigned num_failed = 0;

    t->print_ringbus_out();
    t->allStreamDump();


    std::cout << t->prefix << "Results:\n";
    for(auto it = t->outs["ringbusout"]->data.begin();
        it != t->outs["ringbusout"]->data.end(); it++) {
        if( (*it & 0xff000000) == 0xf0000000 ) {
            const unsigned int fpga  = (*it & 0x000f0000) >> 16;
            const unsigned int error = (*it & 0x0000ffff);
            std::cout << t->prefix << "FPGA " << fpga << " " << lookup_ringbus_enum(fpga) << " reported " << HEX_STRING(error) << "\n";
            if( error == 0 ){
                std::cout << t->prefix << "   passed\n";
                if( fpga == RING_ENUM_CS31 ) {
                    num_passed++;
                }
            } else {
                std::cout << t->prefix << "   failed\n";
                num_failed++;
            }
        }
        // std::cout << "0x" << HEX_STRING(*it) << "\n";
    }



    assert2(num_passed == 1 && "Some FPGA's did not pass internally");
    assert2(num_failed == 0 && "Some FPGA's failed");

    return 0;
}

int Board1Final(HiggsHelper<top_t>* const t, const uint32_t us) {

    unsigned num_passed = 0;
    unsigned num_failed = 0;

    t->print_ringbus_out();
    t->allStreamDump();


    std::cout << t->prefix << "Results:\n";
    for(auto it = t->outs["ringbusout"]->data.begin();
        it != t->outs["ringbusout"]->data.end(); it++) {
        if( (*it & 0xff000000) == 0xf0000000 ) {
            const unsigned int fpga  = (*it & 0x000f0000) >> 16;
            const unsigned int error = (*it & 0x0000ffff);
            std::cout << t->prefix << "FPGA " << fpga << " " << lookup_ringbus_enum(fpga) << " reported " << HEX_STRING(error) << "\n";
            if( error == 0 ){
                std::cout << t->prefix << "   passed\n";
                if( fpga == RING_ENUM_CS31 ) {
                    num_passed++;
                }
            } else {
                std::cout << t->prefix << "   failed\n";
                num_failed++;
            }
        }
        // std::cout << "0x" << HEX_STRING(*it) << "\n";
    }



    assert2(num_passed == 1 && "Some FPGA's did not pass internally");
    assert2(num_failed == 0 && "Some FPGA's failed");

    return 0;
}




int test2(const int argc, char** const argv, char** const env) {


    const uint32_t runtime_us = 400;

    return VerilateMultiple(
        argc,
        argv,
        env,
        runtime_us
    );
}

