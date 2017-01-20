/*
 * LsuLogger.h
 *
 *  Created on: Jan 20, 2017
 *      Author: lsuciu
 */

#ifndef LSULOGGER_H_
#define LSULOGGER_H_

#include <Arduino.h>

class LsuLogger
{
public:
    LsuLogger()
    {
    }
    virtual ~LsuLogger()
    {
    }

    virtual void writeLogger(const String &) = 0;
};

#endif /* LSULOGGER_H_ */
