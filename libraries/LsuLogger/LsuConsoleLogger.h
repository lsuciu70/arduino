/*
 * LsuConsoleLogger.h
 *
 *  Created on: Jan 20, 2017
 *      Author: lsuciu
 */

#ifndef LSUCONSOLELOGGER_H_
#define LSUCONSOLELOGGER_H_

#include <Arduino.h>

#include <LsuLogger.h>

class LsuConsoleLogger: public LsuLogger
{
public:
    LsuConsoleLogger()
    {
    }
    virtual ~LsuConsoleLogger()
    {
    }

    virtual void writeLogger(const String &what)
    {
        Serial.println(what);
    }
};

#endif /* LSUCONSOLELOGGER_H_ */
