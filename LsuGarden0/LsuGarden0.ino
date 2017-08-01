#define DEBUG 2
#define IB_IOT 1
// #define EEPROM_RESET

#include <Arduino.h>
#include <pins_arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include <LsuWiFiC.h>
#include <LsuNtpTimeC.h>

/* WiFi AP settings*/
const char *ssid = "";
const char *passwd = "trilulilu"; // use empty string for no password
/**/

enum days_enum
  : uint8_t
  {
    MO = 0, TU, WE, TH, FR, SA, SU,
};

#define BUFF_SIZE 4096

#define OFF HIGH
#define ON  LOW

#define DAYS_PER_WEEK     7
#define HOURS_PER_DAY    24
#define MINUTES_PER_HOUR 60
#define MINUTES_PER_DAY  (HOURS_PER_DAY * MINUTES_PER_HOUR)
#define MINUTES_PER_WEEK (DAYS_PER_WEEK * MINUTES_PER_DAY)

#define MAX_NB_ZONES                       4 // 4 realys -> max 4 zones
#define MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE  3 // 3 times a day
#define MAX_NB_PROGRAMMS_PER_DAY       /* 24 */ (MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE * MAX_NB_ZONES) // 3 times a day, 8 zones
#define MAX_NB_PROGRAMMS_PER_WEEK     /* 168 */ (MAX_NB_PROGRAMMS_PER_DAY * DAYS_PER_WEEK) // all zones, 7 days

#define MAX_ZONE_STR_LEN               20 // 20 chars in zone name
#define MAX_DURATION                  240 // minutes

#define EEPROM_UNSET          255
#define EEPROM_START          256
#define EEPROM_ZONE_START /*  256 */ EEPROM_START
#define EEPROM_PROG_START /*  426 */ (EEPROM_ZONE_START + 2 + MAX_NB_ZONES * (MAX_ZONE_STR_LEN + 1))
#define EEPROM_SIZE          4096

uint8_t pin[MAX_NB_ZONES] =
{ D1, D2, D3, D4, };

const char* days[DAYS_PER_WEEK] =
{ "Lu", "Ma", "Mi", "Jo", "Vi", "Sa", "Du", };

const char* days_full[DAYS_PER_WEEK] =
{ "Luni", "Mar&#539;i", "Miercuri", "Joi", "Vineri", "S&#226;mb&#259;t&#259;", "Duminic&#259;", };

typedef struct programm_struct
{
  uint16_t mow;
  uint8_t zone;
  uint8_t time;
  bool skip;
  bool running;
}__attribute__((packed)) programm;

// "DD hh:mm"
const uint8_t week_minute_str_len = strlen("DD hh:mm");
const char* to_string(uint16_t);
const char* to_string_time(uint16_t);
const char* to_string(programm&);

// time helper functions
uint8_t day_of_week();
uint8_t day_from_week_minutes(uint16_t);
uint8_t hour_from_week_minutes(uint16_t);
uint8_t minute_from_week_minutes(uint16_t);
uint16_t week_minute(uint8_t, uint8_t, uint8_t);
uint16_t now_to_week_minutes(uint16_t);
uint16_t now_week_minute();

// EEPROM helper functions
uint16_t eepromWrite(uint16_t, const char*);
uint16_t eepromRead(uint16_t, char*);

uint16_t eepromLoadProgramms();
bool eepromSaveProgramms(programm*, uint8_t);
uint16_t eepromLoadZones();
bool eepromSaveZones();
void eepromPrint(uint16_t start = 0, bool stop_if_unset = false);
bool eepromReset();

// programms helper functions
void swapProgramms(programm*, programm*);
void sortProgramms(programm*, uint8_t);
void sortProgrammsByZone(programm*, uint8_t);

void loadDefaultProgramms();
void runProgramms();

bool nb_zone_dirty = false;
void loadDefaultZones();

void printHttpReceivedArgs();

uint8_t nb_zones = 0;

char zones[MAX_NB_ZONES][MAX_ZONE_STR_LEN + 1];

uint8_t nb_programms = 0;
uint8_t nb_programms_ps = 0;
uint8_t nb_ot_programms = 0;

programm programms[MAX_NB_PROGRAMMS_PER_WEEK];
programm programms_ps[MAX_NB_PROGRAMMS_PER_WEEK];
programm ot_programms[MAX_NB_ZONES];

// HTTP
ESP8266WebServer server(80);

void handleIndex();
void handleIndex2();
void handleSkip();
void handleSkipSave();
void handleOnetime();
void handleOnetimeSave();
void handleProgramming1();
void handleProgramming1Save();
void handleProgramming2();
void handleProgramming2Save();
void handleProgramming3(uint8_t, bool copy_previous = false);
void handleProgramming3Save(uint8_t);
void handleProgramming31();
void handleProgramming32();
void handleProgramming33();
void handleProgramming34();
void handleProgramming35();
void handleProgramming36();
void handleProgramming37();
void handleProgramming31Save();
void handleProgramming32Save();
void handleProgramming33Save();
void handleProgramming34Save();
void handleProgramming35Save();
void handleProgramming36Save();
void handleProgramming37Save();
void handleProgramming31Load();
void handleProgramming32Load();
void handleProgramming33Load();
void handleProgramming34Load();
void handleProgramming35Load();
void handleProgramming36Load();
void handleProgramming37Load();






void setup()
{
  Serial.begin(115200);
  Serial.println();
  EEPROM.begin(EEPROM_SIZE);

#ifdef EEPROM_RESET
  eepromReset();
#endif

  // add WiFi AP
  if(strlen(ssid))
    LsuWiFi::addAp(ssid, passwd);

  LsuWiFi::connect(2);
  LsuNtpTime::begin();

  for (uint8_t i = 0; i < MAX_NB_ZONES; ++i)
  {
    pinMode(pin[i], OUTPUT);
    digitalWrite(pin[i], OFF);
  }

  // read EEPROM
  // read zone's names
  eepromLoadZones();
#if DEBUG >= 2
  Serial.println("\nEEPROM zones:");
  eepromPrint(EEPROM_ZONE_START, true);
#endif
  if(!nb_zones)
    loadDefaultZones();

  // read programms
  eepromLoadProgramms();
#if DEBUG >= 2
  Serial.println("\nEEPROM programms:");
  eepromPrint(EEPROM_PROG_START, true);
#endif

  if (!nb_programms)
  {
    loadDefaultProgramms();
    // sort them
    sortProgramms(programms, nb_programms);
  }


#if DEBUG
  Serial.println();
  Serial.print("now_mow: ");Serial.println(to_string(now_week_minute()));
  Serial.print("nb_programms: ");Serial.println(nb_programms);
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    Serial.print((unsigned short)i);Serial.print(": ");Serial.println(to_string(programms[i]));
  }
#endif

  // HTTP
  server.on("/", handleIndex);
  server.on("/index2", handleIndex2);
  server.on("/skip", handleSkip);
  server.on("/skip_save", handleSkipSave);
  server.on("/onetime", handleOnetime);
  server.on("/onetime_save", handleOnetimeSave);
  server.on("/programming_1", handleProgramming1);
  server.on("/programming_1_save", handleProgramming1Save);
  server.on("/programming_2", handleProgramming2);
  server.on("/programming_2_save", handleProgramming2Save);
  server.on("/programming_31",           handleProgramming31);
  server.on("/programming_31_load_prev", handleProgramming31Load);
  server.on("/programming_31_save",      handleProgramming31Save);
  server.on("/programming_32",           handleProgramming32);
  server.on("/programming_32_load_prev", handleProgramming32Load);
  server.on("/programming_32_save",      handleProgramming32Save);
  server.on("/programming_33",           handleProgramming33);
  server.on("/programming_33_load_prev", handleProgramming33Load);
  server.on("/programming_33_save",      handleProgramming33Save);
  server.on("/programming_34",           handleProgramming34);
  server.on("/programming_34_load_prev", handleProgramming34Load);
  server.on("/programming_34_save",      handleProgramming34Save);
  server.on("/programming_35",           handleProgramming35);
  server.on("/programming_35_load_prev", handleProgramming35Load);
  server.on("/programming_35_save",      handleProgramming35Save);
  server.on("/programming_36",           handleProgramming36);
  server.on("/programming_36_load_prev", handleProgramming36Load);
  server.on("/programming_36_save",      handleProgramming36Save);
  server.on("/programming_37",           handleProgramming37);
  server.on("/programming_37_load_prev", handleProgramming37Load);
  server.on("/programming_37_save",      handleProgramming37Save);
  server.begin();
  Serial.println("Web server started");
}





