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
uint8_t packet[53];

void handleRadioData(){

  // read packet(s)
  if (!LoRa.available()) {
    return;
  }

  int readBytes = LoRa.readBytes((uint8_t*) &packet, 52);
  //if(readBytes!=sizeof(DeviceDataStruct)){
    //ESP_LOGD(TAG, "Invalid packet size: %d", readBytes);
    //return;
  //}
  //if(strcmp(newState.header,"J7")!=0){
    //ESP_LOGD(TAG, "Invalid packet header: %s", newState.header);
    //return;
  //}

  packetCounter++;
  lastPacketReceived=millis();

  //ESP_LOGD(TAG, "Received packet %d with RSSI %d", packetCounter, LoRa.packetRssi());
}

void setup() {

  SPI.begin(PIN_SCK,PIN_MISO,PIN_MOSI,PIN_SS);
  LoRa.setPins(PIN_SS,PIN_RST,PIN_DI00);

  if (!LoRa.begin(BAND))
  {
    //ESP_LOGCONFIG(TAG, "Starting LoRa failed!");
    while (1) {
      delay(1000);
    }
  }
}

void loop() {
  int packetSize = LoRa.parsePacket(52);
  if(packetSize){
    handleRadioData();
  }
}