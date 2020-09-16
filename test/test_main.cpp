#include <Arduino.h>
#include <unity.h>

#include <dzb/CRC.h>
#include <dzb/Packet.h>
#include <dzb/PacketType.h>
#include <dzb/PacketWriter.h>
#include <dzb/PacketReader.h>
#include <dzb/Serialization.h>

void setup() {
    UNITY_BEGIN();

    int number_of_packet = 0;
    auto check_packet = [&] (dzb::Packet const& packet) {
        ++number_of_packet;

        dzb::PacketReader reader(packet);

        float temp;
        uint8_t batt_percent0, batt_percent1;
        uint8_t lum;
        bool gpio_d1;
        uint16_t gpio_a0;

        if (
            reader.get_value(dzb::PacketType::TEMPERATURE, 0, temp) &&
            reader.get_value(dzb::PacketType::BATT_PERCENT, 0, batt_percent0) &&
            reader.get_value(dzb::PacketType::BATT_PERCENT, 1, batt_percent1) &&
            reader.get_value(dzb::PacketType::LUMINOSITY, 0, lum) &&
            reader.get_value(dzb::PacketType::GPIO_D1, 0, gpio_d1) &&
            reader.get_value(dzb::PacketType::GPIO_A0, 0, gpio_a0)
        ) {
            TEST_ASSERT_EQUAL_UINT8(85, batt_percent0);
            TEST_ASSERT_EQUAL_UINT8(12, batt_percent1);

            TEST_ASSERT_EQUAL_UINT8(32, lum);

            TEST_ASSERT_EQUAL_UINT16(2020, gpio_a0);

            TEST_ASSERT_FLOAT_WITHIN(0.1, 21, temp);

            TEST_ASSERT_EQUAL(true, gpio_d1);
        } else {
            TEST_FAIL_MESSAGE("Some value could not be retrieved");
        }
    };

    dzb::PacketWriter writer(dzb::id_t{ 'A', 'Z' }, 8 /* bytes */, check_packet);

    writer.write(dzb::PacketType::TEMPERATURE, float{ 21 }); // 4 bytes
    writer.write(dzb::PacketType::BATT_PERCENT, uint8_t{ 85 }); // 1 byte => 5 bytes
    writer.write(dzb::PacketType::GPIO_A0, uint16_t{ 2020 }); // 8 byte => 13 bytes
    writer.write(dzb::PacketType::LUMINOSITY, uint8_t{ 32 }); // 1 byte => 14 bytes
    writer.write(dzb::PacketType::BATT_PERCENT, uint8_t{ 12 }); // 1 byte => 15 bytes
    writer.write(dzb::PacketType::GPIO_D1, bool{ true }); // 1 byte => 16 bytes => Emit packet



    UNITY_END();
}

void loop() {}