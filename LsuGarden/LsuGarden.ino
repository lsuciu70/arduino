#include <Arduino.h>
#include <pins_arduino.h>
#include <ESP8266WiFi.h>

#include <LsuNtpTimeC.h>

#define MINUES_A_DAY (24 * 60)
#define MINUTES_A_WEEK (7 * MINUES_A_DAY)

#define OFF HIGH
#define ON  LOW

enum days : uint8_t
{
  MO = 1, TU, WE, TH, FR, SA, SU,
};

uint8_t out = OFF;
const uint8_t NB_PINS = 8;
uint8_t pin[NB_PINS] =
{
	D1, D2, D3, D4, D5, D6, D7, D8,
};

// time helper functions
uint8_t day_of_week();
uint16_t week_minutes_to_time(uint8_t, uint8_t, uint8_t);
uint16_t now_to_week_minutes(uint16_t);
uint16_t now_as_week_minutes();

// "DD hh:mm"
const uint8_t week_minute_str_len = strlen("DD hh:mm");
char* week_minute_to_str(char*, uint16_t);

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
 * Returns 10080 (minutes in a week) on error.
 */
uint16_t week_minutes_to_time(uint8_t day, uint8_t hour, uint8_t minute)
{
  // day should be between Monday and Sunday
  if(day < MO || day > SU)
    return MINUTES_A_WEEK;
  // hour should be between 0 and 23
  if(hour > 23)
    return MINUTES_A_WEEK;
  // minute should be between 0 and 59
  if(minute > 59)
    return MINUTES_A_WEEK;
  return (day - 1) * MINUES_A_DAY + hour * 60 + minute;
}

/**
 * Returns day of week, 1 to 7; Monday being 1, Sunday 7.
 */
uint8_t day_of_week()
{
  return ((weekday() + 5) % 7) + 1;
}

/**
 * Fills and returns given buffer with string representation of given week
 * minute as DD hh:mm.
 * The buffer must be at least (week_minute_str_len + 1) in length.
 *
 * Example call, returns "Lu 20:40":
 * char *buff[week_minute_str_len + 1]
 * week_minute_to_str(buff, 1240);
 *
 */
char* week_minute_to_str(char *buff, uint16_t week_minutes)
{

  return buff;
}
