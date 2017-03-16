#include <ESP8266WiFi.h>


void setup() {
  Serial.begin(115200);
  printMacAddress();
}

void loop() {
  Serial.println(F("Scanning available networks..."));
  listNetworks();
  Serial.println();
  delay(10000);
}

void printMacAddress() {
  // the MAC address of your Wifi shield
  byte mac[6];

  // print your MAC address:
  WiFi.macAddress(mac);
  Serial.print(F("MAC: "));
  Serial.print(mac[5], HEX);
  Serial.print(F(":"));
  Serial.print(mac[4], HEX);
  Serial.print(F(":"));
  Serial.print(mac[3], HEX);
  Serial.print(F(":"));
  Serial.print(mac[2], HEX);
  Serial.print(F(":"));
  Serial.print(mac[1], HEX);
  Serial.print(F(":"));
  Serial.println(mac[0], HEX);
}

void listNetworks() {
  // scan for nearby networks:
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println(F("Couldn't get a wifi connection"));
    while (true);
  }

  // print the list of networks seen:
  Serial.print(F("number of available networks:"));
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    int thisEncryptionType = WiFi.encryptionType(thisNet);
    Serial.print(thisNet);
    Serial.print(F(") "));
    Serial.print(WiFi.SSID(thisNet));
    Serial.print(F("\tSignal: "));
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(F(" dBm"));
        Serial.print("\tChannel: ");
        Serial.print(WiFi.channel(thisNet));
    Serial.print(F("\tEncryption: "));
    switch (thisEncryptionType) {
      case ENC_TYPE_WEP:
        Serial.println(F("WEP"));
        break;
      case ENC_TYPE_TKIP:
        Serial.println(F("WPA"));
        break;
      case ENC_TYPE_CCMP:
        Serial.println(F("WPA2"));
        break;
      case ENC_TYPE_NONE:
        Serial.println(F("None"));
        break;
      case ENC_TYPE_AUTO:
        Serial.println(F("Auto"));
        break;
      default:
        Serial.print(F("Unknown: "));
        Serial.println(thisEncryptionType);
        break;
    }
  }
}

