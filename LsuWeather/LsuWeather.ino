#include <Arduino.h>

#include <SparkFunBME280.h> // https://github.com/sparkfun/SparkFun_BME280_Arduino_Library
#include <LsuWiFiC.h>
#include <Esp.h>

#include <math.h>

#include "MqttClient.h"

#define SECONDS_AS_PICO (1000000)
#define SECONDS_AS_NANO (1000)

BME280 sensor;

const char* broker = "192.168.100.11";
uint16_t port = 1883;
void mqttCallback(char*, byte*, unsigned int);
WiFiClient espClient;

MqttClient mqttClient(broker, port, mqttCallback, espClient);

int charsOf(float n, uint8_t decimals = 2)
{
  if(decimals)
    return snprintf(0, 0, "%d", (int) n) + 1 + decimals;
  else
    return snprintf(0, 0, "%d", (int) n) + decimals;
}

/**
 * Fills and returns the out buffer with given float number having
 */
char* float2String(char* out, float n, uint8_t decimals = 2)
{
  if(!decimals)
  {
    sprintf(out, "%d", (int)n);
    return out;
  }
  int p = pow(10, decimals);
  // float as integer
  int i = round(n * p);
  // decimal part
  int d = i % p;
  if(d < 0)
    d *= -1;
  // integer part
  i = (i - d) / p;
  sprintf(out, "%d.%d", i, d);
  return out;
}

void startBME280(uint8_t commInterface = I2C_MODE, uint8_t I2CAddress = 0x77,
    uint8_t chipSelectPin = 10, uint8_t runMode = 3, uint8_t tStandby = 0,
    uint8_t filter = 0, uint8_t tempOverSample = 5, uint8_t pressOverSample = 5,
    uint8_t humidOverSample = 5)
{
  sensor.settings.commInterface = commInterface;
  if (commInterface == I2C_MODE)
    sensor.settings.I2CAddress = I2CAddress;
  if (commInterface == SPI_MODE)
    sensor.settings.chipSelectPin = chipSelectPin;
  sensor.settings.runMode = runMode;
  sensor.settings.tStandby = tStandby;
  sensor.settings.filter = filter;
  sensor.settings.tempOverSample = tempOverSample;
  sensor.settings.pressOverSample = pressOverSample;
  sensor.settings.humidOverSample = humidOverSample;
  // BME280 requires 2ms to start up. Make sure sensor had enough time to turn on.
  delay(5);
  // Calling .begin() causes the settings to be loaded
  sensor.begin();
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length)
{
  // does nothing unless subscribed to at least a topic
}

void reconnect()
{
  while (!mqttClient.connected())
  {
    // Create a random mqttClient ID
    const char* mqttClientId = "LsuMqttClient";
    // Attempt to connect
    if (mqttClient.connect(mqttClientId))
    {
      // this is the right place to subscribe to a topic, i.e.
      // mqttClient.subscribe("SomeTopic");
    }
    else
    {
      // Wait 1 second before retrying
      delay(1000);
    }
  }
}

void setup()
{
  unsigned long mills = millis();
  Serial.begin(115200);

  // start sensor
  startBME280();

  // read temperature [C]
  float temp = sensor.readTempC();
  // make it int
  int temp_int = (int) round(temp * 100);
  int temp_len = charsOf(temp_int, 0);

  // read pressure [Pa], 1 atm = 101325 Pa = 1013.25 hPa
  float pres = sensor.readFloatPressure();
  // make it int
  int pres_int = (int) round(pres * 100);
  int pres_len = charsOf(pres_int, 0);

  // read altitude [m]
  float alt = sensor.readFloatAltitudeMeters();
  // make it int
  int alt_int = (int) round(alt * 1000);
  int alt_len = charsOf(alt_int, 0);

  // read humidity [%]
  float humid = sensor.readFloatHumidity();
  // make it int
  int humid_int = (int) round(humid * 100);
  int humid_len = charsOf(humid_int, 0);

  // temp=,pres=,alt=,humid=
  // 123456789 123456789 1234
  int msg_len = temp_len + pres_len + alt_len + humid_len + 24;
  char msg[msg_len];
  sprintf(msg, "temp=%d,pres=%d,alt=%d,humid=%d", temp_int, pres_int, alt_int, humid_int);

  // connect to WiFi
  connectLsuWiFi(2);

  // send data
  if(!mqttClient.loop())
    reconnect();
  mqttClient.publish("weather", msg);

  WiFi.disconnect();

  // deep sleep
  ESP.deepSleep(60 * SECONDS_AS_PICO - (millis() - mills) * SECONDS_AS_NANO);
}

void loop()
{
  // does nothing
}
