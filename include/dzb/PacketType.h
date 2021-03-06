#pragma once

#include <array>

namespace dzb {

enum class PacketType : uint8_t {

    TEMPERATURE     = 1,     // float
    HUMIDITY        = 2,     // uint8_t
    PRESSURE        = 3,     // uint16_t
    LUMINOSITY      = 4,     // uint16_t
    DISTANCE        = 5,     // uint16_t

    SWITCH          = 8,     // bool
    PRESENCE        = 9,     // bool

    BATT_VOLTAGE    = 16,    // float
    BATT_PERCENT    = 17,    // uint8_t
    BATT_LOW        = 18,    // bool
    BATT_FULL       = 19,    // bool
    BATT_CHARGING   = 20,    // bool

    ALARM           = 32,    // bool

    GPIO_D0         = 64,    // bool
    GPIO_D1         = 65,    // bool
    GPIO_D2         = 66,    // bool
    GPIO_D3         = 67,    // bool
    GPIO_D4         = 68,    // bool
    GPIO_D5         = 69,    // bool
    GPIO_D6         = 70,    // bool
    GPIO_D7         = 71,    // bool
    GPIO_D8         = 72,    // bool
    GPIO_D9         = 73,    // bool

    GPIO_A0         = 128,   // uint16_t
    GPIO_A1         = 129,   // uint16_t
    GPIO_A2         = 130,   // uint16_t
    GPIO_A3         = 131,   // uint16_t

    TIMESTAMP       = 192    // uint64_t

    // If you add a type, make sure to integrate it into 'init_packet_type_meta' defined in 'src/PacketType.cpp'
};

struct PacketTypeMeta {
#ifndef DZB_PACKET_TYPE_DONT_STORE_NAME
    char const* name = "<unknown>";
#endif

    std::size_t size_of_data = 0;
    bool known = false;

    PacketTypeMeta() = default;
    PacketTypeMeta(char const* name, std::size_t size_of_data);
};

// Must be called to use the following functions
void init_packet_type_meta();

// Is this type known to the system
bool is_type_known(PacketType type);

// The size of the data following that type
std::size_t size_of_packet(PacketType type);

// The pretty name of that packet, for debugging purpose
// If 'DZB_PACKET_TYPE_DONT_STORE_NAME' is defined, always returns the same value
char const* name_of_type(PacketType type);

}