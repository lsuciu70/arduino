/*
 * ProgrammP3.cpp
 *
 *  Created on: Jan 20, 2017
 *      Author: lsuciu
 */

#include "ProgrammP3.h"
#include "LsuTimeHelper.h"

const String ProgrammP3::TARGET_TEMPERATURE_P3 = "target_temperature_p3";

bool ProgrammP3::shouldRun(int temperature)
{
    if (target_temperature_p3 == 0)
        target_temperature_p3 = temperature + delta_running_temperature_p3;
    bool shouldRun = temperature <= target_temperature_p3;
    if (!shouldRun && running)
    {
        target_temperature_p3 = 0;
    }
    return shouldRun;
}
