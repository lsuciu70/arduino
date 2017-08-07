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

#ifndef DEBUG
#define DEBUG 1
#endif


namespace
{


const uint8_t SSID_SIZE = 3;
uint8_t ssid_size = SSID_SIZE;
char* SSID_t[SSID_SIZE] =
{ "cls-router", "cls-ap", "lsu-tpr" };
char* PASSWD_t[SSID_SIZE] =
{ "r4cD7TPG", "r4cD7TPG", "r4cD7TPG" };

bool initialized = false;
bool defaultSsids = true;
}

namespace LsuWiFi
{

/**
 * Adds a new WiFi AP. Maximum three APs can be added. Returns true on success,
 * false on failure (trying to add more then three).
 * @params:
 *   ssid - the AP SSID
 *   password - the password, for no password use empty string
 */
bool addAp(const char *ssid, const char *password)
{
  if (!initialized && (initialized = true))
      WiFi.mode(WIFI_STA);
  if(defaultSsids)
  {
    defaultSsids = false;
    ssid_size = 0;
  }
  if(ssid_size >= SSID_SIZE)
    return false;
  SSID_t[ssid_size] = (char*) malloc((strlen(ssid) + 1) * sizeof(char));
  strcpy(SSID_t[ssid_size], ssid);
  PASSWD_t[ssid_size] = (char*) malloc((strlen(password) + 1) * sizeof(char));
  strcpy(PASSWD_t[ssid_size], password);
  ++ssid_size;
  return true;
}

/**
 * Returns true if WiFi is connected, false otherwise.
 */
inline bool isConnected()
{
  return (WiFi.status() == WL_CONNECTED);
}

/**
 * Connects to one of LSU WiFi networks.
 * @params:
 *   ssid_idx - the index to try, default 0 (first one)
 *   timeout - the timeout (in milliseconds) before giving up on current index, default 10000
 *   retry_after_timeout - flag indicating if should retry after timeout, default true
 *   try_next_ssid - flag indicating to try next index on retry, default true
 *
 */
bool connect(const uint8_t ssid_idx = 0, const uint16_t timeout = 10000,
    const bool retry_after_timeout = true, const bool try_next_ssid = true, const uint8_t try_index = 1)
{
  if (!initialized && (initialized = true))
    WiFi.mode(WIFI_STA);
  if (isConnected())
    return true;
  uint8_t ssid_ix = ssid_idx % ssid_size;
  const char* ssid = SSID_t[ssid_ix];
  const char* passwd = PASSWD_t[ssid_ix];
#if DEBUG
  Serial.print(F("WiFi: connecting to "));
  Serial.println(ssid);
#endif
  if(strlen(passwd))
    WiFi.begin(ssid, passwd);
  else
    WiFi.begin(ssid);
  unsigned long mllis = millis();
  byte count = 1;
  while (!isConnected())
  {
    delay(10);
    if (millis() - mllis >= timeout)
    {
#if DEBUG
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
      if (!retry_after_timeout && try_index == ssid_size)
        // do not retry, and tried all available
        return false;
      if (try_next_ssid)
        ssid_ix += 1;
      return connect(ssid_ix, timeout, retry_after_timeout,
          try_next_ssid, try_index + 1);
    }
#if DEBUG
    if ((++count) % 10 == 0)
      Serial.print(". ");
    if (count == 100)
      Serial.println();
#endif
    count %= 100;
  }
#if DEBUG
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

}
#endif /* LSUWIFIC_H_ */
