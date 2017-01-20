/*
 * ProgrammP0.h
 *
 *  Created on: Jan 20, 2017
 *      Author: lsuciu
 */

#ifndef PROGRAMMP0_H_
#define PROGRAMMP0_H_

#include <Arduino.h>

#include "Programm.h"

class ProgrammP0: public Programm
{
public:
    ProgrammP0();
    virtual ~ProgrammP0();

    virtual inline bool shouldRun(int doesnt_matter)
    {
        return false;
    }
    virtual inline bool isRunning()
    {
        return false;
    }
    virtual inline void setRunning(bool doesnt_matter)
    {
    }
};

#endif /* PROGRAMMP0_H_ */
