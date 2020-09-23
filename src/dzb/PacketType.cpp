#include <dzb/PacketType.h>

namespace dzb {

std::array<PacketTypeMeta, 256> static_packet_type_meta;

PacketTypeMeta::PacketTypeMeta([[maybe_unused]] char const* name, std::size_t size_of_data)
    :
#ifndef DZB_PACKET_TYPE_DONT_STORE_NAME
    name(name),
#endif
    size_of_data(size_of_data), known(true) {}

void init_packet_type_meta() {
#define ADD_TYPE(type, data_type) static_packet_type_meta[static_cast<uint8_t>(PacketType::type)] = PacketTypeMeta(#type, sizeof(data_type))

    ADD_TYPE(TEMPERATURE,   float);
    ADD_TYPE(HUMIDITY,      uint8_t);
    ADD_TYPE(PRESSURE,      uint16_t);
    ADD_TYPE(LUMINOSITY,    uint8_t);

    ADD_TYPE(BATT_VOLTAGE,  float);
    ADD_TYPE(BATT_PERCENT,  uint8_t);
    ADD_TYPE(BATT_LOW,      bool);
    ADD_TYPE(BATT_FULL,     bool);
    ADD_TYPE(BATT_CHARGING, bool);

    ADD_TYPE(SWITCH,        bool);
    ADD_TYPE(PRESENCE,      bool);

    ADD_TYPE(ALARM,         bool);

    ADD_TYPE(GPIO_D0, bool);
    ADD_TYPE(GPIO_D1, bool);
    ADD_TYPE(GPIO_D2, bool);
    ADD_TYPE(GPIO_D3, bool);
    ADD_TYPE(GPIO_D4, bool);
    ADD_TYPE(GPIO_D5, bool);
    ADD_TYPE(GPIO_D6, bool);
    ADD_TYPE(GPIO_D7, bool);
    ADD_TYPE(GPIO_D8, bool);
    ADD_TYPE(GPIO_D9, bool);

    ADD_TYPE(GPIO_A0, uint16_t);
    ADD_TYPE(GPIO_A1, uint16_t);
    ADD_TYPE(GPIO_A2, uint16_t);
    ADD_TYPE(GPIO_A3, uint16_t);

#undef ADD_TYPE
}

bool is_type_known(PacketType type) {
    return static_packet_type_meta[static_cast<uint8_t>(type)].known;
}

std::size_t size_of_packet(PacketType type) {
    return static_packet_type_meta[static_cast<uint8_t>(type)].size_of_data;
}

char const* name_of_type(PacketType type) {
#ifndef DZB_PACKET_TYPE_DONT_STORE_NAME
    return static_packet_type_meta[static_cast<uint8_t>(type)].name;
#else
    return "<unknown>";
#endif
}

}