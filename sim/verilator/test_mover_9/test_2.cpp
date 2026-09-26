#define DMA_OUT_TRUNK (16)
#define DMA_OUT_CHUNK (1024)
#define FRAME_COUNTER_INDEX (6)

void send_coarse_sync(HiggsHelper<top_t>* t, uint32_t frame_count);

int test2(int argc, char** argv, char** env) {
    STANDARD_TB_START();
    HiggsHelper<top_t>* t = new HiggsHelper<top_t>(top,&main_time,tfp);
    srand(320948);
    preReset(top);
    t->reset(40);
    postReset(top);

    std::vector<uint32_t> &source = t->outs["cs31out"]->data;
    std::vector<uint32_t> qam16_input = \
            file_read_hex("../../data/cs10_out_qam16_rotated.hex");
    std::vector<uint32_t> zrs;
    uint32_t frame_iteration;
    uint32_t fails = 0;
    uint32_t expected_frame_counter = 0;
    uint32_t coarse_sync_t0 = 71;
    uint32_t coarse_sync_t1 = 358;
    uint32_t coarse_sync_num_t0 = 20;
    uint32_t coarse_sync_num_t1 = 15;
    uint32_t coarse_sync_t0_index = 7;
    uint32_t coarse_sync_t1_index = 13;
    uint32_t sample_of_last_full_frame = \
            qam16_input.size() - (qam16_input.size()%(1280));
    int us = 600;

    qam16_input.resize(sample_of_last_full_frame);

    std::cout << "QAM 16 Input Size: " << qam16_input.size() << "\n";


    zrs.resize(802-1);
    t->inStreamAppend("cs31in", zrs);
    t->inStreamAppend("cs31in", qam16_input);

    for(unsigned int i = 0; i < us; i++) {
        if (i == coarse_sync_t0) {
            send_coarse_sync(t, coarse_sync_num_t0);
        }
        if (i == coarse_sync_t1) {
            send_coarse_sync(t, coarse_sync_num_t1);
        }
        t->tick(500);
    }
    frame_iteration = source.size() / DMA_OUT_CHUNK;
    for(uint32_t i = 0; i < frame_iteration; i++) {
        const uint32_t frame_index = DMA_OUT_CHUNK + FRAME_COUNTER_INDEX + \
                               (i * (DMA_OUT_CHUNK + DMA_OUT_TRUNK));
        uint32_t frame_counter = source[frame_index];

        if (i == coarse_sync_t0_index) {
            expected_frame_counter += (coarse_sync_num_t0 * 4 / 5);
        }
        if (i == coarse_sync_t1_index) {
            expected_frame_counter += (coarse_sync_num_t1 * 4 / 5);
        }
        expected_frame_counter++;
        std::cout << "Frame Counter Value: " << frame_counter
                  << " Expected: " << expected_frame_counter << "\n";
        if (frame_counter != expected_frame_counter) fails++;
    }

    t->allStreamDump();

    // Final model cleanup
    top->final();

    // Close trace if opened
    if (tfp) {tfp->close();}

    assert(fails==0);
    std::cout << "All Test Passed\n";
    // Destroy model
    delete top; top = NULL;
    exit(0);
}

void send_coarse_sync(HiggsHelper<top_t>* t, uint32_t frame_count) {
    std::cout << "Sending coarse sync ringbus command\n";
    t->send_ring(RING_ADDR_CS31, SYNCHRONIZATION_CMD|frame_count);
}