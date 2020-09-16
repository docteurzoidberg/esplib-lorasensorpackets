#pragma once

#include <vector>

#include <dzb/Serialization.h>
#include <dzb/Packet.h>

namespace dzb {


struct PacketReader {
    PacketReader(Packet const& packet);

    bool has_value(PacketType type, std::size_t ordinal);

    template<typename T>
    bool get_value(PacketType type, std::size_t ordinal, T& value_out) {
        return get_packet_of_type_and_deserialize(packets, type, ordinal, value_out);
    }

    std::vector<uint8_t const*> packets;
};


}