/*
 * ProgrammP2.h
 *
 *  Created on: Jan 20, 2017
 *      Author: lsuciu
 */

#ifndef PROGRAMMP2_H_
#define PROGRAMMP2_H_

#include <Arduino.h>

#include "Programm.h"

class ProgrammP2: public Programm
{
public:
    // field name constants
    static const String START_HOUR_P2;
    static const String START_MINUTE_P2;
    static const String TARGET_TEMPERATURE_P2;
private:
    // fields
    int target_temperature_p2;
    byte start_hour_p2;
    byte start_minute_p2;
    int delta_running_temperature_p2;

    // running states
    bool running;
    bool hasRun;
public:
    ProgrammP2(byte start_hour, byte start_minute,
            int delta_running_temperature = 30) :
            target_temperature_p2(0), start_hour_p2(
                    start_hour), start_minute_p2(start_minute), delta_running_temperature_p2(
                    delta_running_temperature), running(false), hasRun(
                    false)
    {
    }
    virtual ~ProgrammP2()
    {
    }

    /**
     * Returns true if the program should run, false otherwise.
     */
    virtual bool shouldRun(int temperature);

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

    /**
     * Returns true if the program has run today, false otherwise.
     */
    virtual bool hasRunToday()
    {
        return hasRun;
    }

    /**
     * Sets the program has run today state.
     */
    virtual void setHasRunToday(bool run_t)
    {
        hasRun = run_t;
    }
};

#endif /* PROGRAMMP2_H_ */
