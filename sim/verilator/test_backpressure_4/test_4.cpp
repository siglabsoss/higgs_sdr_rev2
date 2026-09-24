#include <fstream>
#include <stdlib.h>

// #define OUTPUT_SIZE_DIFF (5000)
#define VECTOR_INITIAL_VALUE (0xf0000000)

void test_last_output(
                 std::vector<uint32_t> &cs21_dma_output);

int test4(int argc, char** argv, char** env) {
    STANDARD_TB_START();
    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 
    preReset(top);
    t->reset(40);
    postReset(top);

    int simulation_time_us = 600;

    // simulation_time_us = 300;
    // simulation_time_us = 100;

    uint32_t pull_one = rand() & 0xffffff;
    uint32_t pull_two = rand() & 0xffffff;
    uint32_t pull_three = rand() & 0xffffff;
    const std::string cs22_filename = "cs22_out.hex";
    const std::string cs21_filename = "cs21_out.hex";
    const unsigned expected_length = 1024*8;
    std::vector<uint32_t> cs22_dma_output;
    std::vector<uint32_t> cs21_dma_output;

    cout << "Pull 1: " << pull_one << "\n";
    cout << "Pull 2: " << pull_two << "\n";
    cout << "Pull 3: " << pull_three << "\n";

    bool print_length = false;

    // allows us to exit early when we get the correct length
    // we set simulation_time to to a high number to allow for 
    // long random simulation conditions
    bool length_found = false;

    for(unsigned int i = 0; i < simulation_time_us; i++) {
        t->tick(500);
        if (i == 5) {
            t->send_ring(RING_ADDR_CS20, SEED_RANDOM_CMD | pull_one);
        }
        if (i == 10) {
            t->send_ring(RING_ADDR_CS22, SEED_RANDOM_CMD | pull_two);
        }
        if (i == 15) {
            t->send_ring(RING_ADDR_CS21, SEED_RANDOM_CMD | pull_three);
        }

        if( print_length ) {
            cout << "sz: " << t->outs["cs21out"]->data.size();
            if( t->outs["cs21out"]->data.size() ) {
                cout << " " << HEX32_STRING(t->outs["cs21out"]->data.back());
            }
            cout << "\n";
        }

        if( !length_found ) {
            if(   t->outs["cs21out"]->data.size() >= expected_length
               || (t->outs["cs21out"]->data.size() && ( t->outs["cs21out"]->data.back() >= (VECTOR_INITIAL_VALUE+(1024*7)-1) ))
               ) {
                length_found = true;
                simulation_time_us = i+100; // run for another 50 us nomatter what
            }
        }

        // cout << t->outs["cs21out"]->data.size() << "\n";
    }

    t->print_ringbus_out();
    t->allStreamDump();
    // cs22_dma_output = get_counter(0, (1024*8));
    cs21_dma_output = t->outs["cs21out"]->data;
    test_last_output(cs21_dma_output);

    top->final();

    if (tfp) { tfp->close(); }

    delete top; top = NULL;
    exit(0);
}

void test_last_output(std::vector<uint32_t> &cs21_dma_output) {

    std::vector<uint32_t> blocks;
    blocks.resize(8);

    // const uint32_t len = 1024*8;
    const uint32_t sval = 0xf0000000;

    for(uint32_t i = 0; i < 8; i++) {
        for(uint32_t j = 0; j < 1024; j++) {
            const uint32_t k = (i*1024)+j;
            const uint32_t expected = k+sval;

            // cout << k << "\n";
            const bool found = VECTOR_FIND(cs21_dma_output, expected);

            if( found ) {
                blocks[i]++;
            }
        }
        cout << "block: " << i << " " << blocks[i] << "\n";
    }

    bool pass = true;
    unsigned count_missing = 0;
    std::vector<unsigned> missing_index;
    for(unsigned i = 0; i < blocks.size(); i++) {
        const auto b = blocks[i];
        if( b != 0 && b != 1024) {
            cout << "Block " << i << " failed to recover during last\n";
            pass = false;
        }
        if( b == 0 ) {
            count_missing++;
            missing_index.push_back(i);
        }
    }

    if( count_missing > 3 ) {
        cout << "More than 3 block went missing: " << count_missing << "\n";
        pass = false;
    }


    for (int i = 1; i < missing_index.size(); i++) {
        if ((missing_index[i] - missing_index[i-1])  != 1)  {
            cout << "Missing blocks were not contiguous\n";
            pass = false;
        }
    }

    assert(pass);

}
