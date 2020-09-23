#include <dzb/Packet.h>

#include <dzb/Serialization.h>

namespace dzb {

Packet::Packet(std::array<uint8_t, 2> id, std::vector<uint8_t> data_buffer) {
    buffer.reserve(HEADER_SIZE + data_buffer.size());
    write_header(id);

    std::copy(std::begin(data_buffer), std::end(data_buffer), std::back_inserter(buffer));
}

Packet::Packet(uint8_t const* packet_buffer, std::size_t size) : buffer(packet_buffer, packet_buffer + size) {}

Packet Packet::deconstruct(uint8_t const* packet_buffer, std::size_t size) {
    return Packet(packet_buffer, size);
}

Packet Packet::construct_with_id(std::array<uint8_t, 2> id, std::vector<uint8_t> data_buffer) {
    Packet packet(id, std::move(data_buffer));
    packet.write_crc();
    return packet;
}

void Packet::write_header(std::array<uint8_t, 2> id) {
    buffer.resize(sizeof(uint8_t) * 4);
    auto internal_buffer = buffer.data();
    internal_buffer = serialize<uint8_t>(internal_buffer, id[0]);
    internal_buffer = serialize<uint8_t>(internal_buffer, id[1]);
    internal_buffer = serialize<uint8_t>(internal_buffer, 0); // crc
    internal_buffer = serialize<uint8_t>(internal_buffer, 0); // ack
}

uint8_t Packet::compute_crc() const {
    // Compute crc of the whole packet without the crc
    auto crc_header = ::dzb::compute_crc(buffer.data(), CRC_INDEX);
    auto crc = ::dzb::compute_crc(buffer.data() + CRC_INDEX + 1, buffer.size() - CRC_INDEX - 1, crc_header);

    return crc;
}

void Packet::write_crc() {
    auto crc = compute_crc();
    buffer[CRC_INDEX] = crc;
}

std::array<uint8_t, 2> Packet::get_id() const {
    return { buffer[0], buffer[1] };
}

uint8_t Packet::get_ack() const {
    return buffer[3];
}

uint8_t Packet::get_crc() const {
    return buffer[CRC_INDEX];
}

bool Packet::is_crc_valid() const {
    return compute_crc() == get_crc();
}

bool Packet::is_from_device(char const (&id)[2]) const {
  return get_id()[0] == id[0] && get_id()[1] == id[1];
}

std::pair<uint8_t const*, std::size_t> Packet::get_data() const {
    return { &buffer[4], buffer.size() - HEADER_SIZE };
}


}