#include <LsuWiFi.h>

void setup() {
  Serial.begin(115200);
  LsuWiFi::begin(WIFI_STA);
  char mac[MAC_ADDR_STR_LEN + 1];
  LsuWiFi::macAddressStr(mac);
  Serial.println(mac);
}

void loop() {
}