void loop()
{
  LsuWiFi::connect(2);
  if((millis() % 5000) == 0) // every 5 seconds
    runProgramms();
  server.handleClient();
}







void runProgramms()
{
  uint16_t now_mow = now_week_minute();
  uint8_t run_prog;
  // one time
  if(nb_ot_programms)
  {
    bool future_run = false;
    run_prog = nb_ot_programms;
    for (uint8_t i = 0; i < nb_ot_programms; ++i)
    {
      if(now_mow >= ot_programms[i].mow && now_mow < (ot_programms[i].mow + ot_programms[i].time))
        run_prog = i;
      future_run = future_run || now_mow < ot_programms[i].mow;
    }
    for (uint8_t i = 0; i < nb_ot_programms; ++i)
    {
      // stop everything except the one should run
      if(i == run_prog)
        continue;
      if(ot_programms[i].running)
      {
        ot_programms[i].running = false;
        digitalWrite(pin[ot_programms[i].zone], OFF);
#if DEBUG
        Serial.print(to_string(ot_programms[i]));Serial.println(" - stop");
#endif
      }
    }
    if(run_prog < nb_ot_programms)
    {
      // start it if not already started
      if(!ot_programms[run_prog].running)
      {
        ot_programms[run_prog].running = true;
        digitalWrite(pin[ot_programms[run_prog].zone], ON);
#if DEBUG
        Serial.print(to_string(ot_programms[run_prog]));Serial.println(" - start");
#endif
      }
    }
    else if(!future_run)
    {
      // no one should run
      nb_ot_programms = 0;
    }
    return;
  }
  // end one time


  // weekly schedule
  run_prog = nb_programms;
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    if(now_mow >= programms[i].mow && now_mow < (programms[i].mow + programms[i].time) && !programms[i].skip)
      run_prog = i;
  }
  // stop everything except the one should run
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    if(i == run_prog)
      continue;
    if(programms[i].running)
    {
      programms[i].running = false;
      digitalWrite(pin[programms[i].zone], OFF);
#if DEBUG
      Serial.print(to_string(programms[i]));Serial.println(" - stop");
#endif
    }
  }
  if(run_prog < nb_programms)
  {
    // start it if not already started
    programms[run_prog].running = true;
    digitalWrite(pin[programms[run_prog].zone], ON);
#if DEBUG
    Serial.print(to_string(programms[run_prog]));Serial.println(" - start");
#endif
  }
}

/**
 * Escapes:
 * %s - days_full[day_from_week_minutes(now_mow)]
 * %s - LsuNtpTime::timeString()
 */
const char content_index2_1_fmt[] PROGMEM =
{"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='UTF-8'>"
"<!-- <title>Iriga&#539;ie</title> -->"
"<style type='text/css'>"
"table { border-collapse:collapse; border-style:solid; }"
"th { padding: 15px; border-style:solid; border-width:thin; }"
"td { padding: 5px; border-style:solid; border-width:thin; }"
"</style>"
"</head>"
"<body>"
"<!-- <h2>Iriga&#539;ie</h2> -->"
"<table>"
"<tr><td colspan='6' align='center'>"
"<b>%s, %s</b>"
"</td></tr>"
"<tr><td colspan='6'></td></tr>"
"<tr><td colspan='6' align='center'><form method='post' action='.' name='back'><input type='submit' value='&#206;napoi'></form></td></tr>"
"<tr><td colspan='6'></td></tr>"};

/**
 * Escapes:
 * none
 */
const char content_index2_2_fmt[] PROGMEM =
{"<tr><th colspan='6' align='center'>Pornire rapid&#259;</th></tr>"
"<tr>"
"<th>Zona</th>"
"<th>Ziua</th>"
"<th>Ora</th>"
"<th>Durata</th>"
"<th>Merge</th>"
"<th>Omis</th>"
"</tr>"};

/**
 * Escapes:
 * %s - zones[ot_programms[i].zone]
 * %s - days_full[day_from_week_minutes(ot_programms[i].mow)]
 * %s - to_string_time(ot_programms[i].mow)
 * %d - (int)ot_programms[i].time
 * %s - (ot_programms[i].running ? "da" : "nu")
 * %s - (ot_programms[i].skip ? "da" : "nu")
 */
const char content_index2_3i_fmt[] PROGMEM =
{"<tr>"
"<td align='center'>%s</td>"
"<td align='left'>%s</td>"
"<td align='center'>%s</td>"
"<td align='center'>%d</td>"
"<td align='center'>%s</td>"
"<td align='center'>%s</td>"
"<tr>"
};

/**
 * Escapes:
 * none
 */
const char content_index2_4_fmt[] PROGMEM =
{"<tr><td colspan='6'></td></tr>"
};

/**
 * Escapes:
 * none
 */
const char content_index2_5_fmt[] PROGMEM =
{"<tr><th colspan='6' align='center'>Program s&#259;pt&#259;m&#226;nal</th></tr>"
  "<tr>"
  "<th>Zona</th>"
  "<th>Ziua</th>"
  "<th>Ora</th>"
  "<th>Durata</th>"
  "<th>Merge</th>"
  "<th>Omis</th>"
  "</tr>"
};

/**
 * Escapes:
 * %d - z_c[j]
 * %s - zones[j]
 */
const char content_index2_6i_1_fmt[] PROGMEM =
{"<td align='center' rowspan='%d'>%s</td>"
};

/**
 * Escapes:
 * %s - days_full[day_from_week_minutes(programms[i].mow)]
 * %s - to_string_time(programms[i].mow)
 * %d - (int)programms[i].time
 * %s - (programms[i].running ? "da" : "nu")
 * %s - (programms[i].skip ? "da" : "nu")
 */
const char content_index2_6i_2_fmt[] PROGMEM =
{"<td align='left'>%s</td>"
  "<td align='center'>%s</td>"
  "<td align='center'>%d</td>"
  "<td align='center'>%s</td>"
  "<td align='center'>%s</td>"
  "</tr>"
};

/**
 * Escapes:
 * none
 */
const char content_index2_7_fmt[] PROGMEM =
{"<tr><td colspan='6'></td></tr>"
  "<tr><td colspan='6' align='center'><form method='post' action='.' name='back'><input type='submit' value='&#206;napoi'></form></td></tr>"
  "</table>"
  "</body>"
  "</html>"
};

void handleIndex2()
{
  uint16_t now_mow = now_week_minute();
  uint8_t z_c[MAX_NB_ZONES];
  bool z_ft[MAX_NB_ZONES];
  for (uint8_t j = 0; j < MAX_NB_ZONES; ++j)
  {
    z_c[j] = 0;
    z_ft[j] = true;
  }
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    z_c[programms[i].zone] += 1;
  }

  char content[BUFF_SIZE] = {'\0'};
  sprintf(content + strlen(content), content_index2_1_fmt, days_full[day_from_week_minutes(now_mow)], LsuNtpTime::timeString());

  String html = "";
