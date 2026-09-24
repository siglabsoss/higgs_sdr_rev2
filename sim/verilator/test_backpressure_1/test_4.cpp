#include <fstream>
#include <stdlib.h>

#define OUTPUT_SIZE_DIFF (5000)
#define VECTOR_INITIAL_VALUE (0x2)

void get_file_data(std::string filename, std::vector<uint32_t> &data);

void test_output(std::vector<uint32_t> &cs22_dma_output,
                 std::vector<uint32_t> &cs21_dma_output);

int test4(int argc, char** argv, char** env) {
    STANDARD_TB_START();
    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp); 
    preReset(top);
    t->reset(40);
    postReset(top);

    int simulation_time_us = 600;
    uint32_t pull_one = rand() & 0xffffff;
    std::string cs22_filename = "cs22_out.hex";
    std::string cs21_filename = "cs21_out.hex";
    std::vector<uint32_t> cs22_dma_output;
    std::vector<uint32_t> cs21_dma_output;

    for(unsigned int i = 0; i < simulation_time_us; i++) {
        t->tick(500);
        if (i == 5) {
            t->send_ring(RING_ADDR_CS20, SEED_RANDOM_CMD | pull_one);
        }
    }

    t->print_ringbus_out();
    t->allStreamDump();
    get_file_data(cs22_filename, cs22_dma_output);
    get_file_data(cs21_filename, cs21_dma_output);
    test_output(cs22_dma_output, cs21_dma_output);

    top->final();

    if (tfp) { tfp->close(); }

    delete top; top = NULL;
    exit(0);
}

void get_file_data(std::string filename, std::vector<uint32_t> &data) {
    std::string data_value;
    std::ifstream input_file(filename);

    while(input_file >> data_value) {
        data.push_back(std::stoul(data_value, NULL, 16));
    }
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

    for (std::size_t i = 0; i < n; i++) {
        std::cout << " i: " << i
                  << " First value: "
                  << cs22_dma_output[i] + VECTOR_INITIAL_VALUE
                  << " Second value: "
                  << cs21_dma_output[i]
                  << " Bool: "
                  << (cs22_dma_output[i] + VECTOR_INITIAL_VALUE == cs21_dma_output[i])
                  << "\n";
        assert(cs22_dma_output[i] + VECTOR_INITIAL_VALUE == cs21_dma_output[i]);
    }
}