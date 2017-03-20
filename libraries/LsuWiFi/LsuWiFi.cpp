/*
 * LsuWiFi.cpp
 *
 *  Created on: Jan 19, 2017
 *      Author: lsuciu
 */

#include <LsuWiFi.h>

const IPAddress LsuWiFi::NO_IP = IPAddress(0, 0, 0, 0);

const byte LsuWiFi::SSID_SIZE = 2;

const char* LsuWiFi::SSID_t[] =
{ "cls-router", "cls-ap" };

const char* LsuWiFi::PASSWD_t[] =
{ "r4cD7TPG", "r4cD7TPG" };

byte LsuWiFi::ssid_ix = 0;

bool LsuWiFi::initialized = init();

void LsuWiFi::connect()
{
  if (WiFi.status() == WL_CONNECTED)
    return;
  ssid_ix = ssid_ix % SSID_SIZE;
  const char* ssid = SSID_t[ssid_ix];
  const char* passwd = PASSWD_t[ssid_ix];

  Serial.print("WiFi: connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, passwd);
  unsigned long mllis = millis();
  byte count = 1;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(10);
    if (millis() - mllis >= 10000)
    {
      if (count)
        Serial.println();
      Serial.println("Not connected; 10 s timed out. Trying next SSID.");
      ssid_ix += 1;
      return connect();
    }
    if ((++count) % 10 == 0)
    {
      Serial.print(". ");
    }
    if (count == 100)
    {
      Serial.println();
      count = 0;
    }
  }
  if (count)
    Serial.println();
  Serial.println(
      String("Connected, took ") + (1.0 * (millis() - mllis) / 1000) + " s;"
          + "\n MAC: " + macAddress() + "\n IP:  "
          + String(WiFi.localIP().toString()));
}