//  html += "<!DOCTYPE html>"
//      "<html>"
//      "<head>"
//      "<meta charset='UTF-8'>"
//      "<!-- <title>Iriga&#539;ie</title> -->"
//      "<style type='text/css'>"
//      "table { border-collapse:collapse; border-style:solid; }"
//      "th { padding: 15px; border-style:solid; border-width:thin; }"
//      "td { padding: 5px; border-style:solid; border-width:thin; }"
//      "</style>"
//      "</head>"
//      "<body>"
//      "<!-- <h2>Iriga&#539;ie</h2> -->"
//      "<table>"
//      "<tr><td colspan='6' align='center'>"
//      "<b>";
//  html += days_full[day_from_week_minutes(now_mow)];
//  html += ", ";
//  html += LsuNtpTime::timeString();
//  html += "</b>"
//      "</td></tr>";
//  html += "<tr><td colspan='6'></td></tr>";
//  html += "<tr><td colspan='6' align='center'><form method='post' action='.' name='back'><input type='submit' value='&#206;napoi'></form></td></tr>";
//  html += "<tr><td colspan='6'></td></tr>";
  // one time programms
  if(nb_ot_programms)
  {
    sortProgrammsByZone(ot_programms, nb_ot_programms);
    sprintf(content + strlen(content), content_index2_2_fmt);
//    html += "<tr><th colspan='6' align='center'>Pornire rapid&#259;</th></tr>"
//        "<tr>"
//        "<th>Zona</th>"
//        "<th>Ziua</th>"
//        "<th>Ora</th>"
//        "<th>Durata</th>"
//        "<th>Merge</th>"
//        "<th>Omis</th>"
//        "</tr>";
    for (uint8_t i = 0; i < nb_ot_programms; ++i)
    {
      sprintf(content + strlen(content), content_index2_3i_fmt,
          zones[ot_programms[i].zone],
          days_full[day_from_week_minutes(ot_programms[i].mow)],
          to_string_time(ot_programms[i].mow),
          (int)ot_programms[i].time,
          (ot_programms[i].running ? "da" : "nu"),
          (ot_programms[i].skip ? "da" : "nu"));
//      html += "<tr>";
//      html += "<td align='center'>";
//      html += zones[ot_programms[i].zone];
//      html += "</td>";
//      html += "<td align='left'>";
//      html += days_full[day_from_week_minutes(ot_programms[i].mow)];
//      html += "</td>";
//      html += "<td align='center'>";
//      html += to_string_time(ot_programms[i].mow);
//      html += "</td>";
//      html += "<td align='center'>";
//      html += (int)ot_programms[i].time;
//      html += "</td>";
//      html += "<td align='center'>";
//      html += (ot_programms[i].running ? "da" : "nu");
//      html += "</td>";
//      html += "<td align='center'>";
//      html += (ot_programms[i].skip ? "da" : "nu");
//      html += "</td>";
//      html += "</tr>";
    }
    sprintf(content + strlen(content), content_index2_4_fmt);
//    html += "<tr><td colspan='6'></td></tr>";
  }
  // done - one time programms

  // weekly programming
  sprintf(content + strlen(content), content_index2_5_fmt);
//  html += "<tr><th colspan='6' align='center'>Program s&#259;pt&#259;m&#226;nal</th></tr>"
//      "<tr>"
//      "<th>Zona</th>"
//      "<th>Ziua</th>"
//      "<th>Ora</th>"
//      "<th>Durata</th>"
//      "<th>Merge</th>"
//      "<th>Omis</th>"
//      "</tr>";

  for (uint8_t j = 0; j < MAX_NB_ZONES; ++j)
  {
    if(!z_c[j])
      continue;
    for (uint8_t i = 0; i < nb_programms; ++i)
    {
      if(j != programms[i].zone)
        continue;
      sprintf(content + strlen(content), "<tr>");
//      html += "<tr>";
      if(z_ft[j])
      {
        sprintf(content + strlen(content), content_index2_6i_1_fmt, z_c[j], zones[j]);
//        html += "<td align='center' rowspan='";
//        html += z_c[j];
//        html += "'>";
//        html += zones[j];
//        html += "</td>";
        z_ft[j] = false;
      }
      sprintf(content + strlen(content), content_index2_6i_2_fmt,
          days_full[day_from_week_minutes(programms[i].mow)],
          to_string_time(programms[i].mow),
          (int)programms[i].time,
          (programms[i].running ? "da" : "nu"),
          (programms[i].skip ? "da" : "nu"));
//      html += "<td align='left'>";
//      html += days_full[day_from_week_minutes(programms[i].mow)];
//      html += "</td>";
//      html += "<td align='center'>";
//      html += to_string_time(programms[i].mow);
//      html += "</td>";
//      html += "<td align='center'>";
//      html += (int)programms[i].time;
//      html += "</td>";
//      html += "<td align='center'>";
//      html += (programms[i].running ? "da" : "nu");
//      html += "</td>";
//      html += "<td align='center'>";
//      html += (programms[i].skip ? "da" : "nu");
//      html += "</td>";
//      html += "</tr>";
    }
  }
  // done - weekly programming
  sprintf(content + strlen(content), content_index2_7_fmt);
//  html += "<tr><td colspan='6'></td></tr>";
//  html += "<tr><td colspan='6' align='center'><form method='post' action='.' name='back'><input type='submit' value='&#206;napoi'></form></td></tr>";
//  html += "</table>"
//      "</body>"
//      "</html>";
  server.sendHeader("Refresh", "10");
  server.send(200, "text/html", content);
//  server.send(200, "text/html", html);
}

void handleIndex()
{
  uint16_t now_mow = now_week_minute();
  uint8_t mo_c = 0, tu_c = 0, we_c = 0, th_c = 0, fr_c = 0, sa_c = 0, su_c = 0, rspan = 0;
  bool mo_ft = true, tu_ft = true, we_ft = true, th_ft = true, fr_ft = true, sa_ft = true, su_ft = true;
  uint8_t day_idx = DAYS_PER_WEEK;
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    switch(day_from_week_minutes(programms[i].mow))
    {
      case MO: ++mo_c; break;
      case TU: ++tu_c; break;
      case WE: ++we_c; break;
      case TH: ++th_c; break;
      case FR: ++fr_c; break;
      case SA: ++sa_c; break;
      case SU: ++su_c; break;
    }
  }
  String html =
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "<meta charset='UTF-8'>"
      "<!-- <title>Iriga&#539;ie</title> -->"
      "<style type='text/css'>"
      "table { border-collapse:collapse; border-style:solid; }"
      "th { padding: 15px; border-style:solid; border-width:thin; }"
      "td { padding: 5px; border-style:solid; border-width:thin; }"
      "</style>"
      "</head>"
      "<body>"
      "<!-- <h2>Iriga&#539;ie</h2> -->"
      "<table>"
      "<tr><td colspan='6' align='center'>"
      "<b>";
  html += days_full[day_from_week_minutes(now_mow)];
  html += ", ";
  html += LsuNtpTime::timeString();
  html += "</b>"
      "</td></tr>";
  html += "<tr><td colspan='6'></td></tr>";
  html += "<tr><td colspan='6' align='center'><form method='post' action='index2' name='index2'><input type='submit' value='Vizualizare pe zone'></form></td></tr>";
  html += "<tr><td colspan='6'></td></tr>";
  // one time programms
  if(nb_ot_programms)
  {
    sortProgramms(ot_programms, nb_ot_programms);
    day_idx = day_of_week();
    rspan = nb_ot_programms;
    html += "<tr><th colspan='6' align='center'>Pornire rapid&#259;</th></tr>";
    html += "<tr>"
          "<th>Ziua</th>"
          "<th>Ora</th>"
          "<th>Durata</th>"
          "<th>Zona</th>"
          "<th>Merge</th>"
          "<th>Omis</th>"
          "</tr>";
    html += "<tr>";
    if(rspan)
    {
      html += "<td align='left' rowspan='";
      html += rspan;
      html += "'>";
      html += days_full[day_idx];
      html += "</td>";
    }
    for (uint8_t i = 0; i < nb_ot_programms; ++i)
    {

      html += "<td align='center'>";
      html += to_string_time(ot_programms[i].mow);
      html += "</td>"
          "<td align='center'>";
      html += (int)ot_programms[i].time;
      html += "</td>"
          "<td align='center'>";
      html += zones[ot_programms[i].zone];
      html += "</td>"
          "<td align='center'>";
      html += (ot_programms[i].running ? "da" : "nu");
      html += "</td>"
          "<td align='center'>";
      html += (ot_programms[i].skip ? "da" : "nu");
      html += "</td>"
          "</tr>";
    }
    html += "<tr><td colspan='6'></td></tr>";
  }
  html += "<tr><th colspan='6' align='center'>Program s&#259;pt&#259;m&#226;nal</th></tr>";
  html += "<tr>"
      "<th>Ziua</th>"
      "<th>Ora</th>"
      "<th>Durata</th>"
      "<th>Zona</th>"
      "<th>Merge</th>"
      "<th>Omis</th>"
      "</tr>";

  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    // day
    switch(day_from_week_minutes(programms[i].mow))
    {
      case MO:
        if(mo_ft)
        {
          rspan = mo_c;
          day_idx = MO;
        }
        else
          rspan = 0;
        mo_ft = false;
        break;
      case TU:
        if(tu_ft)
        {
          rspan = tu_c;
          day_idx = TU;
        }
        else
          rspan = 0;
        tu_ft = false;
        break;
      case WE:
        if(we_ft)
        {
          rspan = we_c;
          day_idx = WE;
        }
        else
          rspan = 0;
        we_ft = false;
        break;
      case TH:
        if(th_ft)
        {
          rspan = th_c;
          day_idx = TH;
        }
        else
          rspan = 0;
        th_ft = false;
        break;
      case FR:
        if(fr_ft)
        {
          rspan = fr_c;
          day_idx = FR;
        }
        else
          rspan = 0;
        fr_ft = false;
        break;
      case SA:
        if(sa_ft)
        {
          rspan = sa_c;
          day_idx = SA;
        }
        else
          rspan = 0;
        sa_ft = false;
        break;
      case SU:
        if(su_ft)
        {
          rspan = su_c;
          day_idx = SU;
        }
        else
          rspan = 0;
        su_ft = false;
        break;
    }
    html += "<tr>";
    if(rspan)
    {
      html += "<td align='left' rowspan='";
      html += rspan;
      html += "'>";
      html += days_full[day_idx];
      html += "</td>";
    }

    html += "<td align='center'>";
    html += to_string_time(programms[i].mow);
    html += "</td>"
        "<td align='center'>";
    html += (int)programms[i].time;
    html += "</td>"
        "<td align='center'>";
    html += zones[programms[i].zone];
    html += "</td>"
        "<td align='center'>";
    html += (programms[i].running ? "da" : "nu");
    html += "</td>"
        "<td align='center'>";
    html += (programms[i].skip ? "da" : "nu");
    html += "</td>"
        "</tr>";
  }
  html += "<tr><td colspan='6'></td></tr>";
  html += "<tr><td colspan='6' align='center'><form method='post' action='skip'><input type='submit' value='Omitere'></form></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='onetime'><input type='submit' value='Pornire rapid&#259;'></form></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='programming_1'><input type='submit' value='Programare'></form></td></tr>";
  html += "</table>"
      "</body>"
      "</html>";
  server.sendHeader("Refresh", "10");
  server.send(200, "text/html", html);
}

