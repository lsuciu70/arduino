/*
 * ProgrammP1.h
 *
 *  Created on: Jan 18, 2017
 *      Author: lsuciu
 */

#ifndef PROGRAMMP1_H_
#define PROGRAMMP1_H_

#include <Arduino.h>

class ProgrammP1 {
public:
	// constants
	static const String START_HOUR_P1;
	static const String START_MINUTE_P1;
	static const String STOP_HOUR_P1;
	static const String STOP_MINUTE_P1;
	static const String TARGET_TEMPERATURE_P1;
public:
	ProgrammP1(int target_temperature = 2100, byte start_hour = 15,
			byte start_minute = 0, byte stop_hour = 7, byte stop_minute = 0, int delta_running_temperature = 10) :
			target_temperature_p1(target_temperature), start_hour_p1(
					start_hour), start_minute_p1(start_minute), stop_hour_p1(
					stop_hour), stop_minute_p1(stop_minute), delta_running_temperature_p1(delta_running_temperature), running(false) {
	}
	virtual ~ProgrammP1() {
	}

	bool shouldRun(int temperature);
private:
	// fields
	int target_temperature_p1;
	byte start_hour_p1;
	byte start_minute_p1;
	byte stop_hour_p1;
	byte stop_minute_p1;
	int delta_running_temperature_p1;

	// running states
	bool running;
};

#endif /* PROGRAMMP1_H_ */
