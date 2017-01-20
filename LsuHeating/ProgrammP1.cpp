/*
 * ProgrammP1.cpp
 *
 *  Created on: Jan 18, 2017
 *      Author: lsuciu
 */

#include "ProgrammP1.h"
#include "LsuTimeHelper.h"

const String ProgrammP1::START_HOUR_P1 = "start_hour_p1";
const String ProgrammP1::START_MINUTE_P1 = "start_minute_p1";
const String ProgrammP1::STOP_HOUR_P1 = "stop_hour_p1";
const String ProgrammP1::STOP_MINUTE_P1 = "stop_minute_p1";
const String ProgrammP1::TARGET_TEMPERATURE_P1 = "target_temperature_p1";

bool ProgrammP1::shouldRun(int temperature) {
	int target_temperature = target_temperature_p1;
	if (running)
		target_temperature += delta_running_temperature_p1;
	return temperature <= target_temperature
			&& LsuTimeHelper::isNowBetween(start_hour_p1, start_minute_p1,
					stop_hour_p1, stop_minute_p1);
}
