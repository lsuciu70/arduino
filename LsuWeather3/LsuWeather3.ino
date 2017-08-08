#include <Arduino.h>
#include <Wire.h>

#include <Esp.h>

#include <Adafruit_BME280.h>

#include <LsuWiFi.h>
#include <LsuNtpTime.h>
#include <MqttClient.h>

#include <stdint.h>

#define DEBUG

#define SECONDS_AS_PICO (1000000)
#define SECONDS_AS_MICRO (1000)
#define SEALEVELPRESSURE_HPA (101325)
#define ELEVATION (90)

Adafruit_BME280 bme;

const char* broker = "192.168.100.59";
uint16_t mqtt_port = 1883;

WiFiClient espClient;
MqttClient mqttClient(broker, mqtt_port, espClient);


//                        12345..6..789 12..3..456789 ..1..234567
const char* MSG_FORMAT = "temp=%d.%d,pres=%d.%d,humid=%d.%d,batt=%d";
const int MSG_LEN = 27 + 5 + 6 + 5 + 3 + 1;

void setup()
{
  Serial.begin(115200);
  uint64_t mills = millis();
  if (bme.begin())
  {
    // Suggested settings for weather station:
    // - Sensor mode: forced mode, 1 sample / minute
    // - Oversampling settings: pressure ×1, temperature ×1, humidity ×1
    // - IIR filter settings: filter off
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF);
    bme.takeForcedMeasurement();
#ifdef DEBUG
    Serial.println();
    Serial.println(F("Weather Station Scenario:"));
    Serial.println(F("- Sensor mode: forced mode, 1 sample / minute"));
    Serial.println(F("- Oversampling settings: pressure ×1, temperature ×1, humidity ×1"));
    Serial.println(F("- IIR filter settings: filter off"));
    Serial.println();
#endif
    int temperature = (int)round(bme.readTemperature() * 100); // 100 C
    float pressure_h = bme.readPressure();
    int pressure = (int)round(bme.seaLevelForAltitude(ELEVATION, pressure_h)); // Pa
    int pressure_mm = (int)round(pressure * 760 / SEALEVELPRESSURE_HPA); // mm Hg
    int humidity = (int)round(bme.readHumidity() * 100); // 10000 %
    // send BME280 to sleep
    bme.setSampling(Adafruit_BME280::MODE_SLEEP);

    int batt = analogRead(A0);
    // convert battery level to percent
    // maps
    //   350 aka 4.2 V -> 100 %
    //   265 aka 3.2 V -> 0 %
    int b_i = map(batt, 265, 350, 0, 100);

    int t_d = (int)(temperature) % 100, t_i = (int)((temperature - t_d) / 100);
    int p_d = (int)(pressure) % 100, p_i = (int)((pressure - p_d) / 100);
    int h_d = (int)(humidity) % 100, h_i = (int)((humidity - h_d) / 100);

    char msg[MSG_LEN];
    sprintf(msg, MSG_FORMAT, t_i, t_d, p_i, p_d, h_i, h_d, b_i);

#ifdef DEBUG
//    // NTP Synch
//    if(connectLsuWiFi(0, 5000, false) && LsuNtpBegin())
//    {
//      char datetime[datetimeStringLength + 1];
//      datetimeString(datetime);
//      Serial.println(datetime);
//    }
    Serial.print(F("Temperature [100C] = "));
    Serial.println(temperature);
    Serial.print(F("Pressure H [Pa] = "));
    Serial.println(pressure_h);
    Serial.print(F("Pressure [Pa] = "));
    Serial.println(pressure);
    Serial.print(F("Pressure [mmHg] = "));
    Serial.println(pressure_mm);
    Serial.print(F("Humidity [10000 %] = "));
    Serial.println(humidity);
    Serial.print(F("Battery [%] = "));
    Serial.println(b_i);
    Serial.println();
    Serial.print(F("weather: "));
    Serial.println(msg);
    Serial.println();
#endif

    // connect to WiFi
    if(connectLsuWiFi(0, 5000, false))
    {
      // connect to MQTT broker
      if (mqttClient.connect("CLLS-WeatherStation"))
      {
        // send data
        mqttClient.loop();
        mqttClient.subscribe("weather");
        mqttClient.publish("weather", msg);
#ifdef DEBUG
        Serial.println(F("Sent data to MQTT broker"));
        Serial.println();
#endif
      }
      else
      {
        Serial.println(F("Could not connect MQTT broker"));
        Serial.println();
      }
    }
    else
    {
      Serial.println(F("Could not connect WiFi"));
      Serial.println();
    }
  }
  else
  {
    Serial.println(F("Could not find the BME280 sensor, please check wiring!"));
    Serial.println();
  }

  // deep sleep, suggested rate is 1/60Hz (1m)
#ifdef DEBUG
  Serial.println(F("Sleep 1 minute"));
#endif
  ESP.deepSleep(60 * SECONDS_AS_PICO - (millis() - mills) * SECONDS_AS_MICRO);
}

void loop()
{
}