void handleSkip()
{
  uint16_t now_mow = now_week_minute();
  uint8_t mo_c = 0, tu_c = 0, we_c = 0, th_c = 0, fr_c = 0, sa_c = 0, su_c = 0, rspan = 0;
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    switch(day_from_week_minutes(programms[i].mow))
    {
      case MO: ++mo_c; break;
      case TU: ++tu_c; break;
      case WE: ++we_c; break;
      case TH: ++th_c; break;
      case FR: ++fr_c; break;
      case SA: ++sa_c; break;
      case SU: ++su_c; break;
    }
  }
  String html =
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "<meta charset='UTF-8'>"
      "<!-- <title>Iriga&#539;ie - Omitere</title> -->"
      "<style type='text/css'>"
      "table { border-collapse:collapse; border-style:solid; }"
      "th { padding: 15px; border-style:solid; border-width:thin; }"
      "td { padding: 5px; border-style:solid; border-width:thin; }"
      "</style>"
      "</head>"
      "<body>"
      "<!-- <h2>Iriga&#539;ie - Omitere</h2> -->"
      "<table>"
      "<tr><td colspan='6' align='center'>"
      "<b>";
  html += days_full[day_from_week_minutes(now_mow)];
  html += ", ";
  html += LsuNtpTime::timeString();
  html += "</b>"
      "</td></tr>"
      "<tr><td colspan='6'></td></tr>"
      "<tr>"
      "<th>Ziua</th>"
      "<th>Ora</th>"
      "<th>Durata</th>"
      "<th>Zona</th>"
      "<th>Merge</th>"
      "<th>Omis</th>"
      "</tr>";

  bool mo_ft = true, tu_ft = true, we_ft = true, th_ft = true, fr_ft = true, sa_ft = true, su_ft = true;
  uint8_t day_idx = DAYS_PER_WEEK;
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    html += "<tr>";
    // day
    switch(day_from_week_minutes(programms[i].mow))
    {
      case MO:
        if(mo_ft)
        {
          rspan = mo_c;
          day_idx = MO;
        }
        else
          rspan = 0;
        mo_ft = false;
        break;
      case TU:
        if(tu_ft)
        {
          rspan = tu_c;
          day_idx = TU;
        }
        else
          rspan = 0;
        tu_ft = false;
        break;
      case WE:
        if(we_ft)
        {
          rspan = we_c;
          day_idx = WE;
        }
        else
          rspan = 0;
        we_ft = false;
        break;
      case TH:
        if(th_ft)
        {
          rspan = th_c;
          day_idx = TH;
        }
        else
          rspan = 0;
        th_ft = false;
        break;
      case FR:
        if(fr_ft)
        {
          rspan = fr_c;
          day_idx = FR;
        }
        else
          rspan = 0;
        fr_ft = false;
        break;
      case SA:
        if(sa_ft)
        {
          rspan = sa_c;
          day_idx = SA;
        }
        else
          rspan = 0;
        sa_ft = false;
        break;
      case SU:
        if(su_ft)
        {
          rspan = su_c;
          day_idx = SU;
        }
        else
          rspan = 0;
        su_ft = false;
        break;
    }
    if(rspan)
    {
      html += "<td align='left' rowspan='";
      html += rspan;
      html += "'>";
      html += days_full[day_idx];
      html += "</td>";
    }

    html += "<td align='center'>";
    html += to_string_time(programms[i].mow);
    html += "</td>"
        "<td align='center'>";
    html += (int)programms[i].time;
    html += "</td>"
        "<td align='center'>";
    html += zones[programms[i].zone];
    html += "</td>"
        "<td align='center'>";
    html += (programms[i].running ? "da" : "nu");

    html += "</td>"
        "<td align='center'><input type='checkbox' name='skip_cb_";
    html += i;
    html += "' form='skip_save' value='1'";
    html += (programms[i].skip ? " checked>" : ">");

    html += "</td>"
        "</tr>";
  }
  html += "<tr><td colspan='6'></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='skip_save' id='skip_save'><input type='submit' value='Salvare'></form></td></tr>"
      "<tr><td colspan='6'></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='.' name='back'><input type='submit' value='&#206;napoi'></form></td></tr>"
      "</table>"
      "</body>"
      "</html>";
  server.send(200, "text/html", html);
}

void handleSkipSave()
{
#if DEBUG
  printHttpReceivedArgs();
#endif
  bool save = false, skip;
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    if((skip = server.arg(String("skip_cb_") + i).length() != 0) != programms[i].skip)
    {
      programms[i].skip = skip;
      save = true;
    }
  }
  if(save)
  {
    if(eepromSaveProgramms(programms, nb_programms))
    {
      // reload
      eepromLoadProgramms();
#if DEBUG
      for (uint8_t i = 0; i < nb_programms; ++i)
      {
        Serial.println(to_string(programms[i]));
      }
#endif
      server.sendHeader("refresh", "1;url=/");
      server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><!-- <title>Iriga&#539;ie - Omitere</title> --></head><body><!-- <h2>Iriga&#539;ie - Omitere</h2> --><h3>Salvare reusita</h3></body></html>");
    }
    else
    {
      server.sendHeader("refresh", "3;url=/");
      server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><!-- <title>Iriga&#539;ie - Omitere</title> --></head><body><!-- <h2>Iriga&#539;ie - Omitere</h2> --><h3 style='color:red'>Eroare salvare</h3></body></html>");
    }
  }
  else
  {
    handleIndex();
  }
}

