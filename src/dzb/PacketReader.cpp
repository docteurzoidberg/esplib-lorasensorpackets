#include <dzb/PacketReader.h>

namespace dzb {

PacketReader::PacketReader(Packet const& packet) : packets(parse_packet(packet.get_data().first, packet.get_data().second)) {}

bool PacketReader::has_value(PacketType type, std::size_t ordinal) {
    return get_packet_of_type(packets, type, ordinal);
}


}