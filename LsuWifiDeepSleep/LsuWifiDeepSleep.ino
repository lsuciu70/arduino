#include <LsuWiFi.h>
#include <Esp.h>

void setup()
{
  Serial.begin(115200);
  Serial.println(F("setup ..."));

  LsuWiFi::connect();

  Serial.println(F("ESP8266 in sleep mode"));
  ESP.deepSleep(30 * 1000);

  // TODO: who will wake it up?
}

void loop()
{
  Serial.println(F("loop ..."));
  delay(5000);
}