void handleOnetime()
{
  uint16_t now_mow = now_week_minute();
  String html =
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "<meta charset='UTF-8'>"
      "<!-- <title>Iriga&#539;ie - Pornire rapid&#259;</title> -->"
      "<style type='text/css'>"
      "table { border-collapse:collapse; border-style:solid; }"
      "th { padding: 15px; border-style:solid; border-width:thin; }"
      "td { padding: 5px; border-style:solid; border-width:thin; }"
      "input { text-align: center; }"
      "</style>"
      "</head>"
      "<body>"
      "<!-- <h2>Iriga&#539;ie - Pornire rapid&#259;</h2> -->"
      "<table>"
      "<tr><td colspan='2' align='center'>"
      "<b>";
  html += days_full[day_from_week_minutes(now_mow)];
  html += ", ";
  html += LsuNtpTime::timeString();
  html += "</b>"
      "</td></tr>"
      "<tr><td colspan='2'></td></tr>"
      "<tr>"
      "<td align='center'>Ora pornire</td>"
      "<td align='center'><input type='text' size='3' maxlength='2' name='h' form='onetime_save' value='";
  html += hour();
  html += "'>:<input type='text' size='3' maxlength='2' name='m' form='onetime_save' value='";
  html += minute();
  html += "'></td>"
      "</tr>"
      "<tr><td colspan='2'></td></tr>";
  html += "<tr><td align='center'>Zona</td><td align='center'>Durata</td></tr>";
  for (uint8_t i = 0; i < nb_zones; ++i)
  {
    html += "<tr><td align='center'><select name='z_";
    html += i;
    html += "' form='onetime_save'><option value='";
    html += nb_zones;
    html += "'>-</option>";
    for (uint8_t j = 0; j < nb_zones; ++j)
    {
      html += "<option value='"; html += j; html += "'>"; html += zones[j]; html += "</option>";
    }
    html += "</select></td>"
        "<td align='center'><input type='text' size='4' maxlength='3' name='d_";
    html += i;
    html += "' form='onetime_save' value='15'></td>"
        "</tr>"
        ;
  }
  html += "<tr><td colspan='2'></td></tr>"
      "<tr><td colspan='2' align='center'><form method='post' action='onetime_save' id='onetime_save'><input type='submit' value='Salvare'></form></td></tr>"
      "<tr><td colspan='2'></td></tr>"
      "<tr><td colspan='2' align='center'><form method='post' action='.' name='back'><input type='submit' value='&#206;napoi'></form></td></tr>"
      "</table>"
      "</body>"
      "</html>";
  server.send(200, "text/html", html);
}

void handleOnetimeSave()
{
#if DEBUG
  printHttpReceivedArgs();
#endif
  uint8_t z, d, h, m;
  String arg_str;
  bool hm_ok = (arg_str = server.arg("h")).length() > 0 &&
      (h = arg_str.toInt()) &&
      h < HOURS_PER_DAY &&
      (arg_str = server.arg("m")).length() > 0 &&
      (m = arg_str.toInt()) &&
      m < MINUTES_PER_HOUR;
  uint16_t mow = 0;
  if(hm_ok)
    mow = week_minute(day_of_week(), h, m);
  // stop all that may run
  for (uint8_t i = 0; i < nb_ot_programms; ++i)
  {
    digitalWrite(pin[ot_programms[i].zone], OFF);
  }
  nb_ot_programms = 0;
  uint8_t j = 0;
  for (uint8_t i = 0; i < nb_zones; ++i)
  {
    if((arg_str = server.arg(String("z_") + i)).length() != 0 &&
        (z = arg_str.toInt()) < nb_zones)
    {
      if((arg_str = server.arg(String("d_") + i)).length() == 0 ||
          (d = arg_str.toInt()) > MAX_DURATION)
      {
        server.sendHeader("refresh", "3;url=/");
        server.send(200, "text/html", String("<!DOCTYPE html><html><head><meta charset='UTF-8'><!-- <title>Iriga&#539;ie - Pornire rapid&#259;</title> --></head><body><!-- <h2>Iriga&#539;ie - Pornire rapid&#259;</h2> --><h3 style='color:red'>Eroare, durat&#259; eronat&#259; pentru ") + zones[i] + "</h3></body></html>");
        return;
      }
      // start time not OK
      if(!hm_ok)
      {
        server.sendHeader("refresh", "3;url=/");
        server.send(200, "text/html", String("<!DOCTYPE html><html><head><meta charset='UTF-8'><!-- <title>Iriga&#539;ie - Pornire rapid&#259;</title> --></head><body><!-- <h2>Iriga&#539;ie - Pornire rapid&#259;</h2> --><h3 style='color:red'>Eroare, timpul de pornire eronat: ") + h + ":" + m + " [hh:mm]</h3></body></html>");
        return;
      }
      // save
      ot_programms[j++] = { mow, z, d, false, false};
#if DEBUG
      Serial.print("save ot: ");Serial.println(to_string(ot_programms[j - 1]));
#endif
      mow += d;
    }
  }
#if DEBUG
      Serial.print("nb_ot_programms: ");Serial.println(j);
#endif
  if(nb_ot_programms != j)
  {
    nb_ot_programms = j;
    server.sendHeader("refresh", "3;url=/");
    server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><!-- <title>Iriga&#539;ie - Pornire rapid&#259;</title> --></head><body><!-- <h2>Iriga&#539;ie - Pornire rapid&#259;</h2> --><h3>Salvare reusita</h3></body></html>");
    return;
  }
  server.sendHeader("refresh", "1;url=/");
  server.send(200, "text/html", "<!DOCTYPE html><html></html>");
}

void handleProgramming1()
{
  // initialize the number
  nb_programms_ps = 0;
  nb_zone_dirty = false;
  uint16_t now_mow = now_week_minute();
  String html =
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "<meta charset='UTF-8'>"
      "<!-- <title>Iriga&#539;ie - Programare 1</title> -->"
      "<style type='text/css'>"
      "table { border-collapse:collapse; border-style:solid; }"
      "th { padding: 15px; border-style:solid; border-width:thin; }"
      "td { padding: 5px; border-style:solid; border-width:thin; }"
      "input { text-align: center; }"
      "</style>"
      "</head>"
      "<body>"
      "<!-- <h2>Iriga&#539;ie - Programare</h2> -->"
      "<h3>Pasul 1 - Num&#259;rul de zone (maxim 8)</h3>"
      "<table>";

  html += "<tr><th>Num&#259;rul de zone</th><td><input type='text' size='2' maxlength='1' name='nb_zones' form='programming_1_save'";
  if(nb_zones)
  {
    html += " value='";
    html += nb_zones;
    html += "'";
  }
  html += "></td></tr>"
      "<tr><td colspan='2'></td></tr>"
      "<tr><td colspan='2' align='center'><form method='post' action='programming_1_save' id='programming_1_save'><input type='submit' value='&#206;nainte'></form></td></tr>"
      "<tr><td colspan='2'></td></tr>"
      "<tr><td colspan='2' align='center'><form method='post' action='.'><input type='submit' value='&#206;napoi'></form></td></tr>"
      "</table>"
      "</body>"
      "</html>";
  server.send(200, "text/html", html);
}

