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

class LsuNtpTime {
public:
    static const int SECOND;
private:
    static bool timeAvailable;
    static unsigned long startSecond;
public:
    static void start(time_t seconds = 3600);
    static char * timeString(int day_t = day(), int month_t = month(),
            int year_t = year(), int hour_t = hour(), int minute_t = minute(),
            int second_t = second());
    static bool isTimeAvailable()
    {
        return timeAvailable;
    }
    static unsigned long getStartSecond()
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
    class NoTimeAvailableSetter
    {
    private:
        bool nt;
    public:
        NoTimeAvailableSetter(bool nt_t) :
                nt(nt_t)
        {
        }
        ~NoTimeAvailableSetter()
        {
            timeAvailable = nt;
        }
    };
};

#endif /* LSUNTPTIME_H_ */
