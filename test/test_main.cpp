
#include <unity.h>

#include <Arduino.h>

#include <dzb/CRC.h>
#include <dzb/Packet.h>
#include <dzb/PacketType.h>
#include <dzb/PacketWriter.h>
#include <dzb/PacketReader.h>
#include <dzb/Serialization.h>

void print(char const* msg) {
    UnityPrint(msg);
}

void print_char(char c) {
    UNITY_OUTPUT_CHAR(c);
}

void print_byte(uint8_t byte) {
    static std::array<char, 16> to_hex = {
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'A', 'B',
        'C', 'D', 'E', 'F'
    };

    print_char(to_hex[(byte >> 4) & 0xF]);
    print_char(to_hex[(byte >> 0) & 0xF]);
}

void println(char const* msg) {
    print(msg);
    print_char('\n');
}

void log(char const* msg) {
    print("[^] ");
    print(msg);
}

void logln(char const* msg) {
    log(msg);
    UNITY_OUTPUT_CHAR('\n');
}

void errln(char const* msg) {
    print("[!] ");
    println("Some value could not be retrieved");
}

void print_buffer(uint8_t const* buffer, std::size_t size) {
    std::size_t per_line = 16;
    std::size_t line_count = (size + per_line - 1) / per_line;

    for(int line = 0; line < line_count; ++line) {
        if (line > 0) {
            print_char('\n');
        }

        std::size_t byte_count = line == line_count - 1 ? size - line * per_line : per_line;

        for(int i = 0; i < byte_count; ++i) {
            int idx = line * per_line + i;
            uint8_t byte = buffer[idx];
            print_byte(byte);
            print_char(' ');
        }
    }
}

template<typename T> 
void print_raw(T const& value) {
    std::array<uint8_t, sizeof(T)> raw;
    memcpy(raw.data(), &value, sizeof(T));

    print_buffer(raw.data(), sizeof(T));
}

void print_packet(dzb::Packet const& packet) {
    log("ID: ");
    print_char(packet.get_id()[0]);
    print_char(packet.get_id()[1]);
    print_char('\n');

    log("Ack: ");
    print_byte(packet.get_ack());
    print_char('\n');

    log("CRC: ");
    print_byte(packet.get_crc());
    print_char('\n');

    logln("Buffer: ");
    auto buffer = packet.get_data().first;
    auto size = packet.get_data().second;

    print_buffer(buffer, size);
    println("// EOP //");

    dzb::PacketType type = dzb::deserialize<dzb::PacketType>(buffer);
    log("");
    print_byte(static_cast<uint8_t>(type));
    print(" | ");
    print(dzb::is_type_known(type) ? "known" : "unknown");
    print(" | name: ");
    print(dzb::name_of_type(type));
    print(" | size of payload: ");
    print_byte(dzb::size_of_packet(type));
    print_char('\n');
}

void print_packet_reader(dzb::PacketReader const& reader) {
    logln("Packet Reader: ");

    for(auto packet : reader.packets) {
        dzb::PacketType type = dzb::deserialize<dzb::PacketType>(packet);

        logln(dzb::name_of_type(type));
    }
}

void test_packet(dzb::Packet const& packet) {
    dzb::PacketReader reader(packet);
    print_packet(packet);
    print_packet_reader(reader);

    float temp = 0;
    uint8_t batt_percent0 = 0, batt_percent1 = 0;
    uint8_t lum = 0;
    bool gpio_d1 = false;
    uint16_t gpio_a0 = 0;

    if (
        reader.get_value(dzb::PacketType::TEMPERATURE, 0, temp) && 
        reader.get_value(dzb::PacketType::BATT_PERCENT, 0, batt_percent0) &&
        reader.get_value(dzb::PacketType::BATT_PERCENT, 1, batt_percent1) &&
        reader.get_value(dzb::PacketType::LUMINOSITY, 0, lum) &&
        reader.get_value(dzb::PacketType::GPIO_D1, 0, gpio_d1) &&
        reader.get_value(dzb::PacketType::GPIO_A0, 0, gpio_a0) &&
        true
    ) {
        TEST_ASSERT_EQUAL_UINT8(85, batt_percent0);
        TEST_ASSERT_EQUAL_UINT8(12, batt_percent1);

        TEST_ASSERT_EQUAL_UINT8(32, lum);

        TEST_ASSERT_EQUAL_UINT16(2020, gpio_a0);

        TEST_ASSERT_FLOAT_WITHIN(0.1, 21, temp);

        TEST_ASSERT_EQUAL(true, gpio_d1);
    } else {
        errln("Some value could not be retrieved");
        TEST_FAIL();
    }
}

void setup() {

    dzb::init_packet_type_meta();
    dzb::init_crc_table();

    UNITY_BEGIN();

    int number_of_packet = 0;
    auto check_packet = [&] (dzb::Packet const& packet) {
        ++number_of_packet;

        logln("Packet emitted:");
        test_packet(packet);

        auto deconstructed_packet = dzb::Packet::deconstruct(packet.buffer.data(), packet.buffer.size());
        logln("Packet deconstructed:");
        test_packet(deconstructed_packet);
    };

    dzb::PacketWriter writer(dzb::id_t{ 'A', 'Z' }, 16 /* bytes */, check_packet);

    writer.write(dzb::PacketType::TEMPERATURE, float{ 21 }); // 1 + 4 bytes
    writer.write(dzb::PacketType::BATT_PERCENT, uint8_t{ 85 }); // 1 + 1 byte => 7 bytes
    writer.write(dzb::PacketType::GPIO_A0, uint16_t{ 2020 }); // 1 + 2 bytes => 10 bytes
    writer.write(dzb::PacketType::LUMINOSITY, uint8_t{ 32 }); // 1 + 1 byte => 12 bytes
    writer.write(dzb::PacketType::BATT_PERCENT, uint8_t{ 12 }); // 1 + 1 byte => 14 bytes
    writer.write(dzb::PacketType::GPIO_D1, bool{ true }); // 1 + 1 byte => 16 bytes => Emit packet

    TEST_ASSERT_EQUAL(1, number_of_packet);

    writer.flush();

    TEST_ASSERT_EQUAL(1, number_of_packet);

    UNITY_END();
}

void loop() {}