void handleProgramming1Save()
{
#if DEBUG
  printHttpReceivedArgs();
#endif
  uint8_t nb_zones_n = 0;
  String arg_str;
  if((arg_str = server.arg("nb_zones")).length() > 0)
    nb_zones_n = arg_str.toInt();
  if(nb_zones_n > 0 && nb_zones_n <= MAX_NB_ZONES)
  {
    if(nb_zones != nb_zones_n)
    {
      uint8_t tmp = nb_zones;
      nb_zones = nb_zones_n;
      if(eepromSaveZones() && nb_zones == eepromLoadZones())
      {
        nb_zone_dirty = false;
        server.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/programming_2' /></head></html>");
        return;
      }
      nb_zones = tmp;
      server.sendHeader("refresh", "3;url=/programming_2");
      server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><!-- <title>Iriga&#539;ie - Programare 2</title> --></head><body><!-- <h2>Iriga&#539;ie - Programare 2</h2> --><h3 style='color:red'>Eroare salvare, se va folosi num&#259;rul de zone ini&#539;</h3></body></html>");
      return;

    }
    server.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/programming_2' /></head></html>");
    return;
  }
  else
  {
    server.sendHeader("refresh", "3;url=/programming_1");
    server.send(200, "text/html", String("<!DOCTYPE html><html><head><meta charset='UTF-8'><!-- <title>Iriga&#539;ie - Programare 1</title> --></head><body><!-- <h2>Iriga&#539;ie - Programare 1</h2> --><h3 style='color:red'>Eroare, num&#259;r de zone eronat: ") + nb_zones_n + "<br>Trebuie s&#259; fie &#238;ntre 1 &#x219;i " + MAX_NB_ZONES + " inclusiv</h3></body></html>");
    return;
  }
}

void handleProgramming2()
{
  uint16_t now_mow = now_week_minute();
  String html =
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "<meta charset='UTF-8'>"
      "<!-- <title>Iriga&#539;ie - Programare 2</title> -->"
      "<style type='text/css'>"
      "table { border-collapse:collapse; border-style:solid; }"
      "th { padding: 15px; border-style:solid; border-width:thin; }"
      "td { padding: 5px; border-style:solid; border-width:thin; }"
      "input { text-align: center; }"
      "</style>"
      "</head>"
      "<body>"
      "<!-- <h2>Iriga&#539;ie - Programare</h2> -->"
      "<h3>Pasul 2 - Numele zonelor (maxim ";
  html += MAX_ZONE_STR_LEN;
  html += " caractere)</h3>"
      "<table>";

  for(uint8_t i = 0; i < nb_zones; ++i)
  {
    html += "<tr><td align='center'><b>Zona ";
    html += (i + 1);
    html += "</b></td><td><input type='text' size='";
  html += MAX_ZONE_STR_LEN;
  html += "' maxlength='";
  html += MAX_ZONE_STR_LEN;
  html += "' form='programming_2_save' name='z_";
    html += i;
    html += "' value='";
    html += zones[i];
    html += "'></td></tr>";
  }

  html += "<tr><td colspan='2'></td></tr>"
      "<tr><td colspan='2' align='center'><form method='post' action='programming_2_save' id='programming_2_save'><input type='submit' value='&#206;nainte'></form></td></tr>"
      "<tr><td colspan='2'></td></tr>"
      "<tr><td colspan='2' align='center'><form method='post' action='programming_1'><input type='submit' value='&#206;napoi'></form></td></tr>"
      "</table>"
      "</body>"
      "</html>";

  server.send(200, "text/html", html);
}

void handleProgramming2Save()
{
#if DEBUG
  printHttpReceivedArgs();
#endif
  String arg_str;
  bool one_saved;
  for (uint8_t i = 0; i < nb_zones; ++i)
  {
    if((arg_str = server.arg(String("z_") + i)).length() != 0 &&
        strcmp(zones[i], arg_str.c_str()))
    {
      one_saved = true;
      strncpy(&zones[i][0], arg_str.c_str(), MAX_ZONE_STR_LEN);
    }
  }
  if(one_saved && !(eepromSaveZones() && nb_zones == eepromLoadZones()))
  {
    server.sendHeader("refresh", "3;url=/programming_31");
    server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><!-- <title>Iriga&#539;ie - Programare 2</title> --></head><body><!-- <h2>Iriga&#539;ie - Programare 2</h2> --><h3 style='color:red'>Eroare salvare, se vor folosi numele implicite</h3></body></html>");
    return;
  }
  server.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/programming_31' /></head></html>");
}

void handleProgramming3(uint8_t day_t, bool copy_previous)
{
  uint16_t now_mow = now_week_minute();
  String html =
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "<meta charset='UTF-8'>"
      "<!-- <title>Iriga&#539;ie - Programare 3</title> -->"
      "<style type='text/css'>"
      "table { border-collapse:collapse; border-style:solid; }"
      "th { padding: 15px; border-style:solid; border-width:thin; }"
      "td { padding: 5px; border-style:solid; border-width:thin; }"
      "input { text-align: center; }"
      "</style>"
      "</head>"
      "<body>"
      "<!-- <h2>Iriga&#539;ie - Programare</h2> -->"
      "<h3>Pasul 3 - ";
  html += days_full[day_t];
  html += "</h3>"
      "<table>"
      "<tr>"
      "<th align='center' rowspan='2'>Zona</th>";
  for(uint8_t j = 0; j < MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE; ++j)
  {
    html += "<th align='center'>Interval ";
    html += (j + 1);
    html += "</th>";
  }

  html += "</tr>"
      "<tr>"
      "<td align='center' colspan='";
  html += MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE;
  html += "'>[hh:mm durata]</td>"
      "</tr>";
  for (uint8_t i = 0; i < nb_zones; ++i)
  {
    html += "<tr><th align='center'>";
    html += zones[i];
    html += "<input type='hidden' name='z_"; html += day_t; html += "_"; html += i; html += "' value='"; html += i; html += "' form='programming_3_save'>";
    html += "</th>";

    // get saved programms values
    uint8_t nb_p = 0;
    programm* progs = 0;
    uint8_t day_cp = day_t;
    if(copy_previous)
      day_cp = (day_t + DAYS_PER_WEEK - 1) % DAYS_PER_WEEK; // previous day
    uint8_t h[MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE], m[MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE], d[MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE];
    bool is_set[MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE];
    bool one_set = false;
    uint8_t idx = 0;
    for(uint8_t k = 0; k < MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE; ++k)
    {
      is_set[k] = false;
    }
    if(nb_programms_ps)
    {
      // try already input
      nb_p = nb_programms_ps;
      progs = &programms_ps[0];
      for (uint8_t k = 0; k < nb_p; ++k)
      {
        if(day_cp == day_from_week_minutes(progs[k].mow) &&
            i == progs[k].zone)
        {
          h[idx] = hour_from_week_minutes(progs[k].mow);
          m[idx] = minute_from_week_minutes(progs[k].mow);
          d[idx] = progs[k].time;
          is_set[idx] = true;
          one_set = true;
          ++idx;
        }
      }
    }
    if(!one_set)
    {
      // try saved (old) ones
      nb_p = nb_programms;
      progs = &programms[0];
      for (uint8_t k = 0; k < nb_p; ++k)
      {
        if(day_cp == day_from_week_minutes(progs[k].mow) &&
            i == progs[k].zone)
        {
          h[idx] = hour_from_week_minutes(progs[k].mow);
          m[idx] = minute_from_week_minutes(progs[k].mow);
          d[idx] = progs[k].time;
          is_set[idx] = true;
          ++idx;
        }
      }
    }
    // done

    for(uint8_t j = 0; j < MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE; ++j)
    {
      html += "<td align='center'>";
      if(is_set[j])
      {
        html += "<input type='text' size='4' maxlength='3' name='h_"; html += day_t; html += "_"; html += i; html += "_"; html += j; html += "' form='programming_3_save' value='";;
        html += (int) h[j];
        html += "'>";
        html += ":";
        html += "<input type='text' size='4' maxlength='3' name='m_"; html += day_t; html += "_"; html += i; html += "_"; html += j; html += "' form='programming_3_save' value='";;
        html += (int) m[j];
        html += "'>";
        html += "<input type='text' size='4' maxlength='3' name='d_"; html += day_t; html += "_"; html += i; html += "_"; html += j; html += "' form='programming_3_save' value='";;
        html += (int) d[j];
        html += "'>";
      }
      else
      {
        html += "<input type='text' size='4' maxlength='3' name='h_"; html += day_t; html += "_"; html += i; html += "_"; html += j; html += "' form='programming_3_save'>";
        html += ":";
        html += "<input type='text' size='4' maxlength='3' name='m_"; html += day_t; html += "_"; html += i; html += "_"; html += j; html += "' form='programming_3_save'>";
        html += "<input type='text' size='4' maxlength='3' name='d_"; html += day_t; html += "_"; html += i; html += "_"; html += j; html += "' form='programming_3_save'>";
      }
      html += "</td>";
    }
    html += "</tr>";
  }
  html += "<tr><td colspan='4'></td></tr>";

  html += "<tr><td colspan='";
  html += (MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE + 1);
  html += "' align='center'><form method='post' action='programming_3"; html += (day_t + 1); html += "_load_prev' id='programming_3_load_prev'><input type='submit' value='Valori ziua precedent&#259;'></form></td></tr>";

  html += "<tr><td colspan='";
  html += (MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE + 1);
  html += "' align='center'><form method='post' action='programming_3"; html += (day_t + 1); html += "_save' id='programming_3_save'><input type='submit' value='&#206;nainte'></form></td></tr>";

  html += "<tr><td colspan='";
  html += (MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE + 1);
  html += "'></td></tr>";

  html += "<tr><td colspan='";
  html += (MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE + 1);
  html += "' align='center'><form method='post' action='programming_";
  if(day_t)
  {
    html += "3";
    html += day_t;
  }
  else
    html += "2";
  html += "'><input type='submit' value='&#206;napoi'></form></td></tr>";

  html += "</table>"
      "</body>"
      "</html>";

  server.send(200, "text/html", html);
}

