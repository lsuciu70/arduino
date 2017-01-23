/*
 * Programm.h
 *
 *  Created on: Jan 19, 2017
 *      Author: lsuciu
 */

#ifndef PROGRAMM_H_
#define PROGRAMM_H_

#include <Arduino.h>

class Programm
{
public:
    Programm()
    {
    }
    virtual ~Programm()
    {
    }

    virtual bool shouldRun(int temperature) = 0;
    virtual bool isRunning() = 0;
    virtual void setRunning(bool) = 0;

    virtual char * toString() = 0;
};

#endif /* PROGRAMM_H_ */
