#include <stdio.h>
#include <stdint.h>
#include <cstring>
#include <vector>
#include <cassert>
#include <array>
#include <functional>
#include <iterator>

#include <dzb/CRC.h>
#include <dzb/Packet.h>
#include <dzb/PacketType.h>
#include <dzb/PacketWriter.h>
#include <dzb/PacketReader.h>
#include <dzb/Serialization.h>

uint8_t packet[52]={0};
uint8_t* packet_buffer = packet;


// test read 2 temperature
void onReceive(uint8_t const* packet, std::size_t size) {
    auto packets = dzb::parse_packet(packet, size);

    float temp1, temp2;
    bool okay = true;

    okay = okay && dzb::get_packet_of_type_and_deserialize(packets, dzb::PacketType::TEMPERATURE, 0, temp1);
    okay = okay && dzb::get_packet_of_type_and_deserialize(packets, dzb::PacketType::TEMPERATURE, 1, temp2);

    if (okay) {
        // do_something(temp1, temp2);
    } else {
        // packet doesn't have 2 temperature packets
    }
}

// test read 2 temperature
void onReceiveWithPacketReader(uint8_t* packet_buffer, std::size_t size) {
    dzb::Packet packet = dzb::Packet::deconstruct(packet_buffer, size);
    dzb::PacketReader reader(packet);

    float temp1, temp2;

    if (reader.get_value(dzb::PacketType::TEMPERATURE, 0, temp1) && 
        reader.get_value(dzb::PacketType::TEMPERATURE, 1, temp2)) {
        // do_something(temp1, temp2);
    } else {
        // packet doesn't have 2 temperature packets
    }
}



void test_unserialize(uint8_t const* packet, uint8_t packetsize) {

  struct header_struct {
    unsigned char id[2];
    uint8_t crc;
    uint8_t ack;
  };

  header_struct header;
  memcpy(&header, packet, sizeof(header_struct));
  //header.id[2]=(unsigned char)0;

  printf("--------------------\n");
  printf("Packet size: %d\n", packetsize);
  printf("Header:\n");
  printf("--------------------\n");

  printf(" id:\t %c%c\n", header.id[0],header.id[1]); //pas reussi a faire mieux
  printf(" crc:\t %02X\n", header.crc);
  printf(" ack:\t %02X\n", header.ack);

  uint8_t const* packetAddr = packet;

  packet+=sizeof(header_struct);

  uint8_t curIndex = packet-packetAddr;

  while(curIndex<packetsize) {
    printf("--------------------\n");
    auto currentType = dzb::deserialize<dzb::PacketType>(packet);
    printf("curType: %02X - ", (uint8_t)currentType);

    if(currentType==dzb::PacketType::GPIO_D9) {
      uint8_t currentValue = dzb::deserialize<uint8_t>(packet);
      printf("D9 value: %d\n", currentValue);
    }
    else if(currentType==dzb::PacketType::GPIO_A3) {
      uint16_t currentValue = dzb::deserialize<uint16_t>(packet);
      printf("A3 value: %d\n", currentValue);
    }
    else if(currentType==dzb::PacketType::BATT_VOLTAGE) {
      float currentValue = dzb::deserialize<float>(packet);
      printf("Batt voltage value: %0.2f\n", currentValue);
    }
    else {
      //je sais pas quoi faire si je connais pas le type...
      printf("UNKNOWN TYPE: %d\n", (uint8_t)currentType);
    }
    curIndex=packet-packetAddr;
  }
}

int main() {

  //----------------------------------------
  //Packet builder
  //----------------------------------------

  //sender's device's ID
  unsigned char id[] = "DZ";

  //third value. random: humidity
  float voltage_value = 3.723;

  //fourth value. random: gpio_D9 (digital)
  bool digital_d9_value = true;

  //fifth value. random: gpio_A3 (analog)
  uint16_t analog_a3_value = (uint16_t)278;


  uint8_t* packetBuffAddr = packet_buffer;

  //ID
  packet_buffer = dzb::serialize(packet_buffer, (unsigned char)id[0]);
  packet_buffer = dzb::serialize(packet_buffer, (unsigned char)id[1]);
  //packet_buffer = serialize(packet_buffer, (unsigned char)0);

  uint8_t checksum = 0x3c;
  packet_buffer = dzb::serialize(packet_buffer, checksum);

  uint8_t ack = 0x01;
  packet_buffer = dzb::serialize(packet_buffer, ack);

  //one value
  packet_buffer = dzb::serialize_with_type(packet_buffer, dzb::PacketType::GPIO_D9, digital_d9_value);

  //another value
  packet_buffer = dzb::serialize_with_type(packet_buffer, dzb::PacketType::GPIO_A3, analog_a3_value);

  //another value
  packet_buffer = dzb::serialize_with_type(packet_buffer, dzb::PacketType::BATT_VOLTAGE, voltage_value);


  //TODO: calc checksum
  packet[2] = (uint8_t) 0xff;

  //DISPLAY
  for(uint8_t* ptr = packet; ptr < packet_buffer; ++ptr) {
    printf("%02X => %c\n", (uint8_t)*ptr, (uint8_t)*ptr);
  }

  uint8_t packetSize = packet_buffer - packetBuffAddr;

  test_unserialize(packet, packetSize);
  return 0;
}