void handleProgramming3Save(uint8_t day_t)
{
#if DEBUG
  printHttpReceivedArgs();
#endif

  // clear the day
  for (uint8_t k = 0; k < nb_programms_ps; )
  {
    if(day_t == day_from_week_minutes(programms_ps[k].mow))
    {
      // move it to the end (if not already the last one)
      if(k < (nb_programms_ps - 1))
        swapProgramms(&programms_ps[k], &programms_ps[nb_programms_ps - 1]);
      // remove it
      --nb_programms_ps;
    }
    else
      ++k;
  }

  uint8_t z, h, m, d;
  uint8_t npd = 0;
  String arg_str;
  for (uint8_t i = 0; i < nb_zones; ++i)
  {
    if((arg_str = server.arg(String("z_") + day_t + "_" + i)).length() <= 0)
      continue;
    z = arg_str.toInt();
    for(uint8_t j = 0; j < MAX_NB_PROGRAMMS_PER_DAY_AND_ZONE; ++j)
    {
      if((arg_str = server.arg(String("h_") + day_t + "_" + i + "_" + j)).length() <= 0)
        continue;
      h = arg_str.toInt();
      if((arg_str = server.arg(String("m_") + day_t + "_" + i + "_" + j)).length() <= 0)
        continue;
      m = arg_str.toInt();
      if((arg_str = server.arg(String("d_") + day_t + "_" + i + "_" + j)).length() <= 0)
        continue;
      d = arg_str.toInt();

      programms_ps[nb_programms_ps] =
        { week_minute(day_t, h, m), z, d, false, false};
      ++nb_programms_ps;
    }
  }

  if(day_t < SU) // Sunday is the last one ;-)
  {
    server.send(200, "text/html", String("<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/programming_3") + (day_t + 2) + "' /></head></html>");
  }
  else
  {
    if(eepromSaveProgramms(programms_ps, nb_programms_ps))
    {
      server.sendHeader("refresh", "1;url=/");
      server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><!-- <title>Iriga&#539;ie - Programare 2</title> --></head><body><!-- <h2>Iriga&#539;ie - Programare 3</h2> --><h3>Salvare reusita</h3></body></html>");
      eepromLoadProgramms();
    }
    else
    {
      server.sendHeader("refresh", "3;url=/");
      server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><!-- <title>Iriga&#539;ie - Programare 2</title> --></head><body><!-- <h2>Iriga&#539;ie - Programare 3</h2> --><h3 style='color:red'>Eroare salvare</h3></body></html>");

    }
  }
}

void handleProgramming31()
{
  return handleProgramming3(MO);
}

void handleProgramming32()
{
  return handleProgramming3(TU);
}

void handleProgramming33()
{
  return handleProgramming3(WE);
}

void handleProgramming34()
{
  return handleProgramming3(TH);
}

void handleProgramming35()
{
  return handleProgramming3(FR);
}

void handleProgramming36()
{
  return handleProgramming3(SA);
}

void handleProgramming37()
{
  return handleProgramming3(SU);
}


void handleProgramming31Save()
{
  return handleProgramming3Save(MO);
}

void handleProgramming32Save()
{
  return handleProgramming3Save(TU);
}

void handleProgramming33Save()
{
  return handleProgramming3Save(WE);
}

void handleProgramming34Save()
{
  return handleProgramming3Save(TH);
}

void handleProgramming35Save()
{
  return handleProgramming3Save(FR);
}

void handleProgramming36Save()
{
  return handleProgramming3Save(SA);
}

void handleProgramming37Save()
{
  return handleProgramming3Save(SU);
}


void handleProgramming31Load()
{
  return handleProgramming3(MO, true);
}

void handleProgramming32Load()
{
  return handleProgramming3(TU, true);
}

void handleProgramming33Load()
{
  return handleProgramming3(WE, true);
}

void handleProgramming34Load()
{
  return handleProgramming3(TH, true);
}

void handleProgramming35Load()
{
  return handleProgramming3(FR, true);
}

void handleProgramming36Load()
{
  return handleProgramming3(SA, true);
}

void handleProgramming37Load()
{
  return handleProgramming3(SU, true);
}



void printHttpReceivedArgs()
{
  uint8_t argCnt = server.args();
  Serial.print("argCnt = ");
  Serial.println(argCnt);
  for(int8_t i = 0; i < argCnt; ++i)
  {
    Serial.print(server.argName(i));
    Serial.print(" = ");
    Serial.println(server.arg(i));
  }
  Serial.println();
}

bool eepromSaveProgramms(programm* programms_t, uint8_t nb_programms_t)
{
  // sort before save
  sortProgramms(programms_t, nb_programms_t);
  uint16_t addr = EEPROM_PROG_START;
  EEPROM.write(addr++, nb_programms_t);
  EEPROM.write(addr++, 0);
  for (uint8_t i = 0; i < nb_programms_t; ++i)
  {
    EEPROM.write(addr++, ((programms_t[i].mow >> 8) & 0xFF));
    EEPROM.write(addr++, (programms_t[i].mow & 0xFF));
    EEPROM.write(addr++, programms_t[i].zone);
    EEPROM.write(addr++, programms_t[i].time);
    EEPROM.write(addr++, ((programms_t[i].skip) ? 1 : 0));
  }
  if(EEPROM.commit())
  {
    nb_programms = nb_programms_t;
    return true;
  }
  return false;
}

uint16_t eepromLoadProgramms()
{
  // read programms
  uint16_t addr = EEPROM_PROG_START;
  nb_programms = EEPROM.read(addr++);

  if(nb_programms == EEPROM_UNSET)
    nb_programms = 0;
  addr++; // the next one after nb_programms should be 0 or unset.
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    programms[i].mow = (EEPROM.read(addr++) << 8) | EEPROM.read(addr++);
    programms[i].zone = EEPROM.read(addr++);
    programms[i].time = EEPROM.read(addr++);
    programms[i].skip = EEPROM.read(addr++) == 0 ? false : true;
    programms[i].running = false;
  }
  return nb_programms;
}

uint16_t eepromLoadZones()
{
  // read zone's names
  uint16_t addr = EEPROM_ZONE_START;
  nb_zones = EEPROM.read(addr++);
  if(nb_zones == EEPROM_UNSET)
    nb_zones = 0;
  addr++; // the next one after nb_zones should be 0 or unset.
  for (uint8_t i = 0; i < nb_zones; ++i)
  {
    addr = eepromRead(addr, zones[i]);
  }
  return nb_zones;
}

