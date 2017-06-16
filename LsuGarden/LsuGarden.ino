#define DEBUG 1
//#define EEPROM_RESET

#include <Arduino.h>
#include <pins_arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include <LsuWiFiC.h>
#include <LsuNtpTimeC.h>

enum days_enum
  : uint8_t
  {
    MO = 0, TU, WE, TH, FR, SA, SU, DAYS_A_WEEK,
};

#define OFF HIGH
#define ON  LOW

const uint8_t HOURS_A_DAY = 24;
const uint8_t MINUTES_A_HOUR = 60;
const uint16_t MINUTES_A_DAY = (HOURS_A_DAY * MINUTES_A_HOUR);
const uint16_t MINUTES_A_WEEK = (DAYS_A_WEEK * MINUTES_A_DAY);

const uint8_t MAX_NB_ZONES = 8; // 8 realys -> max 8 zones
const uint8_t MAX_PROGRAMMS_PER_DAY = 3; // 3 times a day
const uint8_t MAX_ZONE_STR_LEN = 20; // 20 chars in zone name
const uint8_t MAX_DURATION = 240; // minutes
const uint8_t MAX_NB_PROGRAMMS = /*168*/(MAX_PROGRAMMS_PER_DAY * 7
    * MAX_NB_ZONES); // 3 times a day, 7 days, each zone

const uint8_t EEPROM_UNSET = 255; // 0xFF
const uint8_t EEPROM_START = 128;
const uint8_t EEPROM_ZONE_START = /*128*/EEPROM_START;
const uint16_t EEPROM_PROG_START = /*298*/EEPROM_ZONE_START + 2 + (MAX_NB_ZONES) * (MAX_ZONE_STR_LEN + 1);
const uint16_t EEPROM_SIZE = 0xe80;

//uint8_t pin[MAX_NB_ZONES] =
//{ D1, D2, D3, D4, D5, D6, D7, D8, };

char zones[MAX_NB_ZONES][MAX_ZONE_STR_LEN + 1] =
    {
      "Zona 1",
      "Zona 2",
      "Zona 3",
      "Zona 4",
      "Zona 5",
      "Zona 6",
      "Zona 7",
      "Zona 8", };

const char* days[DAYS_A_WEEK] =
{ "Lu", "Ma", "Mi", "Jo", "Vi", "Sa", "Du", };

const char* days_full[DAYS_A_WEEK] =
{ "Luni", "Mar&#x21B;i", "Miercuri", "Joi", "Vineri", "S&#226;mb&#259;t&#259;", "Duminic&#259;", };

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
uint16_t week_minute(uint8_t, uint8_t, uint8_t);
uint16_t now_to_week_minutes(uint16_t);
uint16_t now_week_minute();

// EEPROM helper functions
uint16_t eepromWrite(uint16_t, const char*);
uint16_t eepromRead(uint16_t, char*);
bool eepromSaveProgramming();
uint16_t eepromLoadProgramming();

// programms helper functions
void sortProgramms();
void loadDefaultProgramming();
void runProgramms();

uint8_t nb_zones = 0;
uint8_t nb_programms = 0;

uint8_t curr_programm = MAX_NB_PROGRAMMS;

programm programms[MAX_NB_PROGRAMMS];

// HTTP
ESP8266WebServer server(80);

void handleIndex();
void handleSkip();
void handleSkipSave();

void eepromPrint(uint16_t start = 0, bool stop_if_unset = false)
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
    EEPROM.write(i, (unsigned int)0xFF);
  }
  return EEPROM.commit();
}


void setup()
{
  Serial.begin(115200);
  Serial.println();
  EEPROM.begin(EEPROM_SIZE);

#ifdef EEPROM_RESET
  eepromReset();
  // print EEPROM data
//  eepromPrint();
#endif


  LsuWiFi::connect(2,10000,true,false);
  LsuNtpTime::begin();

//  for (uint8_t i = 0; i < MAX_NB_ZONES; ++i)
//  {
//    pinMode(pin[i], OUTPUT);
//    digitalWrite(pin[i], OFF);
//  }

  // read EEPROM
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
  if(!nb_zones)
    nb_zones = MAX_NB_ZONES;

  // read programms
  eepromLoadProgramming();
#if DEBUG >= 2
  Serial.println();
  eepromPrint(EEPROM_PROG_START, true);
#endif
  if (!nb_programms)
    loadDefaultProgramming();
  if (!nb_programms)
    curr_programm = 0;
  // get
  uint16_t now_mow = now_week_minute();
#if DEBUG
  Serial.println();
  Serial.print("now_mow: ");Serial.println(to_string(now_mow));
#endif
  // sort programms
  sortProgramms();

#if DEBUG
  Serial.print("nb_programms: ");Serial.println(nb_programms);
#endif
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
#if DEBUG
    Serial.print((unsigned short)i);Serial.print(": ");Serial.println(to_string(programms[i]));
#endif
    if (now_mow < programms[i].mow)
    {
      // OK
      continue;
    }
    curr_programm = i;
  }
