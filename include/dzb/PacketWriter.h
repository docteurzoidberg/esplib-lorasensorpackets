#pragma once

#include <array>
#include <vector>
#include <functional>
#include <cassert>

#include <dzb/Packet.h>
#include <dzb/PacketType.h>

namespace dzb {

struct PacketWriter {
private:

    Packet make_packet();
    void flush();

    std::vector<uint8_t> buffer;

public:

    id_t id;
    std::size_t size_threshold;
    std::function<void(Packet)> on_packet_emit;


    PacketWriter(id_t id, std::size_t size_threshold, std::function<void(Packet)> on_packet_emit);

    // The size of value must not exceed the threshold
    template<typename T>
    void write(PacketType type, T const& value) {
        constexpr std::size_t size_to_write = sizeof(PacketType) + sizeof(T);
        assert(size_to_write <= size_threshold);

        std::size_t internal_buffer_offset = buffer.size();

        // reserve room for type + value
        buffer.resize(buffer.size() + size_to_write);

        // get position to write type + value
        uint8_t* internal_buffer = buffer.data() + internal_buffer_offset;

        serialize_with_type(internal_buffer, type, value);

        if (buffer.size() >= size_threshold) {
            flush();
        }
    }
};


} 