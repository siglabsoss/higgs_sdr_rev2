#include <vector>
#include <iostream>
#include <map>

std::map<uint8_t, uint32_t> REVERSE_QPSK_MAP = {{0b11, 0x5a815a81}, \
                                                {0b10, 0x5a81a57e}, \
                                                {0b01, 0xa57e5a81}, \
                                                {0b00, 0xa57ea57e}};

std::map<uint32_t, uint8_t> QPSK_MAP = {{0x5a815a81, 0b11}, \
                                        {0x5a81a57e, 0b10}, \
                                        {0xa57e5a81, 0b01}, \
                                        {0xa57ea57e, 0b00}};

void _print_packet(std::vector<uint32_t> *packet) {
    for (uint32_t i = 0; i < packet->size(); i++) {
        std::cout << "Packet: " << std::hex << packet->at(i) << std::endl;
    }
}

void data_sync(std::vector<uint32_t> *subcarrier_data,
               uint32_t sync_word,
               uint32_t subcarrier_count) {
    uint32_t first_bit;
    uint32_t second_bit;
    for (int i = 30; i >= 0; (i = i - 2)) {
        uint32_t final_word = 0;
        first_bit = (sync_word >> i) & 1;
        second_bit = (sync_word >> i + 1) & 1;
        for (uint32_t j = 0; j < 32; j++) {
            if (j < 16) {
                final_word = (final_word << 1) | first_bit;
            } else {
                final_word = (final_word << 1) | second_bit;
            }
        }
        for (uint32_t k = 0; k < subcarrier_count/16; k++) {
            subcarrier_data->push_back(final_word);
        }
    }
}

void modulate_word(std::vector<uint32_t> *modulated_data, uint32_t data) {
    uint16_t first_bit;
    uint16_t second_bit;
    uint16_t qpsk_bit;
    uint16_t lower_16_bit = data & 0xFFFF;
    uint16_t upper_16_bit = data >> 16;

    for (int i = 0; i < 16; i++) {
        first_bit = (lower_16_bit >> i) & 1;
        second_bit = (upper_16_bit >> i) & 1;
        qpsk_bit = (first_bit << 1) | second_bit;
        modulated_data->push_back(REVERSE_QPSK_MAP[qpsk_bit]);
    }
}

int main() {
    uint32_t sync_word = 0xCAFEBABE;
    uint32_t subcarrier_count = 16;
    std::vector<uint32_t> subcarrier_data;
    std::vector<uint32_t> modulated_data;

    data_sync(&subcarrier_data, sync_word, subcarrier_count);
    // modulate_word(&modulated_data, subcarrier_data[0]);
    _print_packet(&subcarrier_data);

    return 0;
}