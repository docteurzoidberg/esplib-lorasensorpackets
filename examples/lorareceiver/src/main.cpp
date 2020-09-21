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

static const char *TAG = "DEMO";

uint16_t packetCounter=0;
uint32_t lastPacketReceived=0;

void on_lora_packet_received(int size) {
  std::vector<uint8_t> buffer(size);
  LoRa.readBytes(buffer.data(), size);

  auto packet = dzb::Packet::deconstruct(buffer.data(), size);

  if (!packet.is_crc_valid()) {
    // TODO: handle error
  }

  dzb::PacketReader reader(packet);
  // TODO: use reader.get_value(dzb::PacketType::*) to read data

  packetCounter++;
  lastPacketReceived=millis();
  ESP_LOGD(TAG, "Received packet %d of %d bytes with RSSI %d", packetCounter, readBytes, LoRa.packetRssi());
}

void setup(){
  SPI.begin(PIN_SCK,PIN_MISO,PIN_MOSI,PIN_SS);
  LoRa.setPins(PIN_SS,PIN_RST,PIN_DI00);
  if (!LoRa.begin(BAND)){
    ESP_LOGD(TAG, "Starting LoRa failed!");
    while(1){
      delay(1000);
    }
  }

  LoRa.onReceive(on_lora_packet_received);
  LoRa.receive();
}

void loop(){}