#include <Arduino.h>

#include <SparkFunBME280.h> // https://github.com/sparkfun/SparkFun_BME280_Arduino_Library

#include <LsuWiFi.h>
#include <Esp.h>

#include <math.h>

#include "MqttClient.h"

#define SECONDS_AS_PICO (1000000)

BME280 sensor;

const char* broker = "192.168.100.59";
uint16_t mqtt_port = 1883;
void mqttCallback(char*, byte*, unsigned int);
WiFiClient espClient;

typedef uint8_t (*SEND_FUNCTION)(uint8_t*, uint8_t, uint8_t, ...);

MqttClient mqttClient(broker, mqtt_port, mqttCallback, espClient);

void startBME280()
{
  sensor.settings.commInterface = I2C_MODE;
  sensor.settings.I2CAddress = 0x77;
  sensor.settings.runMode = 3;
  sensor.settings.tStandby = 0;
  sensor.settings.filter = 5;
  sensor.settings.tempOverSample = 5;
  sensor.settings.pressOverSample = 5;
  sensor.settings.humidOverSample = 5;
  // BME280 requires 2ms to start up. Make sure sensor had enough time to turn on.
  delay(10);
  // Calling .begin() causes the settings to be loaded
  sensor.begin();
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length)
{
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print(F("] "));
  for (int i = 0; i < length; i++)
  {
    Serial.print((char) payload[i]);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);

  // start sensor
  startBME280();

  Serial.println();

  // read temperature [C]
  float temp = sensor.readTempC();
  Serial.print("temp=");Serial.println(temp);
//  // make it int
//  int temp_i = (int) round(temp * 100);
//  int temp_d = temp_i % 100;
//  temp_i = (temp_i - temp_d) / 100
  int temp_len = 6;

  // read altitude [m]
  float alt = sensor.readFloatAltitudeMeters();
  Serial.print("alt=");Serial.println(alt);
  // make it int
//  int alt_i = (int) round(alt * 100);
  int alt_len = 5;//charsOf(alt_i, 0);

  // read pressure [Pa], 1 atm = 101325 Pa = 1013.25 hPa = 760 mmHg
  float pres = sensor.readFloatPressure() * .99;
  Serial.print("pres=");Serial.println(pres);
  // make it int
//  int pres_i = (int) round(pres);
  int pres_len = 6;//charsOf(pres_i, 0);

  // read humidity [%]
  float humid = sensor.readFloatHumidity();
  Serial.print("humid=");Serial.println(humid);
  // make it int
//  int humid_i = (int) round(humid * 100);
  int humid_len = 5;//charsOf(humid_i, 0);

  int batt = analogRead(A0);
  // convert battery level to percent
  // maps
  //   350 aka 4.2 V -> 100 %
  //   265 aka 3.2 V -> 0 %
  int batt_i = map(batt, 265, 350, 0, 100);
  int batt_len = 3;//charsOf(batt_i, 0);

  // temp=,pres=,alt=,humid=,batt=
  // 123456789 123456789 1234567890
  int msg_len = temp_len + pres_len + alt_len + humid_len + batt_i + 30;
  char msg[msg_len];
  sprintf(msg, "temp=%d.%d,pres=%d.%d,alt=%d.%d,humid=%d.%d,batt=%d"
      , (int) round(temp), ((int) round(temp * 100)) % 100
      , (int) round(pres), ((int) round(pres * 100)) % 100
      , (int) round(alt), ((int) round(alt * 100)) % 100
      , (int) round(humid), ((int) round(humid * 100)) % 100
      , batt_i);
  Serial.println(msg);

  // connect to WiFi
  if(connectLsuWiFi(0, 5000, false))
  {
    // connect to MQTT broker
    if (mqttClient.connect("LsuWeather"))
    {
      // send data
      mqttClient.loop();
      mqttClient.subscribe("weather");
      mqttClient.publish("weather", msg);
      Serial.println("Sent date to MQTT broker");
    }
    else
    {
      Serial.println("Could not connect MQTT broker");
    }
  }
  else
  {
    Serial.println("Could not connect WiFi");
  }

  // deep sleep
  ESP.deepSleep(60 * SECONDS_AS_PICO);
}

void loop()
{
  // does nothing
}
