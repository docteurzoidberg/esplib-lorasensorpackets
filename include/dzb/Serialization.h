#pragma once

#include <cstring>
#include <cstdint>
#include <vector>

#include <dzb/PacketType.h>

namespace dzb {

// Append a value into the buffer and returns the end of the buffer
template<typename T>
uint8_t* serialize(uint8_t* buffer, T value) {
  memcpy(buffer, &value, sizeof(T));
  return buffer + sizeof(T);
}

// Append a value with it's type in header into the buffer and returns the end of the buffer
template<typename T>
uint8_t* serialize_with_type(uint8_t* buffer, PacketType type, T value) {
  buffer = serialize(buffer, type);
  buffer = serialize(buffer, value);
  return buffer;
}

// Parse a value from the buffer, the buffer is modified to the first byte after the value
template<typename T>
T deserialize(uint8_t const*& buffer) {
  T value;
  memcpy(&value, buffer, sizeof(T));
  buffer += sizeof(T);
  return value;
}

// Parse a value only if the header match the type, the buffer is modified to the first byte after the value if the value has been parsed, otherwise stay the same
template<typename T>
bool deserialize_if_type(uint8_t const*& buffer, PacketType type, T& value_out) {
    uint8_t const* temp_buffer = buffer;

    auto type_in_buffer = deserialize<PacketType>(temp_buffer);

    if (type_in_buffer != type) {
        return false;
    }

    value_out = deserialize<T>(temp_buffer);
    buffer = temp_buffer;

    return true;
}

// Returns all start of packets which are valid
// i.e.: has a PacketType followed by their data
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

// Returns the (ordinal - 1) th occurence of the packet with the corresponding type
uint8_t const* get_packet_of_type(std::vector<uint8_t const*> const& packets, PacketType type, std::size_t ordinal) {
    for(auto packet : packets) {
        auto packet_type = deserialize<PacketType>(packet);
        if (packet_type == type && ordinal-- == 0) {
            return packet;
        } 
    }

    return nullptr;
}

// Same as get_packet_of_type but deserialize the packet as well
template<typename T>
bool get_packet_of_type_and_deserialize(std::vector<uint8_t const*> const& packets, PacketType type, std::size_t ordinal, T& value_out) {
    if (auto* p = get_packet_of_type(packets, type, ordinal)) {
        deserialize_if_type(p, type, value_out);
        return true;
    }

    return false;
}

}