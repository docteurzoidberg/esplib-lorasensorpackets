#include <dzb/Serialization.h>

namespace dzb {

std::vector<uint8_t const*> parse_packet(uint8_t const* packet, std::size_t size) {
    std::vector<uint8_t const*> res;

    uint8_t const* const end_packet = packet + size;
    while(packet < end_packet) {
        auto const start_current_packet = packet;
        auto const type = deserialize<PacketType>(packet);

        if (is_type_known(type)) {
            res.emplace_back(start_current_packet);
            packet += size_of_packet(type);
        } else {
            // we can't do anything past this
            // as we don't know the size of the data used for this (unknown) type
            return res;
        }
    }

    return res;
}

uint8_t const* get_packet_of_type(std::vector<uint8_t const*> const& packets, PacketType type, std::size_t ordinal) {
    for(auto packet : packets) {
        auto packet_type = deserialize<PacketType>(packet);
        if (packet_type == type && ordinal-- == 0) {
            return packet;
        } 
    }

    return nullptr;
}


}