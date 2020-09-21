#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <math.h>

#include <dzb/CRC.h>
#include <dzb/Packet.h>
#include <dzb/PacketType.h>
#include <dzb/PacketWriter.h>
#include <dzb/PacketReader.h>
#include <dzb/Serialization.h>

#define TIME_TO_SLEEP  60         //wakeup every X seconds to report
#define ALARM_MIN_DURATION 10     //time in seconds before toggling off alert
#define LORA_REPORT_TIMER 1       //report every Xsec

#define SCK     5
#define MISO    19
#define MOSI    27
#define SS      18
#define RST     14
#define DI00    26
#define PIR_DATA 35
#define BAND    433E6
#define PABOOST true
#define uS_TO_S_FACTOR 1000000    /* Conversion factor for micro seconds to seconds */

static const char *TAG = "J7";

esp_sleep_wakeup_cause_t wakeup_reason;
RTC_DATA_ATTR int bootCount = 0;

ulong lastDrawScreen=0;
ulong lastPirEvent=0;
ulong lastPirRead=0;
ulong lastAdcRead=0;
ulong lastLoraReport=0;
ulong packetCounter=0;


struct DeviceDataStruct {
  char 				header[3];
  //char 				device[6];
  uint64_t    timer;
  float       deviceBattVoltage;
  uint8_t     deviceBattPercent;
  bool        devicePirState;
  bool        deviceAlarmActive;
  bool        deviceLowBatt;
};


DeviceDataStruct state = {
  .header="J7",
  //.device="J7-1",
  .timer=0,
  .deviceBattVoltage=0.00f,
  .deviceBattPercent=100,
  .devicePirState=false,
  .deviceAlarmActive=false,
  .deviceLowBatt=false
};

bool readPIR(){
  if(((millis()-lastPirRead)<1000) && (lastPirRead!=0)){
    return state.devicePirState;
  }
  state.devicePirState = (digitalRead(PIR_DATA));
  lastPirRead=millis();
  return state.devicePirState;
}

void readBatteryVoltage(){
  if(((millis()-lastAdcRead)<5000) && (lastAdcRead!=0)){
    return;
  }
  state.deviceBattVoltage = (analogRead(34)/4095.00f)*2*3.3f*1.1f;
  state.deviceBattPercent = map((int)(state.deviceBattVoltage*1000),3000,4200,0,100);
  state.deviceLowBatt = state.deviceBattPercent<=10;
  lastAdcRead=millis();
}
void print64(uint64_t value) {
  const int NUM_DIGITS    = log10(value) + 1;
  char sz[NUM_DIGITS + 1];
  sz[NUM_DIGITS] =  0;
  for ( size_t i = NUM_DIGITS; i--; value /= 10)
  {
    sz[i] = '0' + (value % 10);
  }

  Serial.print(sz);
}

void loraReportState() {

  if(((millis()-lastLoraReport)<(LORA_REPORT_TIMER*1000)) && (lastLoraReport>0)){
    return;
  }

  state.timer=(uint64_t)esp_timer_get_time();


  Serial.print("[LORA REPORT]");
  Serial.print(" TS=");
  print64(state.timer);
  Serial.print(" Alarm=");
  Serial.print(String(state.deviceAlarmActive));
  Serial.print(" PIR=");
  Serial.print(String(state.devicePirState));
  Serial.print(" BattVoltage=");
  Serial.print(String(state.deviceBattVoltage));
  Serial.print(" BattPercent=");
  Serial.print(String(state.deviceBattPercent));
  Serial.print(" Lowbatt=");
  Serial.println(String(state.deviceLowBatt));

  // send packet
  LoRa.beginPacket(true);
  //TODO: use lib
  LoRa.write((const uint8_t*)&state,sizeof(DeviceDataStruct));
  LoRa.endPacket();

  packetCounter++;
  lastLoraReport=millis();
}

void alarmLoop() {

  Serial.println("> Alarm loop");
  state.deviceAlarmActive=true;
  while(state.deviceAlarmActive) {
    readBatteryVoltage();
    if(readPIR()) {
      lastPirEvent=millis();
    }
    state.deviceAlarmActive = (millis()-lastPirEvent)<(ALARM_MIN_DURATION*1000);
    loraReportState();
    delay(200);
  }
  state.deviceAlarmActive = false;
  state.devicePirState = false;
}

void alarmEndLoop(){
  Serial.println("> Alarm end loop");
  ulong endAlarmTime = millis();
  state.deviceAlarmActive=false;
  state.devicePirState=false;
  for(int i=0;i<5;i++){
    readBatteryVoltage();
    loraReportState();
    delay(1020);
  }
}

void setup(){

  pinMode(PIR_DATA, INPUT);

  wakeup_reason = esp_sleep_get_wakeup_cause();

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.print("IDS-DEVICE: Starting ! (reason=");
  Serial.print(wakeup_reason);
  Serial.println(")");

  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI00);

  if(bootCount == 0)
    bootCount++;

  if (!LoRa.begin(BAND)){
    Serial.println("IDS-DEVICE: Starting LoRa failed!");
    Serial.flush();
    delay(1000);
    ESP.restart();
  }

  readBatteryVoltage();

  if(readPIR()){
    alarmLoop();
    alarmEndLoop();
  }else{
    loraReportState();
  }

  delay(2000);

  Serial.println("IDS-DEVICE: going to sleep !");
  Serial.flush();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 1); //1 = High, 0 = Low
  esp_deep_sleep_start();
}


//never called
void loop(){
}
