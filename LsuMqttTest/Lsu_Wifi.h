/*
 * Lsu_Wifi.h
 *
 *  Created on: Mar 19, 2017
 *      Author: lsuciu
 */

#ifndef LSU_WIFI_H_
#define LSU_WIFI_H_

#include <ESP8266WiFi.h>

namespace
{
uint8_t SSID_SIZE = 2;
const char* SSID_t[] =
{ "cls-router", "cls-ap" };

const char* PASSWD_t[] =
{ "r4cD7TPG", "r4cD7TPG" };
}

void lsuWiFiConnect(uint8_t ssid_ix = 0, uint16_t timeout = 5000)
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
    if (millis() - mllis >= timeout)
    {
      if (count)
        Serial.println();
      Serial.println("Not connected; 10 s timed out. Trying next SSID.");
      return lsuWiFiConnect(ssid_ix + 1, timeout);
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
          + "\n MAC: " + WiFi.macAddress() + "\n IP:  "
          + String(WiFi.localIP().toString()));
}

#endif /* LSU_WIFI_H_ */
