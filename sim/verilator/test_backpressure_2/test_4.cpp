#include <fstream>
#include <stdlib.h>

#define OUTPUT_SIZE_DIFF (5000)
#define VECTOR_INITIAL_VALUE (0x2)

void test_output(std::vector<uint32_t> &cs22_dma_output,
                 std::vector<uint32_t> &cs21_dma_output);

int test4(int argc, char** argv, char** env) {
    STANDARD_TB_START();
    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 
    preReset(top);
    t->reset(40);
    postReset(top);

    int simulation_time_us = 1600;
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

        if( !length_found ) {
            if( t->outs["cs21out"]->data.size() >= expected_length ) {
                length_found = true;
                simulation_time_us = i+50; // run for another 50 us nomatter what
            }
        }

        // cout << t->outs["cs21out"]->data.size() << "\n";
    }

    t->print_ringbus_out();
    t->allStreamDump();
    cs22_dma_output = t->outs["cs22out"]->data;//file_read_hex(cs22_filename);
    cs21_dma_output = t->outs["cs21out"]->data;//file_read_hex(cs21_filename);
    test_output(cs22_dma_output, cs21_dma_output);

    top->final();

    if (tfp) { tfp->close(); }

    delete top; top = NULL;
    exit(0);
}

void test_output(std::vector<uint32_t> &cs22_dma_output,
                 std::vector<uint32_t> &cs21_dma_output) {
    std::size_t output_diff;
    std::size_t n;
    assert(cs22_dma_output.size() == 1024*8);
    assert(cs21_dma_output.size() == 1024*8);
    if (cs22_dma_output.size() - cs21_dma_output.size()) {
        output_diff = cs22_dma_output.size() - cs21_dma_output.size();
        n = cs21_dma_output.size();
    } else {
        output_diff = cs21_dma_output.size() - cs22_dma_output.size();
        n = cs22_dma_output.size();
    }

    std::cout << "The difference in output between CS22 and CS21 is "
              << output_diff << "\n";
    assert(output_diff < OUTPUT_SIZE_DIFF);

    bool pass = true;

    for (std::size_t i = 0; i < n; i++) {
        if( (cs22_dma_output[i] + VECTOR_INITIAL_VALUE) != cs21_dma_output[i] ) {
            pass = false;
            break;
        }
    }

    if( !pass ) {
        for (std::size_t i = 0; i < n; i++) {
            std::cout << " i: " << i
                      << " First value: "
                      << cs22_dma_output[i] + VECTOR_INITIAL_VALUE
                      << " Second value: "
                      << cs21_dma_output[i]
                      << " Bool: "
                      << (cs22_dma_output[i] + VECTOR_INITIAL_VALUE == cs21_dma_output[i])
                      << "\n";
            assert((cs22_dma_output[i] + VECTOR_INITIAL_VALUE) == cs21_dma_output[i]);
        }
    }

    cout << "All values are correct\n";
}