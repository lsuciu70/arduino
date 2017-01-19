/*
 * LsuLog.h
 *
 *  Created on: Jan 19, 2017
 *      Author: lsuciu
 */

#ifndef LSULOG_H_
#define LSULOG_H_

#include "LsuLogger.h"

class LsuLog
{
private:
    static LsuLogger * logger;
public:
    static void setLogger(LsuLogger * logger_t)
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
