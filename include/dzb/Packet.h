#pragma once

#include <vector>

#include <dzb/CRC.h>

namespace dzb {

using id_t = std::array<uint8_t, 2>;

struct Packet {
private:

    Packet(id_t id, std::vector<uint8_t> data_buffer);
    Packet(uint8_t const* packet_buffer, std::size_t size);

    uint8_t compute_crc() const;
    void write_crc();
    void write_header(id_t id);

public:

    // Construct a packet from a buffer containing the header and the data
    static Packet deconstruct(uint8_t const* packet_buffer, std::size_t size);

    // Construct a packet from an id and some data
    // Compute the crc and assign it in the header
    static Packet construct_with_id(id_t id, std::vector<uint8_t> data_buffer);

    static constexpr std::size_t CRC_INDEX = 2;
    static constexpr std::size_t HEADER_SIZE = sizeof(uint8_t) * 4;


    id_t get_id() const;

    uint8_t get_ack() const;
    uint8_t get_crc() const;
    std::pair<uint8_t const*, std::size_t> get_data() const;

    bool is_from_device(char const (&id)[3]) const;
    bool is_crc_valid() const;

    std::vector<uint8_t> buffer;
};

}