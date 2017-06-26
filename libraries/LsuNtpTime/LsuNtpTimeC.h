/*
 * LsuNtpTimeC.h
 *
 *  Created on: Apr 17, 2017
 *      Author: lsuciu
 */

#ifndef LSUNTPTIMEC_H_
#define LSUNTPTIMEC_H_

#include <Arduino.h>

#include <Time.h>         // https://github.com/PaulStoffregen/Time
#include <Timezone.h>     // https://github.com/JChristensen/Timezone

#include <WiFiUdp.h>      // arduino_ide/libraries/WiFi/src

namespace
{
time_t getTime()
{
  if (!(WiFi.status() == WL_CONNECTED))
  {
    Serial.println(F("NTP ERROR: WiFi not connected."));
    return 0;
  }
  WiFiUDP udp;
  const byte POLL_INTERVAL = 10; // poll every this many ms
  const byte POLL_TIMES = 100;  // poll up to this many times
  const byte PKT_LEN = 48; // NTP packet length
  const byte USELES_BYTES = 40; // Useless bytes to be discarded; set useless to 32 for speed; set to 40 for accuracy.

  // Eastern European Time (Timisoara)
  const TimeChangeRule EEST =
  { "EEST", Last, Sun, Mar, 3, 180 };  // Eastern European Summer Time
  const TimeChangeRule EET =
  { "EET", Last, Sun, Oct, 4, 120 };   // Eastern European Standard Time
  Timezone EasternEuropeanTime(EEST, EET);

  TimeChangeRule *tcr;

  unsigned long mllis = millis();

  // Initializes the WiFi UDP library and network settings. Starts WiFiUDP socket, listening at local port 12670
  static int udpInitialized = udp.begin(12670);
  if (0 == udpInitialized) // returns 0 if there are no sockets available to use
  {
    Serial.println(F("NTP ERROR: there are no sockets available to use."));
    return 0;
  }
  static char timeServer[] = "ro.pool.ntp.org";  // the NTP server

  static long ntpFirstFourBytes = 0xEC0600E3; // the NTP request header

  udp.flush(); // Clear received data from possible stray received packets

  // Send an NTP request to timeserver on NTP port: 123
  if (!(udp.beginPacket(timeServer, 123)
      && udp.write((const uint8_t *) &ntpFirstFourBytes, (size_t) PKT_LEN)
          == PKT_LEN && udp.endPacket()))
  {
    Serial.println(F("NTP ERROR: sending request failed"));
    return 0; // sending request failed
  }

  int pktLen;               // received packet length
  // Wait for NTP server response; check every POLL_INTERVAL ms up to POLL_TIMES times
  byte j = 0;
  for (; j < POLL_TIMES; j++)
  {
    if ((pktLen = udp.parsePacket()) == PKT_LEN)
      break;
    delay((unsigned long) POLL_INTERVAL);
  }
  if (pktLen != PKT_LEN)
  {
    Serial.print(F("NTP ERROR: no correct packet received; pktLen = "));
    Serial.print(pktLen);
    Serial.println(F(", expected 48"));
    return getTime(); // no correct packet received
  }

  // Read and discard the first useless bytes
  for (byte i = 0; i < USELES_BYTES; ++i)
    udp.read();

  // Read the integer part of sending time
  time_t t_time = udp.read();  // NTP time
  for (byte i = 1; i < 4; i++)
    t_time = t_time << 8 | udp.read();

  // Round to the nearest second if we want accuracy
  // The fractionary part is the next byte divided by 256: if it is
  // greater than 500ms we round to the next second; we also account
  // for an assumed network delay of 30ms, and (0.5-0.05)*256=120;
  // additionally, we account for how much we delayed reading the packet
  // since its arrival, which we assume on average to be POLL_INTERVAL/2.
  t_time += (udp.read() > 120 - POLL_INTERVAL / 8);

  // Discard the rest of the packet
  udp.flush();

  // convert NTP time to GMT time
  t_time -= 2208988800ul;
  // convert Unix to locale (EET | EEST)
  t_time = EasternEuropeanTime.toLocal(t_time, &tcr);
  return t_time;
}
}

namespace LsuNtpTime
{
/** "DD-MM-YYYY" */
const byte dateStringLength = 10;

/** "hh:mm:ss" */
const byte timeStringLength = 8;

/** "DD-MM-YYYY hh:mm:ss" */
const byte datetimeStringLength = dateStringLength + 1 + timeStringLength;

/**
 * Starts the NTP time library, and waits for first synch.
 * Sets the sync interval in seconds, default 6 hours.
 */
void begin(time_t seconds = (6 * 3600))
{
  // Set the external time provider
  setSyncProvider(getTime);
  // Set synch interval to one second till sync
  setSyncInterval(2);
  while (timeStatus() != timeSet)
  {
    ;
  }
  // Set synch interval
  setSyncInterval(seconds);
}

/**
 * Fills and returns the input datetime buffer as "DD-MM-YYYY hh:mm:ss".
 * The buffer should be at least (datetimeStringLength + 1) in size.
 */
char* datetimeString(char* datetimebuff, int day_t = day(), int month_t =
    month(), int year_t = year(), int hour_t = hour(), int minute_t = minute(),
    int second_t = second())
{
  sprintf(datetimebuff, "%02d-%02d-%04d %02d:%02d:%02d", day_t, month_t, year_t,
      hour_t, minute_t, second_t);
  return datetimebuff;
}

/**
 * Returns the date and time as "DD-MM-YYYY hh:mm:ss".
 */
char* datetimeString(int day_t = day(), int month_t = month(), int year_t =
    year(), int hour_t = hour(), int minute_t = minute(), int second_t =
    second())
{
  static char datetimebuff[datetimeStringLength + 1];
  sprintf(datetimebuff, "%02d-%02d-%04d %02d:%02d:%02d", day_t, month_t, year_t,
      hour_t, minute_t, second_t);
  return datetimebuff;
}

/**
 * Fills and returns the input date buffer as "DD-MM-YYYY"
 * The buffer should be at least (dateStringLength + 1) in size.
 */
char* dateString(char* datebuff, int day_t = day(), int month_t = month(),
    int year_t = year())
{
  sprintf(datebuff, "%02d-%02d-%04d", day_t, month_t, year_t);
  return datebuff;
}

/**
 * Returns the date as "DD-MM-YYYY"
 * The buffer should be at least (dateStringLength + 1) in size.
 */
char* dateString(int day_t = day(), int month_t = month(), int year_t = year())
{
  static char datebuff[dateStringLength + 1];
  sprintf(datebuff, "%02d-%02d-%04d", day_t, month_t, year_t);
  return datebuff;
}

/**
 * Fills and returns the input time buffer as "hh:mm:ss"
 * The buffer should be at least (timeStringLength + 1) in size.
 */
char* timeString(char* timebuff, int hour_t = hour(), int minute_t = minute(),
    int second_t = second())
{
  sprintf(timebuff, "%02d:%02d:%02d", hour_t, minute_t, second_t);
  return timebuff;
}

/**
 * Returns the time as "hh:mm:ss"
 */
char* timeString(int hour_t = hour(), int minute_t = minute(), int second_t =
    second())
{
  static char timebuff[timeStringLength + 1];
  sprintf(timebuff, "%02d:%02d:%02d", hour_t, minute_t, second_t);
  return timebuff;
}

}

#endif /* LSUNTPTIMEC_H_ */
