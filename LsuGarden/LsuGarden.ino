#include <Arduino.h>
#include <pins_arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#include <LsuWiFiC.h>
#include <LsuNtpTimeC.h>

enum days
  : uint8_t
  {
    MO = 0, TU, WE, TH, FR, SA, SU, DAYS_A_WEEK,
};

#define OFF HIGH
#define ON  LOW

const uint8_t HOURS_A_DAY =    24;
const uint8_t MINUTES_A_HOUR = 60;
const uint16_t MINUTES_A_DAY = (HOURS_A_DAY * MINUTES_A_HOUR);
const uint16_t MINUTES_A_WEEK = (DAYS_A_WEEK * MINUTES_A_DAY);

const uint8_t MAX_NB_ZONES =       8;
const uint8_t MAX_ZONE_STR_LEN =  20; // 20 chars in zone name
const uint8_t MAX_DURATION =     240; // minutes
const uint8_t MAX_NB_PROGRAMMS = (3 * 7 * MAX_NB_ZONES); // 168: 3 times a day, 7 days, each zone

const uint16_t EEPROM_START =      128;
const uint16_t EEPROM_ZONE_START = EEPROM_START;
const uint16_t EEPROM_PROG_START = EEPROM_ZONE_START + 2 + (MAX_NB_ZONES) * (MAX_ZONE_STR_LEN + 1); // 298
const uint16_t EEPROM_SIZE =       4096;
const uint8_t EEPROM_UNSET =       0xFF;

uint8_t pin[MAX_NB_ZONES] =
{ D1, D2, D3, D4, D5, D6, D7, D8, };

char zones[MAX_NB_ZONES][MAX_ZONE_STR_LEN + 1] =
{ "Zona 1", "Zona 2", "Zona 3", "Zona 4", "Zona 5", "Zona 6", "Zona 7", "Zona 8", };

const char* days[DAYS_A_WEEK] =
{ "Lu", "Ma", "Mi", "Jo", "Vi", "Sa", "Du", };

const char* days_full[DAYS_A_WEEK] =
{ "Luni", "Marti", "Miercuri", "Joi", "Vi", "Sa", "Du", };

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
const char* to_string(programm&);

// time helper functions
uint8_t day_of_week();
uint8_t day_of_week_minutes(uint16_t);
uint16_t week_minute(uint8_t, uint8_t, uint8_t);
uint16_t now_to_week_minutes(uint16_t);
uint16_t now_week_minute();

// EEPROM helper functions
uint16_t eepromWrite(uint16_t, const char*);
uint16_t eepromRead(uint16_t, char*);

// programms helper functions
void sortProgramms();
void loadDefaultProgramming();

uint8_t nb_zones = 0;
uint8_t nb_programms = 0;

uint8_t curr_programm = MAX_NB_PROGRAMMS;

programm programms[MAX_NB_PROGRAMMS];






void setup()
{
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  LsuWiFi::connect();
  LsuNtpTime::begin();

  for (uint8_t i = 0; i < MAX_NB_ZONES; ++i)
  {
    pinMode(pin[i], OUTPUT);
    digitalWrite(pin[i], OFF);
  }

  // read EEPROM
  // read zone's names
  uint16_t addr = EEPROM_ZONE_START;
  nb_zones = EEPROM.read(addr++);
  nb_zones %= EEPROM_UNSET;
  ++addr; // the next one after nb_zones should be 0 or unset.
  for(uint8_t i; i < nb_zones; ++i)
  {
    addr = eepromRead(addr, zones[i]);
  }

  // read programms
  addr = EEPROM_PROG_START;
  nb_programms = EEPROM.read(addr);
  nb_programms %= EEPROM_UNSET;
  ++addr; // the next one after nb_programms should be 0 or unset.
  for(uint8_t i; i < nb_programms; ++i)
  {
    uint16_t mow;
    uint8_t zone, time, skip;
    programms[i].mow = EEPROM.read(addr++) << 8 + EEPROM.read(addr++);
    programms[i].zone = EEPROM.read(addr++);
    programms[i].time = EEPROM.read(addr++);
    programms[i].skip = EEPROM.read(addr++);
  }
  if(!nb_programms)
    loadDefaultProgramming();
  Serial.println(nb_programms);
  if(nb_programms)
    curr_programm = 0;
  // get
  uint16_t now_mow = now_week_minute();
  Serial.println(to_string(now_mow));
  // sort programms
  sortProgramms();
  for(uint8_t i = 0; i < nb_programms; ++i)
  {
    Serial.println(to_string(programms[i]));
    if(now_mow < programms[i].mow)
      // OK
      continue;
    curr_programm = i;
  }
  Serial.println(curr_programm);
}






void loop()
{
  static char datetimebuff[(LsuNtpTime::datetimeStringLength + 1)];
  Serial.print(LsuNtpTime::datetimeString(datetimebuff));
  Serial.print(" - ");
  Serial.println(now_week_minute());
  delay(5000);
}










uint16_t now_week_minute()
{
  return week_minute(day_of_week(), hour(), minute());
}

