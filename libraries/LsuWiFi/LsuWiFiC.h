/*
 * LsuWiFiC.h
 *
 *  Created on: Mar 20, 2017
 *      Author: lsuciu
 */

#ifndef LSUWIFIC_H_
#define LSUWIFIC_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino

namespace
{

#ifndef DEBUG
#define DEBUG
#endif

const uint8_t SSID_SIZE = 3;

const char* SSID_t[] =
{ "cls-router", "cls-ap", "lsu-tpr" };

const char* PASSWD_t[] =
{ "r4cD7TPG", "r4cD7TPG", "r4cD7TPG" };

bool initialized = false;
}

/**
 * Connects to one of LSU WiFi networks.
 * ssid_idx - the index to try, default 0 (first one)
 * timeout - the timeout (in milliseconds) before giving up on current index, default 5000
 * retry_after_timeout - flag indicating if should retry after timeout
 * try_next_ssid - flag indicating to try next index on retry
 */
bool connectLsuWiFi(const uint8_t ssid_idx = 0, const uint16_t timeout = 5000,
    const bool retry_after_timeout = true, const bool try_next_ssid = true, const uint8_t try_index = 1)
{
  if (!initialized && (initialized = true))
    WiFi.mode(WIFI_STA);
  if (WiFi.status() == WL_CONNECTED)
    return true;
  uint8_t ssid_ix = ssid_idx % SSID_SIZE;
  const char* ssid = SSID_t[ssid_ix];
  const char* passwd = PASSWD_t[ssid_ix];
#ifdef DEBUG
  Serial.print(F("WiFi: connecting to "));
  Serial.println(ssid);
#endif
  WiFi.begin(ssid, passwd);
  unsigned long mllis = millis();
  byte count = 1;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(10);
    if (millis() - mllis >= timeout)
    {
#ifdef DEBUG
      if (count)
        Serial.println();
      if (retry_after_timeout)
      {
        Serial.print(F("Not connected; "));
        Serial.print(timeout / 1000);
        Serial.println(F(" s timed out. Trying next SSID."));
      }
      else
      {
        Serial.print(F("Not connected; "));
        Serial.print(timeout / 1000);
        Serial.println(F(" s timed out. No retry."));
      }
#endif
      if (!retry_after_timeout && try_index == SSID_SIZE)
        // do not retry, and tried all available
        return false;
      if (try_next_ssid)
        ssid_ix += 1;
      return connectLsuWiFi(ssid_ix, timeout, retry_after_timeout,
          try_next_ssid, try_index + 1);
    }
#ifdef DEBUG
    if ((++count) % 10 == 0)
      Serial.print(". ");
    if (count == 100)
      Serial.println();
#endif
    count %= 100;
  }
#ifdef DEBUG
  if (count)
    Serial.println();
  Serial.print(F("Connected, took "));
  Serial.print(1.0 * (millis() - mllis) / 1000);
  Serial.println(F(" s;"));
  Serial.print(F("  MAC: "));
  Serial.println(WiFi.macAddress());
  Serial.print(F("  IP:  "));
  Serial.println(WiFi.localIP().toString());
#endif
  return true;
}

#endif /* LSUWIFIC_H_ */
