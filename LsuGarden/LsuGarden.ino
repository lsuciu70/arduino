#include <Arduino.h>
#include <pins_arduino.h>
#include <ESP8266WiFi.h>

#include <LsuNtpTimeC.h>

#define MINUES_A_DAY (24 * 60)
#define MINUTES_A_WEEK (7 * MINUES_A_DAY)

enum
{
  MO = 1, TU, WE, TH, FR, SA, SU
};

#define OFF HIGH
#define ON  LOW

uint8_t out = OFF;
const uint8_t NB_PINS = 8;
uint8_t pin[NB_PINS] =
{
	D1, D2, D3, D4, D5, D6, D7, D8
};

// time helper functions
uint8_t day_of_week();
uint16_t week_minutes_to_time(uint8_t, uint8_t, uint8_t);
uint16_t now_to_week_minutes(uint16_t);
uint16_t now_as_week_minutes();

void setup()
{
  for(uint8_t i = 0; i < NB_PINS; ++i)
  {
  	pinMode(pin[i], OUTPUT);
  	digitalWrite(pin[i], OFF);
  }
  out = OFF;
}

void loop()
{
//  delay(5000);
//  out = (++out) % 2;
//  for(uint8_t i = 0; i < NB_PINS; ++i)
//  {
//  	digitalWrite(pin[i], out);
//  }
}

uint16_t now_as_week_minutes()
{
  return week_minutes_to_time(day_of_week(), hour(), minute());
}

/**
 * Returns number of minutes between now and given week minutes.
 */
uint16_t now_to_week_minutes(uint16_t week_minutes)
{
  uint16_t now_week_minutes = now_as_week_minutes();
  if(week_minutes < now_week_minutes)
    // will be next week
    week_minutes += MINUTES_A_WEEK;
  return (week_minutes - now_week_minutes) % MINUTES_A_WEEK;
}

/**
 * Returns the minutes from beginning of week (Monday 0:00) till given day of
 * week, hour and minute.
 *
 * Example call for Thursday, 20:45:
 * uint16_t week_minutes = week_minutes_to_time(TH, 20, 45);
 *
 */
uint16_t week_minutes_to_time(uint8_t day, uint8_t hour, uint8_t minute)
{
  // day should be between 1 and 7
  if(day == 0 || day > 7)
    return 0;
  // hour should be between 0 and 23
  if(hour > 23)
    return 0;
  // minute should be between 0 and 59
  if(minute > 59)
    return 0;
  return (day - 1) * MINUES_A_DAY + hour * 60 + minute;
}

/**
 * Returns day of week, 1 to 7; Monday being 1, Sunday 7.
 */
uint8_t day_of_week()
{
  return ((weekday() + 5) % 7) + 1;
}