/**
 * Returns number of minutes between now and given week minutes.
 */
uint16_t now_to_week_minutes(uint16_t week_minutes)
{
  uint16_t now_week_minutes = now_week_minute();
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
uint16_t week_minute(uint8_t day, uint8_t hour, uint8_t minute)
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
 * Returns the day number from the given week minutes, 0 for Monday, 6 for
 * Sunday.
 */
uint8_t day_of_week_minutes(uint16_t week_minutes)
{
  return (week_minutes / (MINUTES_A_HOUR * HOURS_A_DAY)) % DAYS_A_WEEK;
}

/**
 * Returns a string representation of given week minute as "DD hh:mm".
 */
const char* to_string(uint16_t week_minutes)
{
  static char buff[week_minute_str_len + 1];
  sprintf(buff, "%s %d:%02d (%d)",
      days[day_of_week_minutes(week_minutes)],
      ((week_minutes / MINUTES_A_HOUR) % HOURS_A_DAY),
      (week_minutes % MINUTES_A_HOUR), week_minutes);
  return buff;
}

/**
 * Returns a string representation of given proramm as "Zone: DD hh:mm - min".
 */
const char* to_string(programm& p)
{
  static char buff[40];
  snprintf(buff, 39, "%s: %s - %d", zones[p.zone % nb_zones], to_string(p.mow), p.time);
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
 * Reads the EEPROM starting at given address into given char* up to and
 * including terminating 0.
 *
 * Returns the next position; the one after 0, regardless of given size.
 */
uint16_t eepromRead(uint16_t addr, char* value)
{
  uint8_t i = 0;
  char c;
  while((c = EEPROM.read(addr++)))
    *(value + (i++)) = c;
  *(value + (i++)) = c;
  return addr;
}

void sortProgramms()
{
  for(uint8_t i; i < nb_programms; ++i)
  {
    for(uint8_t j = i + 1; j < nb_programms; ++j)
    {
      if(programms[i].mow < programms[j].mow)
        // OK
        continue;
      // swap
      uint16_t mow = programms[i].mow;
      uint8_t zone = programms[i].zone;
      uint8_t time = programms[i].time;
      uint8_t skip = programms[i].skip;
      programms[i].mow = programms[j].mow;
      programms[i].zone = programms[j].zone;
      programms[i].time = programms[j].time;
      programms[i].skip = programms[j].skip;
      programms[j].mow = mow;
      programms[j].zone = zone;
      programms[j].time = time;
      programms[j].skip = skip;
    }
  }
}

void loadDefaultProgramming()
{
  nb_zones = 5;
  uint8_t i = 0;
  // zona 5, MO 21:30, 30
  programms[i++] = {week_minute(MO, 19, 30), 4, 30, 0};
  // zona 1, TU, 20:40, 10
  programms[i++] = {week_minute(MO, 19, 40), 0, 10, 0};
  // zone 2, TU, 20:50, 15
  programms[i++] = {week_minute(MO, 19, 50), 1, 15, 0};
  // zona 3, TU, 21:05, 15
  programms[i++] = {week_minute(TU, 21, 5), 2, 15, 0};
  // zona 4, TU, 21:20, 10
  programms[i++] = {week_minute(TU, 21, 20), 3, 10, 0};
  // zona 5, TU, 21:30, 30
  programms[i++] = {week_minute(TU, 21, 30), 4, 30, 0};
  // zona 5, WE, 21:30, 30
  programms[i++] = {week_minute(WE, 21, 30), 4, 30, 0};
  // zona 1, TH, 20:40, 10
  programms[i++] = {week_minute(TU, 20, 40), 0, 10, 0};
  // zone 2, TH, 20:50, 15
  programms[i++] = {week_minute(TU, 20, 50), 1, 15, 0};
  // zona 3, TH, 21:05, 15
  programms[i++] = {week_minute(TU, 21, 5), 2, 15, 0};
  // zona 4, TH, 21:20, 10
  programms[i++] = {week_minute(TU, 21, 20), 3, 10, 0};
  // zona 5, TH, 21:30, 30
  programms[i++] = {week_minute(TH, 21, 30), 4, 30, 0};
  // zona 5, FR, 21:30, 30
  programms[i++] = {week_minute(FR, 21, 30), 4, 30, 0};
  // zona 5, SA, 21:30, 30
  programms[i++] = {week_minute(SA, 21, 30), 4, 30, 0};
  // zona 1, SU, 20:40, 10
  programms[i++] = {week_minute(SU, 20, 40), 0, 10, 0};
  // zone 2, SU, 20:50, 15
  programms[i++] = {week_minute(SU, 20, 50), 1, 15, 0};
  // zona 3, SU, 21:05, 15
  programms[i++] = {week_minute(SU, 21, 5), 2, 15, 0};
  // zona 4, SU, 21:20, 10
  programms[i++] = {week_minute(SU, 21, 20), 3, 10, 0};
  // zona 5, SU, 21:30, 30
  programms[i++] = {week_minute(SU, 21, 30), 4, 30, 0};
  nb_programms = i;
}

