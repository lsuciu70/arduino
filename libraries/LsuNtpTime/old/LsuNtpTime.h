/*
 * LsuNtpTime.h
 *
 *  Created on: Jan 19, 2017
 *      Author: lsuciu
 */

#ifndef LSUNTPTIME_H_
#define LSUNTPTIME_H_

#include <Arduino.h>

#include <Time.h>         // https://github.com/PaulStoffregen/Time
#include <Timezone.h>     // https://github.com/JChristensen/Timezone

#include <LsuWiFi.h>      // https://github.com/lsuciu70/arduino/tree/master/libraries/LsuWiFi

extern time_t getTime();

class LsuNtpTime
{
public:
  /**
   * One second, 1000 milliseconds.
   */
  static constexpr int SECOND = 1000;
  /**
   * The length of date and time String (DD-MM-YYYY hh:mm:ss): 19
   */
  static constexpr byte datetimeStringLength = 19;
  /**
   * The length of date String (DD-MM-YYYY): 10
   */
  static constexpr byte dateStringLength = 10;
  /**
   * The length of time String (hh:mm:ss): 8
   */
  static constexpr byte timeStringLength = 8;
private:
  static bool timeAvailable;
  static unsigned long startSecond;
  static bool disconnectAfterSynch;
  static const char* UNSET;
public:
  /**
   * Starts the library, makes synch every given seconds, default one hour.
   * By default disconnects WiFi unless disconnect is true.
   */
  static void start(time_t seconds = 3600, bool disconnect = true);

  /**
   * Returns the date and time as "DD-MM-YYYY hh:mm:ss"
   */
  static const char * datetimeString(int day_t = day(), int month_t = month(),
      int year_t = year(), int hour_t = hour(), int minute_t = minute(),
      int second_t = second());

  /**
   * Returns the date as "DD-MM-YYYY"
   */
  static const char * dateString(int day_t = day(), int month_t = month(),
      int year_t = year());

  /**
   * Returns the time as "hh:mm:ss"
   */
  static const char * timeString(int hour_t = hour(), int minute_t = minute(),
      int second_t = second());

  static inline bool isTimeAvailable()
  {
    return timeAvailable;
  }

  static inline unsigned long getStartSecond()
  {
    return startSecond;
  }

  static inline unsigned long secondsRunning()
  {
    return millis() / SECOND;
  }
private:
  LsuNtpTime()
  {
  }
  LsuNtpTime(LsuNtpTime const&);
  void operator=(LsuNtpTime const&);

  friend time_t getTime();
private:
  class TimeAvailableSetter
  {
  private:
    bool ta;
  public:
    TimeAvailableSetter(bool ta_t) :
        ta(ta_t)
    {
    }
    ~TimeAvailableSetter()
    {
      timeAvailable = ta;
    }
  };
};

#endif /* LSUNTPTIME_H_ */