#if DEBUG
  Serial.print("curr_programm: ");Serial.println(curr_programm);
  Serial.println();
#endif

  // HTTP
  server.on("/", handleIndex);
  server.on("/skip", handleSkip);
  server.on("/skip_save", handleSkipSave);
  server.begin();
  Serial.println("Web server started");
}





void loop()
{
  if(!(millis() % 1000))
    runProgramms();
  server.handleClient();
}







void runProgramms()
{
  uint16_t now_mow = now_week_minute();
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    if (now_mow < programms[i].mow)
    {
      // OK
      continue;
    }
    curr_programm = i;
  }
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    // start / stop
#if DEBUG
    bool was_running = programms[i].running;
#endif
    programms[i].running = (i == curr_programm && (programms[i].mow + programms[i].time) > now_mow && !programms[i].skip);
//    digitalWrite(pin[programms[i].zone], (programms[i].running ? ON : OFF));
#if DEBUG
    if(was_running != programms[i].running)
    {
      Serial.print(to_string(programms[i]));Serial.print(" - ");Serial.println((programms[i].running ? "start" : "stop"));
    }
#endif
  }
}

void handleIndex()
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
      "<!-- <title>CLLS Iriga&#x21B;ie</title> -->"
      "<style type='text/css'>"
      "table { border-collapse:collapse; border-style:solid; }"
      "th { padding: 15px; border-style:solid; border-width:thin; }"
      "td { padding: 5px; border-style:solid; border-width:thin; }"
      "</style>"
      "</head>"
      "<body>"
      "<!-- <h1>CLLS Iriga&#x21B;ie</h1> -->"
      "<table>"
      "<tr><td colspan='6' align='center'>"
      "<b>";
  html += days_full[day_from_week_minutes(now_mow)];
  html += ", ";
  html += LsuNtpTime::timeString();
  html += "</b>"
      "</td></tr>"
      "<tr><td colspan='6' align='center'></td></tr>"
      "<tr>"
      "<th>Ziua</th>"
      "<th>Ora</th>"
      "<th>Durata</th>"
      "<th>Zona</th>"
      "<th>Merge</th>"
      "<th>Omis</th>"
      "</tr>";

  bool mo_ft = true, tu_ft = true, we_ft = true, th_ft = true, fr_ft = true, sa_ft = true, su_ft = true;
  uint8_t day_idx = DAYS_A_WEEK;
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
        "<td align='center'>";
    html += (programms[i].skip ? "da" : "nu");
    html += "</td>"
        "</tr>";
  }
  html += "<tr><td colspan='6' align='center'></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='skip'><input type='submit' value='Omitere'></form></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='onetime'><input type='submit' value='Pornire rapid&#259;'></form></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='programming_1'><input type='submit' value='Programare'></form></td></tr>"
      "</table>"
      "</body>"
      "</html>";
  server.sendHeader("Refresh", "10");
  server.send(200, "text/html", html);
}

