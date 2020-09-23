#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#include <dzb/CRC.h>
#include <dzb/Packet.h>
#include <dzb/PacketType.h>
#include <dzb/PacketWriter.h>
#include <dzb/PacketReader.h>
#include <dzb/Serialization.h>

#define  PIN_SCK    5
#define  PIN_MISO   19
#define  PIN_MOSI   27
#define  PIN_SS     18
#define  PIN_RST    14
#define  PIN_DI00   26
#define  BAND       433E6
#define  PABOOST    true

uint16_t packetCounter=0;
uint32_t lastPacketReceived=0;

void print_byte(uint8_t byte) {
    static std::array<char, 16> to_hex = {
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'A', 'B',
        'C', 'D', 'E', 'F'
    };

    Serial.print(to_hex[(byte >> 4) & 0xF]);
    Serial.print(to_hex[(byte >> 0) & 0xF]);
}

void print_buffer(uint8_t const* buffer, std::size_t size) {
    std::size_t per_line = 16;
    std::size_t line_count = (size + per_line - 1) / per_line;

    for(int line = 0; line < line_count; ++line) {
        if (line > 0) {
            Serial.print('\n');
        }

        std::size_t byte_count = line == line_count - 1 ? size - line * per_line : per_line;

        for(int i = 0; i < byte_count; ++i) {
            int idx = line * per_line + i;
            uint8_t byte = buffer[idx];
            print_byte(byte);
            Serial.print(' ');
        }
    }
}

void print_packet_content(dzb::Packet const& packet) {
  dzb::PacketReader reader(packet);
  for(auto payload : reader.packets) {
    Serial.printf("%s\n", dzb::name_of_type(dzb::deserialize<dzb::PacketType>(payload)));
  }
}

void print_packet(dzb::Packet const& packet) {

    Serial.print('\n');
    Serial.println("ID: ");
    Serial.printf("%c",packet.get_id()[0]);
    Serial.printf("%c",packet.get_id()[1]);
    Serial.print('\n');

    Serial.println("Ack: ");
    Serial.print(packet.get_ack());
    Serial.print('\n');

    Serial.println("CRC: ");
    Serial.print(packet.get_crc());
    Serial.print('\n');

    Serial.println("Buffer: ");
    auto buffer = packet.get_data().first;
    auto size = packet.get_data().second;

    Serial.write(buffer, size);
    Serial.println("// EOP //");

    dzb::PacketType type = dzb::deserialize<dzb::PacketType>(buffer);
    Serial.println("");
    Serial.print(static_cast<uint8_t>(type));
    Serial.print(" | ");
    Serial.print(dzb::is_type_known(type) ? "known" : "unknown");
    Serial.print(" | name: ");
    Serial.print(dzb::name_of_type(type));
    Serial.print(" | size of payload: ");
    Serial.print(dzb::size_of_packet(type));
    Serial.print('\n');
}

void on_lora_packet_received(int size) {
  std::vector<uint8_t> buffer(size);

  size = LoRa.readBytes(buffer.data(), size);
  //print_buffer(buffer.data(), size);

  Serial.printf("[LORA RCV] Received packet %d of %d bytes with RSSI %d\n", packetCounter, size, LoRa.packetRssi());

  auto packet = dzb::Packet::deconstruct(buffer.data(), size);
  //print_packet(packet);

  if (!packet.is_crc_valid()) {
    // TODO: handle error
    Serial.println("[PACKET] CRC error");
    print_packet(packet);
    return;
  }

  float batt_voltage=0.0f;
  uint8_t batt_percent=0;
  bool presence=0;
  bool gpio_d1=0;
  bool gpio_d2=0;

  dzb::PacketReader reader(packet);
  if(
    reader.get_value(dzb::PacketType::PRESENCE, 0, presence) &&
    reader.get_value(dzb::PacketType::GPIO_D1, 0, gpio_d1) &&
    reader.get_value(dzb::PacketType::GPIO_D2, 0, gpio_d2) &&
    reader.get_value(dzb::PacketType::BATT_PERCENT, 0, batt_percent)&&
    reader.get_value(dzb::PacketType::BATT_VOLTAGE, 0, batt_voltage)&&
  true) {
    //?
  }
  else {
    Serial.println("[PACKET] Error: Not all values where read");
  }

  Serial.printf("[LORA RCV] Alarm=%d|LowBatt=%d|BattVoltage=%0.2f|BattPercent=%d|Presence=%d\n", gpio_d1,gpio_d2, batt_voltage, batt_percent, presence);
  packetCounter++;
  lastPacketReceived=millis();
}

void setup(){

  Serial.begin(115200);
  SPI.begin(PIN_SCK,PIN_MISO,PIN_MOSI,PIN_SS);
  LoRa.setPins(PIN_SS,PIN_RST,PIN_DI00);

  dzb::init_packet_type_meta();
  dzb::init_crc_table();

  if (!LoRa.begin(BAND)){
    Serial.println("[LORA RCV] Starting LoRa failed!");
    while(1){
      delay(1000);
    }
  }

  LoRa.onReceive(on_lora_packet_received);
  LoRa.receive();
}

void loop(){}