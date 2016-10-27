#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(WiFi.macAddress());
}

void loop() {
}
