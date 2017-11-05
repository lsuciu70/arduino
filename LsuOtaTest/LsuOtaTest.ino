#define LSU_OTA_DEBUG 1
#include <LsuOta.h>

const uint8_t version = 6;

void setup()
{
  Serial.begin(115200);
  LsuOta::begin((version + 1));
  Serial.printf("\nStarted; application version: %d\n", version);
  Serial.printf("OTA next version URL: %s\n", LsuOta::otaUrl());
}

void loop()
{
  LsuOta::loop();
}
