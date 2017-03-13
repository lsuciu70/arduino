/*
 * LsuWiFi.h
 *
 *  Created on: Jan 19, 2017
 *      Author: lsuciu
 */

#ifndef LSUWIFI_H_
#define LSUWIFI_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino

class LsuWiFi
{
public:
  // constants
  static const IPAddress NO_IP;
private:
  // constants
  static const byte SSID_SIZE;
  static const char* SSID_t[];
  static const char* PASSWD_t[];
  static byte ssid_ix;

  static bool initialized;
private:
  LsuWiFi()
  {
  }
  LsuWiFi(LsuWiFi const&);
  void operator=(LsuWiFi const&);
  static inline bool init()
  {
    WiFi.mode(WIFI_STA);
    return true;
  }
public:
  static void connect();
  static inline String macAddress()
  {
    return WiFi.macAddress();
  }
};

#endif /* LSUWIFI_H_ */
