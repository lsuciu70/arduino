#include <LsuWiFi.h>
#include <Esp.h>

#define DEBUG 1

#define SECONDS_AS_PICO (1000000)
#define SECONDS_AS_NANO (1000)

void setup()
{
  Serial.begin(115200);

  Serial.println(F("\n"));
  Serial.println(millis());
  Serial.println(F("setup ..."));

  LsuWiFi::connect();

  Serial.println(F("Wait 10 seconds"));
  delay(10 * SECONDS_AS_NANO);

  Serial.println(F("Deep sleep 30 seconds"));
  ESP.deepSleep(30 * SECONDS_AS_PICO);
}

void loop()
{
  Serial.println(F("loop ..."));
  delay(5 * SECONDS_AS_NANO);
}
