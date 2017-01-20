/*
 * ProgrammP3.h
 *
 *  Created on: Jan 20, 2017
 *      Author: lsuciu
 */

#ifndef PROGRAMMP3_H_
#define PROGRAMMP3_H_

#include <Arduino.h>

#include "Programm.h"

class ProgrammP3: public Programm
{
public:
    // field name constant
    static const String TARGET_TEMPERATURE_P3;
private:
    // fields
    int target_temperature_p3;
    int delta_running_temperature_p3;

    // running states
    bool running;
public:
    ProgrammP3(int delta_running_temperature = 30) :
            target_temperature_p3(0), delta_running_temperature_p3(
                    delta_running_temperature), running(false)
    {
    }
    virtual ~ProgrammP3()
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
};

#endif /* PROGRAMMP3_H_ */
