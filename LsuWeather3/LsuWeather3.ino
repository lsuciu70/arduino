#include <Arduino.h>
#include <Wire.h>

#include <Esp.h>

#include <Adafruit_BME280.h>

#define DEBUG

#define SECONDS_AS_PICO (1000000)
#define SEALEVELPRESSURE_HPA (101325)
#define ELEVATION (90)

Adafruit_BME280 bme;

//                        12345..6..789 12..3..456789 ..1
const char* MSG_FORMAT = "temp=%d.%d,pres=%d.%d,humid=%d.%d";
const int MSG_LEN = 21 + 5 + 6 + 5 + 1;

void setup()
{
  Serial.begin(115200);
  if (bme.begin())
  {
#ifdef DEBUG
    Serial.println();
    Serial.println(F("Weather Station Scenario:"));
    Serial.println(F("- Sensor mode: forced mode, 1 sample / minute"));
    Serial.println(F("- Oversampling settings: pressure ×1, temperature ×1, humidity ×1"));
    Serial.println(F("- IIR filter settings: filter off"));
    Serial.println();
#endif
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
    int temperature = (int)round(bme.readTemperature() * 100); // 100 C
    int pressure = (int)round(bme.seaLevelForAltitude(ELEVATION, bme.readPressure())); // Pa
    int pressure_mm = (int)round(pressure * 760 / SEALEVELPRESSURE_HPA); // mm Hg
    int humidity = (int)round(bme.readHumidity() * 100); // 10000 %
    int t_d = (int)(temperature) % 100, t_i = (int)((temperature - t_d) / 100);
    int p_d = (int)(pressure) % 100, p_i = (int)((pressure - p_d) / 100);
    int h_d = (int)(humidity) % 100, h_i = (int)((humidity - h_d) / 100);
    
    char msg[MSG_LEN];
    sprintf(msg, MSG_FORMAT, t_i, t_d, p_i, p_d, h_i, h_d);
#ifdef DEBUG
    Serial.print(F("Temperature [C] = "));
    Serial.println(temperature);
    Serial.print(F("Pressure [hPa] = "));
    Serial.println(pressure);
    Serial.print(F("Pressure [mmHg] = "));
    Serial.println(pressure_mm);
    Serial.print(F("Humidity [%] = "));
    Serial.println(humidity);
    Serial.println();
    Serial.print(F("weather: "));
    Serial.println(msg);
    Serial.println();
#endif
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
  ESP.deepSleep(60 * SECONDS_AS_PICO);
}

void loop()
{
}
