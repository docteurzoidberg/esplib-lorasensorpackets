#pragma once

#include <array>

namespace dzb {


void init_crc_table(uint8_t polynomial = 0x9b);

uint8_t compute_crc(uint8_t const* buffer, std::size_t size, uint8_t initial_crc = 0);

}