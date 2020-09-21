#include <dzb/PacketWriter.h>

namespace dzb {

PacketWriter::PacketWriter(std::array<uint8_t, 2> id, std::size_t size_threshold, std::function<void(Packet)> on_packet_emit) 
    : id(id)
    , size_threshold(size_threshold)
    , on_packet_emit(on_packet_emit) {}

Packet PacketWriter::make_packet() {
    auto packet = Packet::construct_with_id(id, std::move(buffer));
    buffer.clear();
    return packet;
}

void PacketWriter::flush() {
    if(buffer.size() > 0) {
        auto packet = make_packet();
        if (on_packet_emit) {
            on_packet_emit(std::move(packet));
        }
    }
}


}