/*
 *
 */
#ifndef _LsuWater_H_
#define _LsuWater_H_

#include <Arduino.h>

#include <LsuWiFi.h>
#include "LsuMqttClient.h"

#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson.git

namespace
{

// Water flow part
/* Second(s) */
const unsigned char WATER_HANDLER_PERIOD = 60;
/* Liters per count (full rotation) */
const float LITER_PER_COUNT = 1 / 60;

/* Interrupts may be attached to any ESP8266 GPIO except GPIO16 */
#define WFLOW_PIN 2

/* Water flow rotation counter */
volatile unsigned long wFlowCount = 0;

unsigned long nextHandleTime;


// MQTT part
#define MQTT_PAYLOAD_LEN 24

const char* WATER_COMMAND = "WCmd";
const char* WATER_COMMAND_RESET = "reset";
const char* WATER_COMMAND_CLOSE = "close";
const char* WATER_COMMAND_OPEN = "open";

const char* INSTANT_WATER_VOLUME = "water";
const char* INSTANT_WATER_VOLUME_FMT = "{\"iwv\":\"%d.%d\"}";

DynamicJsonBuffer jsonBuffer(MQTT_PAYLOAD_LEN + 1);

WiFiClient espClient; // the WiFi client

const char* broker = "192.168.100.60"; // the broker's IP (Raspberry Pi)
uint16_t mqtt_port = 1883; // MQTT default port
const char* wClientId = "MQTTClient"; // Client ID

// the callback function
void callback(char* topic, byte* payload, unsigned int length)
{
  if(strcmp(topic, WATER_COMMAND) == 0)
  {
    JsonObject& msg = jsonBuffer.parseObject(payload);
    if (msg.success()) {
      if(msg[WATER_COMMAND_RESET])
      {
        // reset
      }
      else if(msg[WATER_COMMAND_OPEN])
      {
        // open the valve
      }
      else if(msg[WATER_COMMAND_CLOSE])
      {
        // close the valve
      }
    }
  }
}

void onConnect(MqttClient &client)
{
  client.subscribe(WATER_COMMAND);
}

void waterFlowInterrupt()
{
  ++wFlowCount;
}

void handleWaterFlow()
{
  nextHandleTime = millis() + WATER_HANDLER_PERIOD;
  // compute flow per handler period (liter / minute)
  float waterCounted = LITER_PER_COUNT * wFlowCount;
  wFlowCount = 0;

  // send it
  int waterCount = roundf(waterCounted * 100);
  int waterCount_d = waterCount % 100;
  int waterCount_i = (waterCount - waterCount_d) / 100;
  if (waterCount_d < 0)
    waterCount_d *= -1;
  char mqtt_msg[MQTT_PAYLOAD_LEN + 1];
  sprintf(mqtt_msg, INSTANT_WATER_VOLUME_FMT, waterCount_i, waterCount_d);
  LsuMqtt::publish(INSTANT_WATER_VOLUME, mqtt_msg);
}

} // end anonymous namespace

void setup()
{
  Serial.begin(115200);
  Serial.println();

  LsuWiFi::connect();

  LsuMqtt::init(broker, mqtt_port, wClientId, espClient, callback, onConnect);

  pinMode(WFLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WFLOW_PIN), waterFlowInterrupt, FALLING);

  nextHandleTime = millis() + WATER_HANDLER_PERIOD;
}

void loop()
{
  LsuMqtt::loop();
  if(millis() >= nextHandleTime)
  {
    handleWaterFlow();
  }
}
#endif /* _LsuWater_H_ */
