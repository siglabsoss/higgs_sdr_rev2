int test1(int argc, char** argv, char** env) {



    std::vector<uint32_t> v0;
    v0 = file_read_hex("schedule_000.hex");

    std::vector<uint32_t> v1;
    v1 = file_read_hex("schedule_001.hex");


    unsigned min_hash = std::min(v0.size(), v1.size());

    cout << "Got      v0 size: " << v0.size() << "\n";
    cout << "Expected v1 size: " << v1.size() << "\n";

    std::map<uint32_t, uint32_t> totals;

    for(unsigned i = 0; i < min_hash; i++) {
        auto w0 = v0[i];
        auto w1 = v1[i];

        auto delta = w1 - w0;

        cout << HEX16_STRING(i) << " " << HEX32_STRING(delta) <<  "\n";

        if( totals.find(delta) != totals.end() ) {
            // cout << "found\n";
            totals[delta]++;
        } else {
            // cout << "not found\n";
            totals[delta] = 1;
        }

        // cout << HEX16_STRING(i) << " " << HEX32_STRING(w0) << " " << HEX32_STRING(w1) << "\n";
    }

    for(const auto w : totals) {
        cout << "total: " << std::get<0>(w) << ", " << std::get<1>(w)  << "\n";
    }


    exit(0);
}
