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
    bool changed = false;
    if (target_temperature_p3 == 0)
    {
        target_temperature_p3 = temperature + delta_running_temperature_p3;
        changed = true;
    }
    bool shouldRun = temperature <= target_temperature_p3;
    if (!shouldRun && running)
    {
        target_temperature_p3 = 0;
        changed = true;
    }
    if (changed && target_temperature_p3 == 0)
    sprintf(asString,
            "P3 - porneste acum si face temperatura de la pornire + %.2f &deg;C",
            (1.0 * delta_running_temperature_p3 / 100));
    if (changed && target_temperature_p3 != 0)
        sprintf(asString,
                "P3 - porneste acum si face %.2f &deg;C",
                (1.0 * target_temperature_p3 / 100));
    return shouldRun;
}
