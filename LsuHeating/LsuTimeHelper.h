/*
 * LsuTimeHelper.h
 *
 *  Created on: Jan 18, 2017
 *      Author: lsuciu
 */

#ifndef LSUTIMEHELPER_H_
#define LSUTIMEHELPER_H_

#include <Arduino.h>

#include <Time.h>         // https://github.com/PaulStoffregen/Time

class LsuTimeHelper {
private:
	LsuTimeHelper() {}
	LsuTimeHelper(LsuTimeHelper const&);
	void operator=(LsuTimeHelper const&);
public:
	static bool isNowBetween(byte start_h, byte start_m, byte stop_h, byte stop_m)
	{
	  int minutes_start = start_h * 60 + start_m;
	  int minutes_end = stop_h * 60 + stop_m;
	  // check continuous running
	  if (minutes_start == minutes_end)
	    return true;
	  // next day
	  bool next_day = minutes_end < minutes_start;
	  int minutes_now = hour() * 60 + minute();
	  if (next_day)
	    return minutes_start <= minutes_now || minutes_now < minutes_end;
	  else
	    return minutes_start <= minutes_now && minutes_now < minutes_end;
	  return false;
	}

	static bool isNowAfter(byte start_h, byte start_m)
	{
	  int minutes_now = hour() * 60 + minute();
	  int minutes_start = start_h * 60 + start_m;
	  return minutes_start <= minutes_now;
	}
};

#endif /* LSUTIMEHELPER_H_ */