void handleIndex_1()
{
  uint16_t now_mow = now_week_minute();
  String html =
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "<meta charset='UTF-8'>"
      "<!-- <title>CLLS Iriga&#x21B;ie</title> -->"
      "<style type='text/css'>"
      "table { border-collapse:collapse; border-style:solid; }"
      "th { padding: 15px; border-style:solid; border-width:thin; }"
      "td { padding: 5px; border-style:solid; border-width:thin; }"
      "</style>"
      "</head>"
      "<body>"
      "<!-- <h1>CLLS Iriga&#x21B;ie</h1> -->"
      "<table>"
      "<tr>"
      "<th>Curent</th>"
      "<th>Data</th>"
      "<th>Zona</th>"
      "<th>Durata</th>"
      "<th>Merge</th>"
      "<th>Omis</th>"
      "</tr>";
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    html += "<tr>";
    // current is right after now -> before current
    if((i == 0 && now_mow < programms[i].mow)
        || (now_mow >= programms[i - 1].mow + programms[i - 1].time && now_mow < programms[i].mow)) // in between
    {
      html += "<td align='center'>";
      html += to_string(now_mow);
      html += "</td>"
          "<td colspan='5'></td>"
          "</tr>"
          "<tr>"
          "<td align='center'>";
      html += "</td>";
    }
    // current is now -> inline w/ current
    else if (now_mow >= programms[i].mow && now_mow < programms[i].mow + programms[i].time)
    {
      html += "<td align='center'>";
      html += to_string(now_mow);
      html += "</td>";
    }
    // current is sometine else -> none
    else
    {
      html += "<td align='center'>";
      html += "</td>";

    }
    html += "<td align='center'>";
    html += to_string(programms[i].mow);
    html += "</td>"
        "<td align='center'>";
    html += zones[programms[i].zone];
    html += "</td>"
        "<td align='center'>";
    html += (int)programms[i].time;
    html += "</td>"
        "<td align='center'>";
    html += (programms[i].running ? "da" : "nu");
    html += "</td>"
        "<td align='center'>";
    html += (programms[i].skip ? "da" : "nu");
    html += "</td>"
        "</tr>";
  }
  html += "<tr><td colspan='6' align='center'></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='skip'><input type='submit' value='Omitere'></form></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='onetime'><input type='submit' value='Pornire rapid&#259;'></form></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='programming_1'><input type='submit' value='Programare'></form></td></tr>"
      "</table>"
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
      "<!-- <title>CLLS Iriga&#x21B;ie - Omitere</title> -->"
      "<style type='text/css'>"
      "table { border-collapse:collapse; border-style:solid; }"
      "th { padding: 15px; border-style:solid; border-width:thin; }"
      "td { padding: 5px; border-style:solid; border-width:thin; }"
      "</style>"
      "</head>"
      "<body>"
      "<!-- <h1>CLLS Iriga&#x21B;ie - Omitere</h1> -->"
      "<table>"
      "<tr><td colspan='6' align='center'>"
      "<b>";
  html += days_full[day_from_week_minutes(now_mow)];
  html += ", ";
  html += LsuNtpTime::timeString();
  html += "</b>"
      "</td></tr>"
      "<tr><td colspan='6' align='center'></td></tr>"
      "<tr>"
      "<th>Ziua</th>"
      "<th>Ora</th>"
      "<th>Durata</th>"
      "<th>Zona</th>"
      "<th>Merge</th>"
      "<th>Omis</th>"
      "</tr>";

  bool mo_ft = true, tu_ft = true, we_ft = true, th_ft = true, fr_ft = true, sa_ft = true, su_ft = true;
  uint8_t day_idx = DAYS_A_WEEK;
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
  html += "<tr><td colspan='6' align='center'></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='skip_save' id='skip_save'><input type='submit' value='Salvare'></form></td></tr>"
      "<tr><td colspan='6' align='center'></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='.' name='back'><input type='submit' value='&#206;napoi'></form></td></tr>"
      "</table>"
      "</body>"
      "</html>";
  server.send(200, "text/html", html);
}

void handleSkip1()
{
  uint16_t now_mow = now_week_minute();
  String html =
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "<meta charset='UTF-8'>"
      "<!-- <title>CLLS Iriga&#x21B;ie - Omitere</title> -->"
      "<style type='text/css'>"
      "table { border-collapse:collapse; border-style:solid; }"
      "th { padding: 15px; border-style:solid; border-width:thin; }"
      "td { padding: 5px; border-style:solid; border-width:thin; }"
      "input { text-align: right; }"
      "</style>"
      "</head>"
      "<body>"
      "<!-- <h1>CLLS Iriga&#x21B;ie - Omitere</h1> -->"
      "<table>"
      "<tr><td colspan='6' align='center'><form method='post' action='.' name='back'><input type='submit' value='&#206;napoi'></form></td></tr>"
      "<tr><td colspan='6' align='center'></td></tr>"
      "<tr>"
      "<th>Curent</th>"
      "<th>Data</th>"
      "<th>Zona</th>"
      "<th>Durata</th>"
      "<th>Merge</th>"
      "<th>Omis</th>"
      "</tr>";
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    html += "<tr>";
    // current is right after now -> before current
    if((i == 0 && now_mow < programms[i].mow)
        || (now_mow >= programms[i - 1].mow + programms[i - 1].time && now_mow < programms[i].mow)) // in between
    {
      html += "<td align='center'>";
      html += to_string(now_mow);
      html += "</td>"
          "<td colspan='5'></td>"
          "</tr>"
          "<tr>"
          "<td align='center'>";
      html += "</td>";
    }
    // current is now -> inline w/ current
    else if (now_mow >= programms[i].mow && now_mow < programms[i].mow + programms[i].time)
    {
      html += "<td align='center'>";
      html += to_string(now_mow);
      html += "</td>";
    }
    // current is sometine else -> none
    else
    {
      html += "<td align='center'>";
      html += "</td>";

    }
    html += "<td align='center'>";
    html += to_string(programms[i].mow);
    html += "</td>"
        "<td align='center'>";
    html += zones[programms[i].zone];
    html += "</td>"
        "<td align='center'>";
    html += (programms[i].running ? "da" : "nu");
    html += "</td>"
        "<td align='center'>";
    html += (int)programms[i].time;
    html += "</td>"
        "<td align='center'><input type='checkbox' name='skip_cb_";
    html += i;
    html += "' form='skip_save' value='1'";
    html += (programms[i].skip ? " checked>" : ">");
    html += "</td>"
        "</tr>";
  }
  html += "<tr><td colspan='6' align='center'></td></tr>"
      "<tr><td colspan='6' align='center'><form method='post' action='skip_save' id='skip_save'><input type='submit' value='Salvare'></form></td></tr>"
      "</table>"
      "</body>"
      "</html>";
  server.send(200, "text/html", html);
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
    if(eepromSaveProgramming())
    {
      // reload
      eepromLoadProgramming();
      for (uint8_t i = 0; i < nb_programms; ++i)
      {
        Serial.println(to_string(programms[i]));
      }

      server.sendHeader("refresh", "3;url=/");
      server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><!-- <title>CLLS Iriga&#x21B;ie - Omitere</title> --></head><body><!-- <h1>CLLS Iriga&#x21B;ie - Omitere</h1> --><h2>Salvare reusita</h2></body></html>");
    }
    else
    {
      server.sendHeader("refresh", "3;url=/");
      server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><!-- <title>CLLS Iriga&#x21B;ie - Omitere</title> --></head><body><!-- <h1>CLLS Iriga&#x21B;ie - Omitere</h1> --><h2 style='color:red'>Eroare salvare</h2></body></html>");
    }
  }
  else
  {
    handleIndex();
  }
}

