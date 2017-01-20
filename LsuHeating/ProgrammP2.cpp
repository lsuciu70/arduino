/*
 * ProgrammP2.cpp
 *
 *  Created on: Jan 20, 2017
 *      Author: lsuciu
 */

#include "ProgrammP2.h"
#include "LsuTimeHelper.h"

const String ProgrammP2::START_HOUR_P2 = "start_hour_p2";
const String ProgrammP2::START_MINUTE_P2 = "start_minute_p2";
const String ProgrammP2::TARGET_TEMPERATURE_P2 = "target_temperature_p2";

bool ProgrammP2::shouldRun(int temperature)
{
    bool isfter = LsuTimeHelper::isNowAfter(start_hour_p2, start_minute_p2);
    if (!isfter)
        hasRun = false;
    bool shouldRun = !hasRun && isfter;
    if (shouldRun && target_temperature_p2 == 0)
        target_temperature_p2 = temperature + delta_running_temperature_p2;
    shouldRun = shouldRun && temperature <= target_temperature_p2;
    if (!shouldRun && running)
    {
        hasRun = true;
        target_temperature_p2 = 0;
    }
    return shouldRun;
}
