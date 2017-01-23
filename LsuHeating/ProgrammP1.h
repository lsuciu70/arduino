/*
 * ProgrammP1.h
 *
 *  Created on: Jan 18, 2017
 *      Author: lsuciu
 */

#ifndef PROGRAMMP1_H_
#define PROGRAMMP1_H_

#include <Arduino.h>

#include "Programm.h"

class ProgrammP1 : public Programm
{
public:
    // field name constants
    static const String START_HOUR_P1;
    static const String START_MINUTE_P1;
    static const String STOP_HOUR_P1;
    static const String STOP_MINUTE_P1;
    static const String TARGET_TEMPERATURE_P1;
private:
    // fields
    int target_temperature_p1;
    byte start_hour_p1;
    byte start_minute_p1;
    byte stop_hour_p1;
    byte stop_minute_p1;
    int delta_running_temperature_p1;

    // running states
    bool running;

    char asString[70];
public:
	ProgrammP1(int target_temperature = 2100, byte start_hour = 15,
            byte start_minute = 0, byte stop_hour = 7, byte stop_minute = 0,
            int delta_running_temperature = 10) :
            target_temperature_p1(target_temperature), start_hour_p1(
                    start_hour), start_minute_p1(start_minute), stop_hour_p1(
                    stop_hour), stop_minute_p1(stop_minute), delta_running_temperature_p1(
                    delta_running_temperature), running(false)
    {
        int start = start_hour_p1 + start_minute_p1 * 60;
        int stop = stop_hour_p1 + stop_minute_p1 * 60;
        if (start == stop)
            sprintf(asString, "P1 - merge continuu si face %.2f &deg;C",
                    (1.0 * target_temperature_p1 / 100));
        else if (start > stop)
            sprintf(asString,
                    "P1 - merge intre  %d:%02d si %d:%02d (ziua urmatoare) si face %.2f &deg;C",
                    start_hour_p1, start_minute_p1, stop_hour_p1,
                    stop_minute_p1, (1.0 * target_temperature_p1 / 100));
        else
            sprintf(asString,
                    "P1 - merge intre  %d:%02d si %d:%02d si face %.2f &deg;C",
                    start_hour_p1, start_minute_p1, stop_hour_p1,
                    stop_minute_p1, (1.0 * target_temperature_p1 / 100));
    }
    virtual ~ProgrammP1()
    {
	}

    /**
     * Returns true if the program should run, false otherwise.
     */
    virtual bool shouldRun(int temperature);

    virtual char * toString()
    {
        return asString;
    }

    /**
     * Returns true if the program is running, false otherwise.
     */
    virtual bool isRunning()
    {
        return running;
    }

    /**
     * Sets the running state.
     */
    virtual void setRunning(bool running_t)
    {
        running = running_t;
    }
};

#endif /* PROGRAMMP1_H_ */