bool eepromSaveProgramming()
{
  uint16_t addr = EEPROM_PROG_START;
  EEPROM.write(addr++, nb_programms);
  EEPROM.write(addr++, 0);
  for (uint8_t i = 0; i < nb_programms; ++i)
  {
    EEPROM.write(addr++, ((programms[i].mow >> 8) & 0xFF));
    EEPROM.write(addr++, (programms[i].mow & 0xFF));
    EEPROM.write(addr++, programms[i].zone);
    EEPROM.write(addr++, programms[i].time);
    EEPROM.write(addr++, ((programms[i].skip) ? 1 : 0));
  }
  return EEPROM.commit();
}

uint16_t eepromLoadProgramming()
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
uint8_t day_from_week_minutes(uint16_t week_minutes)
{
  return (week_minutes / (MINUTES_A_HOUR * HOURS_A_DAY)) % DAYS_A_WEEK;
}

/**
 * Returns a string representation of given week minute as "DD hh:mm".
 */
const char* to_string(uint16_t week_minutes)
{
  static char wm_buff[week_minute_str_len + 1];
  sprintf(wm_buff, "%s %d:%02d", days[day_from_week_minutes(week_minutes)],
      ((week_minutes / MINUTES_A_HOUR) % HOURS_A_DAY),
      (week_minutes % MINUTES_A_HOUR));
  return wm_buff;
}

/**
 * Returns a string representation of given week minute as "DD hh:mm".
 */
const char* to_string_time(uint16_t week_minutes)
{
  static char wm_buff[week_minute_str_len - 2];
  sprintf(wm_buff, "%d:%02d", ((week_minutes / MINUTES_A_HOUR) % HOURS_A_DAY),
      (week_minutes % MINUTES_A_HOUR));
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

/**
 * Writes the given char* into EEPROM starting at given address up to
 * terminating 0.
 * Return the next address, the one after 0.
 */
uint16_t eepromWrite(uint16_t addr, const char* value)
{
  uint8_t i = 0;
  char c;
  while ((c = *(value + (i++))))
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

void sortProgramms()
{
  for (uint8_t i = 0; i < nb_programms - 1; ++i)
  {
    for (uint8_t j = i + 1; j < nb_programms; ++j)
    {
      if (programms[i].mow <= programms[j].mow)
        // OK
        continue;
#if DEBUG
      Serial.println(String("swap: ") + programms[i].mow + " <-> " + programms[j].mow);
#endif
      // swap
      uint16_t mow = programms[i].mow;
      uint8_t zone = programms[i].zone;
      uint8_t time = programms[i].time;
      bool skip = programms[i].skip;
      bool running = programms[i].running;
      programms[i].mow = programms[j].mow;
      programms[i].zone = programms[j].zone;
      programms[i].time = programms[j].time;
      programms[i].skip = programms[j].skip;
      programms[i].running = programms[j].running;
      programms[j].mow = mow;
      programms[j].zone = zone;
      programms[j].time = time;
      programms[j].skip = skip;
      programms[j].running = running;
    }
  }
}

void loadDefaultProgramming()
{
  nb_zones = 5;
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
