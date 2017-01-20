/*
 * LsuLog.h
 *
 *  Created on: Jan 19, 2017
 *      Author: lsuciu
 */

#ifndef LSULOG_H_
#define LSULOG_H_

#include "LsuWebLogger.h"

class LsuLog
{
private:
    static LsuWebLogger * logger;
public:
    static void setLogger(LsuWebLogger * logger_t)
    {
        logger = logger_t;
    }
    static void writeLogger(const String &what)
    {
        if (!logger)
            return;
        logger->writeLogger(what);
    }
private:
    LsuLog()
    {
    }
    LsuLog(LsuLog const&);
    void operator=(LsuLog const&);
};

#endif /* LSULOG_H_ */