bool eepromSaveZones()
{
  uint16_t addr = EEPROM_ZONE_START;
  EEPROM.write(addr++, nb_zones);
  EEPROM.write(addr++, 0);
  for (uint8_t i = 0; i < nb_zones; ++i)
  {
    addr = eepromWrite(addr, zones[i]);
  }
  return EEPROM.commit();
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
    week_minutes += MINUTES_PER_WEEK;
  return (week_minutes - now_week_minutes) % MINUTES_PER_WEEK;
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
  if (day >= DAYS_PER_WEEK)
    day %= DAYS_PER_WEEK;
  // hour should be between 0 and 23
  if (hour >= HOURS_PER_DAY)
    hour %= HOURS_PER_DAY;
  // minute should be between 0 and 59
  if (minute >= MINUTES_PER_HOUR)
    minute %= MINUTES_PER_HOUR;
  return day * MINUTES_PER_DAY + hour * MINUTES_PER_HOUR + minute;
}

/**
 * Returns today's day of week; Monday is 0, Sunday is 6.
 */
uint8_t day_of_week()
{
  return (weekday() + DAYS_PER_WEEK - 2) % DAYS_PER_WEEK;
}

/**
 * Returns the day number from the given week minutes, 0 for Monday, 6 for
 * Sunday.
 */
uint8_t day_from_week_minutes(uint16_t week_minutes)
{
  return (week_minutes / (MINUTES_PER_HOUR * HOURS_PER_DAY)) % DAYS_PER_WEEK;
}

/**
 * Returns the hour from the given week minutes, 0 to 23.
 */
uint8_t hour_from_week_minutes(uint16_t week_minutes)
{
  return (week_minutes / MINUTES_PER_HOUR) % HOURS_PER_DAY;
}

/**
 * Returns the minutes from the given week minutes, 0 to 59.
 */
uint8_t minute_from_week_minutes(uint16_t week_minutes)
{
  return week_minutes % MINUTES_PER_HOUR;
}

/**
 * Returns a string representation of given week minute as "DD hh:mm".
 */
const char* to_string(uint16_t week_minutes)
{
  static char wm_buff[week_minute_str_len + 1];
  sprintf(wm_buff, "%s %d:%02d", days[day_from_week_minutes(week_minutes)],
      ((week_minutes / MINUTES_PER_HOUR) % HOURS_PER_DAY),
      (week_minutes % MINUTES_PER_HOUR));
  return wm_buff;
}

/**
 * Returns a string representation of given week minute as "DD hh:mm".
 */
const char* to_string_time(uint16_t week_minutes)
{
  static char wm_buff[week_minute_str_len - 2];
  sprintf(wm_buff, "%d:%02d", ((week_minutes / MINUTES_PER_HOUR) % HOURS_PER_DAY),
      (week_minutes % MINUTES_PER_HOUR));
  return wm_buff;
}

/**
 * Returns a string representation of given proramm as "Zone: DD hh:mm - min".
 */
const char* to_string(programm& p)
{
  static char pg_buff[MAX_ZONE_STR_LEN + week_minute_str_len + 9];
  sprintf(pg_buff, "%s: %s - %d", zones[p.zone % nb_zones], to_string(p.mow),
      p.time);
  return pg_buff;
}

void eepromPrint(uint16_t start, bool stop_if_unset)
{
  uint8_t rd;
  for (uint16_t i = start; i < EEPROM_SIZE; ++i)
  {
    rd = EEPROM.read(i);
    if(stop_if_unset && rd == EEPROM_UNSET)
      break;
    Serial.print(i, HEX);
    Serial.print(" - ");
    Serial.println((unsigned int)rd, HEX);
  }
}

bool eepromReset()
{
  for (uint16_t i = EEPROM_START; i < EEPROM_SIZE; ++i)
  {
    EEPROM.write(i, EEPROM_UNSET);
  }
  return EEPROM.commit();
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
  while ((c = EEPROM.read(addr++)))
    *(value + (i++)) = c;
  *(value + (i++)) = c;
  return addr;
}

void swapProgramms(programm* p1, programm* p2)
{
  if(!p1 || !p2)
    return;
  uint16_t mow = p1->mow;
  uint8_t zone = p1->zone;
  uint8_t time = p1->time;
  bool skip = p1->skip;
  bool running = p1->running;
  p1->mow = p2->mow;
  p1->zone = p2->zone;
  p1->time = p2->time;
  p1->skip = p2->skip;
  p1->running = p2->running;
  p2->mow = mow;
  p2->zone = zone;
  p2->time = time;
  p2->skip = skip;
  p2->running = running;

}

void sortProgramms(programm* programms_t, uint8_t nb_programms_t)
{
  for (uint8_t i = 0; i < nb_programms_t - 1; ++i)
  {
    for (uint8_t j = i + 1; j < nb_programms_t; ++j)
    {
      if (programms_t[i].mow <= programms_t[j].mow)
        // OK
        continue;
      // swap
      swapProgramms(&programms_t[i], &programms_t[j]);
    }
  }
}

void sortProgrammsByZone(programm* programms_t, uint8_t nb_programms_t)
{
  for (uint8_t i = 0; i < nb_programms_t - 1; ++i)
  {
    for (uint8_t j = i + 1; j < nb_programms_t; ++j)
    {
      if (programms_t[i].zone <= programms_t[j].zone)
        // OK
        continue;
      // swap
      swapProgramms(&programms_t[i], &programms_t[j]);
    }
  }
}

void loadDefaultProgramms()
{
//  nb_zones = 5;
  uint8_t i = 0;
  // zona 5, MO 21:30, 30
  programms[i++] =
  { week_minute(MO, 21, 30), 4, 30, false, false};
  // zona 1, TU, 20:40, 10
  programms[i++] =
  { week_minute(TU, 20, 40), 0, 10, false, false};
  // zone 2, TU, 20:50, 15
  programms[i++] =
  { week_minute(TU, 20, 50), 1, 15, false, false};
  // zona 3, TU, 21:05, 15
  programms[i++] =
  { week_minute(TU, 21, 5), 2, 15, false, false};
  // zona 4, TU, 21:20, 10
  programms[i++] =
  { week_minute(TU, 21, 20), 3, 10, false, false};
  // zona 5, TU, 21:30, 30
  programms[i++] =
  { week_minute(TU, 21, 30), 4, 30, false, false};
  // zona 5, WE 21:30, 30
  programms[i++] =
  { week_minute(WE, 21, 30), 4, 30, false, false};
  // zona 1, TH, 20:40, 10
  programms[i++] =
  { week_minute(TH, 20, 30), 0, 10, false, false};
  // zone 2, TH, 20:50, 15
  programms[i++] =
  { week_minute(TH, 20, 50), 1, 15, false, false};
  // zona 3, TH, 21:05, 15
  programms[i++] =
  { week_minute(TH, 21, 5), 2, 15, false, false};
  // zona 4, TH, 21:20, 10
  programms[i++] =
  { week_minute(TH, 21, 20), 3, 10, false, false};
  // zona 5, TH, 21:30, 30
  programms[i++] =
  { week_minute(TH, 21, 30), 4, 30, false, false};
  // zona 5, FR, 21:30, 30
  programms[i++] =
  { week_minute(FR, 21, 30), 4, 30, false, false};
  // zona 5, SA, 21:30, 30
  programms[i++] =
  { week_minute(SA, 21, 30), 4, 30, false, false};
  // zona 1, SU, 20:40, 10
  programms[i++] =
  { week_minute(SU, 20, 40), 0, 10, false, false};
  // zone 2, SU, 20:50, 15
  programms[i++] =
  { week_minute(SU, 20, 50), 1, 15, false, false};
  // zona 3, SU, 21:05, 15
  programms[i++] =
  { week_minute(SU, 21, 5), 2, 15, false, false};
  // zona 4, SU, 21:20, 10
  programms[i++] =
  { week_minute(SU, 21, 20), 3, 10, false, false};
  // zona 5, SU, 21:30, 30
  programms[i++] =
  { week_minute(SU, 21, 30), 4, 30, false, false};
  nb_programms = i;
}

void loadDefaultZones()
{
  String zone;
  for (uint8_t i = 0; i < MAX_NB_ZONES; ++i)
  {
    zone = String("Zona ") + (i + 1);
    strncpy(&zones[i][0], zone.c_str(), MAX_ZONE_STR_LEN);
    // for safety - put terminating char
    zones[i][MAX_ZONE_STR_LEN] = '\0';
  }
  nb_zones = MAX_NB_ZONES;
}
