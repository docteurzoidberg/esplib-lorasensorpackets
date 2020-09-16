#include <dzb/CRC.h>

namespace dzb {

std::array<uint8_t, 256> static_crc_table;

void init_crc_table(uint8_t polynomial) {
    for(int i = 0; i < static_crc_table.size(); ++i) {
        int t = i;
        for(int j = 0; j < sizeof(uint8_t); ++j) {
            if ((t & 0x80) != 0) {
                t = (t << 1) ^ polynomial;
            } else {
                t <<= 1;
            }
        }

        static_crc_table[i] = static_cast<uint8_t>(t);
    }
}

uint8_t compute_crc(uint8_t const* buffer, std::size_t size, uint8_t initial_crc) {
    auto crc = initial_crc;
    
    for(std::size_t i = 0; i < size; ++i) {
        crc = static_crc_table[crc ^ buffer[i]];
    }

    return crc;
}


}