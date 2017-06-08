#include <Arduino.h>
#include <pins_arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#include <LsuNtpTimeC.h>

enum days
  : uint8_t
  {
    MO = 0, TU, WE, TH, FR, SA, SU, DAYS_A_WEEK,
};

#define HOURS_A_DAY    24
#define MINUTES_A_HOUR 60
#define MINUTES_A_DAY (HOURS_A_DAY * MINUTES_A_HOUR)
#define MINUTES_A_WEEK (DAYS_A_WEEK * MINUTES_A_DAY)

#define OFF HIGH
#define ON  LOW

#define EEPROM_PROG_START   128
#define EEPROM_SIZE        4096
#define EEPROM_UNSET       0xFF

#define MAX_NB_ZONES       8
#define MAX_ZONE_STR_LEN  20 // 20 chars in zone name
#define MAX_DURATION     240 // minutes
#define MAX_NB_PROGRAMMS (3 * 7 * MAX_NB_ZONES) // 168: 3 times a day, 7 days, each zone

uint8_t pin[MAX_NB_ZONES] =
{ D1, D2, D3, D4, D5, D6, D7, D8, };

char zones[MAX_NB_ZONES][MAX_ZONE_STR_LEN + 1] =
{ "Zona 1", "Zona 2", "Zona 3", "Zona 4", "Zona 5", "Zona 6", "Zona 7", "Zona 8", };

const char* days[DAYS_A_WEEK] =
{ "Lu", "Ma", "Mi", "Jo", "Vi", "Sa", "Du", };

typedef struct prog_s
{
  uint16_t mow;
  uint8_t zone;
  uint8_t time;
  uint8_t skip;
} __attribute__((packed)) programm;

// "DD hh:mm"
const uint8_t week_minute_str_len = strlen("DD hh:mm");
const char* to_string(uint16_t);
const char* to_string(programm*);

// time helper functions
uint8_t day_of_week();
uint16_t week_minutes_to_time(uint8_t, uint8_t, uint8_t);
uint16_t now_to_week_minutes(uint16_t);
uint16_t now_as_week_minutes();

// EEPROM helper functions
uint16_t eepromWrite(uint16_t, const char*);
uint16_t eepromWrite(uint16_t, uint16_t);
uint16_t eepromRead(uint16_t, char*, uint16_t);
uint16_t eepromRead(uint16_t, uint16_t &);

uint8_t nb_zones = 0;
uint8_t nb_programms = 0;

programm programms[MAX_NB_PROGRAMMS];

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  for (uint8_t i = 0; i < MAX_NB_ZONES; ++i)
  {
    pinMode(pin[i], OUTPUT);
    digitalWrite(pin[i], OFF);
  }

  // read EEPROM
  uint16_t addr = EEPROM_PROG_START;
  nb_zones = EEPROM.read(addr++);
  ++addr; // the next one after nb_zones should be 0 or unset.
  if(nb_zones == EEPROM_UNSET)
    nb_zones = 0;
  for(uint8_t i; i < nb_zones; ++i)
  {
    addr = eepromRead(addr, zones[i], MAX_ZONE_STR_LEN);
  }

  nb_programms = EEPROM.read(addr);
  ++addr; // the next one after nb_programms should be 0 or unset.
  if(nb_programms == EEPROM_UNSET)
    nb_programms = 0;
  for(uint8_t i; i < nb_programms; ++i)
  {
    uint16_t mow;
    uint8_t zone, time, skip;
    addr = eepromRead(addr, mow);
    zone = EEPROM.read(addr++);
    time = EEPROM.read(addr++);
    skip = EEPROM.read(addr++);
    programms[i] = {mow, zone, time, skip};
  }
}

void loop()
{
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
  if (week_minutes < now_week_minutes)
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
 */
uint16_t week_minutes_to_time(uint8_t day, uint8_t hour, uint8_t minute)
{
  // day should be between Monday and Sunday
  if (day >= DAYS_A_WEEK)
    day %= DAYS_A_WEEK;
  // hour should be between 0 and 23
  if (hour >= HOURS_A_DAY)
    hour %= HOURS_A_DAY;
  // minute should be between 0 and 59
  if (minute >= MINUTES_A_HOUR)
    minute %= MINUTES_A_HOUR;
  return day * MINUTES_A_DAY + hour * MINUTES_A_HOUR + minute;
}

/**
 * Returns day of week; Monday is 0, Sunday is 6.
 */
uint8_t day_of_week()
{
  return (weekday() + 5) % 7;
}

/**
 * Returns a string representation of given week minute as "DD hh:mm".
 */
const char* to_string(uint16_t week_minutes)
{
  static char buff[week_minute_str_len + 1];
  sprintf(buff, "%s %d:%02d",
      days[(week_minutes / (MINUTES_A_HOUR * HOURS_A_DAY)) % DAYS_A_WEEK],
      ((week_minutes / MINUTES_A_HOUR) % HOURS_A_DAY),
      (week_minutes % MINUTES_A_HOUR));
  return buff;
}

/**
 * Returns a string representation of given proramm as "Zone: DD hh:mm - min".
 */
const char* to_string(programm* p)
{
  static char buff[40];
  snprintf(buff, 39, "%s: %s - %d", zones[p->zone % nb_zones], to_string(p->mow), p->time);
  return buff;
}

/**
 * Writes the given char* into EEPROM starting at given address up to
 * terminating 0.
 * Return the next address, the one after 0.
 */
uint16_t eepromWrite(uint16_t addr, const char* value)
{
  uint8_t i = 0;
  char c;
  while((c = *(value + (i++))))
    EEPROM.write(addr++, c);
  EEPROM.write(addr++, 0);
  return addr;
}

/**
 * Writes the given two bytes value into EEPROM starting at given address.
 * Return the next (free) address.
 */
uint16_t eepromWrite(uint16_t addr, uint16_t value)
{
  EEPROM.write(addr++, ((value & 0xff00) >> 8));
  EEPROM.write(addr++, (value & 0xff));
  return addr;
}

/**
 * Reads the EEPROM starting at given address into given char* up to and
 * including terminating 0 or until char* reaches given size, whichever come
 * first.
 *
 * Returns the next position; the one after 0, regardless of given size.
 */
uint16_t eepromRead(uint16_t addr, char* value, uint16_t size)
{
  uint16_t addr0 = addr;
  char c;
  while((c = EEPROM.read(addr)))
  {
    if((addr - addr0) < size)
      *(value + (addr - addr0)) = c;
    else
      ++addr0;
    ++addr;
  }
  *(value + (addr - addr0)) = '\0';
  return ++addr;
}

/**
 * Reads two bytes into given value starting at given address.
 * Return the next (free) address.
 */
uint16_t eepromRead(uint16_t addr, uint16_t & value)
{
  value = EEPROM.read(addr++) << 8 + EEPROM.read(addr++);
  return addr;
}

/**
 * Reads two bytes into given value starting at given address.
 * Return the next (free) address.
 */
uint16_t eepromRead(uint16_t addr, uint8_t *value, uint16_t size)
{
  uint16_t addr0 = addr;
  char c;
  while((addr - addr0) < size)
  {
    *(value + (addr - addr0)) = EEPROM.read(addr);
    ++addr;
  }
  return addr;
}

