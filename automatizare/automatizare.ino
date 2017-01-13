/* automatizare */
#include <OneWire.h>           // https://github.com/PaulStoffregen/OneWire
#include <DallasTemperature.h> // https://github.com/milesburton/Arduino-Temperature-Control-Library

#include <Time.h>         // https://github.com/PaulStoffregen/Time
#include <Timezone.h>     // https://github.com/JChristensen/Timezone

#include <WiFiServer.h>   // arduino_ide/libraries/WiFi/src
#include <WiFiClient.h>   // arduino_ide/libraries/WiFi/src
#include <WiFiUdp.h>      // arduino_ide/libraries/WiFi/src
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino

#include <LsuScheduler.h> // https://github.com/lsuciu70/arduino/tree/master/libraries/LsuScheduler

// one second, 1000 milliseconds
const int SECOND = 1000;
// Sensor count
const byte SENZOR_COUNT = 4;

// parter
const String T_LOC_PARTER = "parter";
const String MAC_PARTER = "18:FE:34:D4:0D:EC";
// etaj
const String T_LOC_ETAJ = "etaj";
const String MAC_ETAJ = "5C:CF:7F:88:EE:49";

String T_LOC = "UNKNOWN";

// register section
const byte MAX_REGISTER = 5;
byte last_register = 0;
IPAddress requester_ip[MAX_REGISTER];
int requester_port[MAX_REGISTER];
String requester_page[MAX_REGISTER];

const String REQUESTER_IP = "requester_ip";
const String REQUESTER_PORT = "requester_port";
const String REQUESTER_PAGE = "requester_page";

// logging section
// log count
const int MAX_LOGGER = 15;
int log_index = -1;
String* logger[MAX_LOGGER] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
String* deferred_log[MAX_LOGGER] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned long deferred_time[MAX_LOGGER];
int deferred_index = 0;

unsigned long start_second = 0;


// WiFi section
const byte SSID_SIZE = 2;
const char* SSID_t[SSID_SIZE] =
{ "cls-router", "cls-ap" };
const char* PASSWD_t[SSID_SIZE] =
{ "r4cD7TPG", "r4cD7TPG" };

byte ssid_ix = 0;

const int HTTP_PORT = 80;

WiFiServer server(HTTP_PORT);

const String OK_CODE = "200 OK";
const String NOK_CODE = "400 Bad Request";


// NTP section
// Eastern European Time (Timisoara)
const TimeChangeRule EEST =
{ "EEST", Last, Sun, Mar, 3, 180 }; // Eastern European Summer Time
const TimeChangeRule EET =
{ "EET", Last, Sun, Oct, 4, 120 }; // Eastern European Standard Time
Timezone EasternEuropeanTime(EEST, EET);

TimeChangeRule *tcr;

// Constants waiting for NTP server response; check every POLL_INTERVAL (ms) up to POLL_TIMES times
const byte POLL_INTERVAL = 10; // poll every this many ms
const byte POLL_TIMES = 100;  // poll up to this many times
const byte PKT_LEN = 48; // NTP packet length
const byte USELES_BYTES = 40; // Useless bytes to be discarded; set useless to 32 for speed; set to 40 for accuracy.

WiFiUDP udp;

const IPAddress master_server_ip(192, 168, 100, 100);
const int master_server_port = 8081;

// Temperature senzor section
// Feather HUZZAH pin 0
const byte ONE_WIRE_1ST_PIN_0 = 0;
// Feather HUZZAH pin 2
const byte ONE_WIRE_2ND_PIN_2 = 2;

// Temperature senzor resolution: 9, 10, 11, or 12 bits
const byte RESOLUTION = 12;

// Conversion times:
// -  9 bit =  94 ms
// - 10 bit = 188 ms
// - 11 bit = 375 ms
// - 12 bit = 750 ms
// Calculate conversion time and add 10 ms
const int CONVERSION_TIME = 10 + 750 / (1 << (12 - RESOLUTION));

const byte SENZOR_ADDRESS_LENGTH = 8;
// Temperature senzor unique I2C addresses.
const uint8_t SENZOR_ADDRESS[2 * SENZOR_COUNT][SENZOR_ADDRESS_LENGTH] =
{
    { 0x28, 0xFF, 0x9F, 0x1C, 0xA6, 0x15, 0x04, 0xEF }, // s0
    { 0x28, 0xFF, 0x09, 0x4F, 0xA6, 0x15, 0x04, 0x94 }, // s1
    { 0x28, 0xFF, 0x18, 0x1A, 0xA6, 0x15, 0x03, 0xFF }, // s2
    { 0x28, 0xFF, 0xDC, 0x0A, 0xA6, 0x15, 0x03, 0xFA }, // s3
    { 0x28, 0xFF, 0x40, 0x09, 0xA6, 0x15, 0x03, 0xE4 }, // j0
    { 0x28, 0xFF, 0x21, 0x14, 0xA6, 0x15, 0x03, 0x9D }, // j1
    { 0x28, 0xFF, 0xCE, 0x1C, 0xA6, 0x15, 0x04, 0xB7 }, // j2
    { 0x28, 0xFF, 0x37, 0x1A, 0xA6, 0x15, 0x03, 0x68 }, // j3
};

// The interval temperature is read
const byte TEMP_READ_INTERVAL = 10;

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire1st_pin0(ONE_WIRE_1ST_PIN_0);
OneWire oneWire2nd_pin2(ONE_WIRE_2ND_PIN_2);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature dallasTemperature1st_pin0(&oneWire1st_pin0);
DallasTemperature dallasTemperature2nd_pin2(&oneWire2nd_pin2);

int temperature[SENZOR_COUNT];

const String room_name[] =
{ "Camera Luca", "Dormitor matrimonial", "Dormitor oaspeti", "Baie etaj",
    "Bucatarie", "Living", "Birou", "Baie parter", };

// Relay section
// Feather HUZZAH relay's pins
const byte relay[] =
{ 12, 13, 14, 15, };

// programming section
// temperature increase amount for P2, P3, and P4 in centi degrees Celsius
const byte DELTA_TEMP = 30;
const String DELTA_TEMP_STR = String("")
    + ((DELTA_TEMP - (DELTA_TEMP % 100)) / 100) + "." + (DELTA_TEMP % 100);
const byte DELTA_P1_TEMP = 10;
const int MAX_DELTA = 100; // 100 centi grades = 1 degree

const byte MIN_RUNNING = 2;

enum
{
  P0_STOPPED = 0,              // stopped
  P1_RUN_HOURS_MAKE_TEMP,      // run between hour and keep target temperature
  P2_START_HOUR_1_INCREASE_05, // start at given hour and increase temperature with DELTA_TEMP/100 deg.C
  P3_START_HOUR_2_INCREASE_05, // start at given hour and increase temperature with DELTA_TEMP/100 deg.C
  P4_START_NOW_INCREASE_05, // start now and increase temperature with DELTA_TEMP/100 deg.C
  P_AFTER_LAST,
  P_NONE,
};
bool is_running[] =
{
false, false, false, false, };

bool force_running[] =
{
false, false, false, false, };

// POST names
const String T_LOC_NAME = "t_loc";
const String PROGRAMMING = "programming";
// P1
const String START_HOUR_P1 = "start_hour_p1";
const String START_MINUTE_P1 = "start_minute_p1";
const String STOP_HOUR_P1 = "stop_hour_p1";
const String STOP_MINUTE_P1 = "stop_minute_p1";
const String TARGET_TEMPERATURE_P1 = "target_temperature_p1";
// P2
const String START_HOUR_P2 = "start_hour_p2";
const String START_MINUTE_P2 = "start_minute_p2";
const String NEXT_PROGRAMM_P2 = "next_programm_p2";
const String TARGET_TEMPERATURE_P2 = "target_temperature_p2";
// P3
const String START_HOUR_P3 = "start_hour_p3";
const String START_MINUTE_P3 = "start_minute_p3";
const String NEXT_PROGRAMM_P3 = "next_programm_p3";
const String TARGET_TEMPERATURE_P3 = "target_temperature_p3";
// P4
const String NEXT_PROGRAMM_P4 = "next_programm_p4";
const String TARGET_TEMPERATURE_P4 = "target_temperature_p4";

byte programming[] =
{ P1_RUN_HOURS_MAKE_TEMP, P1_RUN_HOURS_MAKE_TEMP, P1_RUN_HOURS_MAKE_TEMP,
    P1_RUN_HOURS_MAKE_TEMP, };
// p1: P1_RUN_HOURS_MAKE_TEMP - run between hour and keep target temperature
// p1: start_hour_p1, start_minute_p1, stop_hour_p1, stop_minute_p1, target_temperature_p1
// starts at 15:00 and ends at 7:00 making 20 deg.C downstairs and 22 deg.C upstairs
byte start_hour_p1[] =
{ 15, 15, 15, 15, };
byte start_minute_p1[] =
{ 0, 0, 0, 0, };
byte stop_hour_p1[] =
{ 7, 7, 7, 7, };
byte stop_minute_p1[] =
{ 0, 0, 0, 0, };
int target_temperature_p1[] =
{ 2400, 2400, 2300, 2100, 2000, 2000, 1900, 2000, };

// p2: P2_START_HOUR_1_INCREASE_05 - start at given hour and increase temperature with DELTA_TEMP/100 deg.C
// p2: start_hour_p2, start_minute_p2
// starts at 4:30 and increases the temperature with DELTA_TEMP/100 deg.C

byte start_hour_p2[] =
{ 4, 4, 4, 4, };
byte start_minute_p2[] =
{ 30, 30, 30, 30, };
int target_temperature_p2[] =
{ 0, 0, 0, 0, };
bool has_p2_run_today[] =
{
false, false, false, false, };
byte next_programm_p2[] =
{ P3_START_HOUR_2_INCREASE_05, P3_START_HOUR_2_INCREASE_05,
    P3_START_HOUR_2_INCREASE_05, P3_START_HOUR_2_INCREASE_05, };

// p3: P3_START_HOUR_1_INCREASE_05 - start at given hour and increase temperature with DELTA_TEMP/100 deg.C
// p3: start_hour_p3, start_minute_p3
// starts at 14:00 and increases the temperature with DELTA_TEMP/100 deg.C
byte start_hour_p3[] =
{ 14, 14, 14, 14, };
byte start_minute_p3[] =
{ 30, 30, 30, 30, };
int target_temperature_p3[] =
{ 0, 0, 0, 0,
//  0, 0, 0, 0,
    };
bool has_p3_run_today[] =
{
false, false, false, false, };
byte next_programm_p3[] =
{ P2_START_HOUR_1_INCREASE_05, P2_START_HOUR_1_INCREASE_05,
    P2_START_HOUR_1_INCREASE_05, P2_START_HOUR_1_INCREASE_05, };

// p4: P4_START_NOW_INCREASE_05 - start now and increase temperature with DELTA_TEMP/100 deg,C
// starts now and increases the temperature with DELTA_TEMP/100 deg.C
// target temperature for p1 and p2, calculated at start time by
// adding DELTA_TEMP/100 to current temperature
int target_temperature_p4[] =
{ 0, 0, 0, 0, };
byte next_programm_p4[] =
{ P0_STOPPED, P0_STOPPED, P0_STOPPED, P0_STOPPED, };

// Task scheduler section
LsuScheduler scheduler;

// funtions prototype section
void checkProgramming();

void connectWifi();

void defere_log(const String &);

int floatToRound05Int(float);

byte* getIpBytes(String&, byte*);

time_t getTime();

bool isNowAfter(byte, byte);

bool isNowBetween(byte, byte, byte, byte);

void listen4HttpClient();

void parseRequest(const String &, const String &, String &);

void parseRequest(const String &, const String &, int &);

String printProgramming(byte, byte programm = P_NONE);

void pritSerial();

void processPostData_Programming(const String &);

bool processPostData_Register(const String &);

bool processPostData_Unregister(const String &);

void saveRegister(const IPAddress &, const int, const String &);

void recal_log();

void requestProgramming();

unsigned long secondsRunning();

void sendCurrentTemperatures();

void sendCurrentTemperatures(const String &, const IPAddress & server =
    master_server_ip, const int port = master_server_port);

int savePostData_Programming(const String &, int);

bool savePostData_Register(const String &);

void sendPostData(const String &, const String &, const IPAddress & server =
    master_server_ip, const int port = master_server_port);

void sendHttpIndex(WiFiClient &);

void sendHttpProgramming(WiFiClient &, byte);

void sendHttpResponse(WiFiClient &, const String &);

void sendIp();

void startConversion_1();

void startConversion_2();

void startConversion_3();

void startConversion_4();

//String timeString();
//
String timeString(int day_t = day(), int month_t = month(), int year_t = year(),
    int hour_t = hour(), int minute_t = minute(), int second_t = second());

void updatePxRunToday();

void updateTemperature();

void writeLogger(String &);

bool no_time = true;

bool got_programming = false;

byte offset = 0;

// the setup routine runs once when starts
void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  String mac = WiFi.macAddress();
  if (mac.equalsIgnoreCase(MAC_ETAJ))
  {
    offset = 0;
    T_LOC = T_LOC_ETAJ;
  }
  else if (mac.equalsIgnoreCase(MAC_PARTER))
  {
    offset = SENZOR_COUNT;
    T_LOC = T_LOC_PARTER;
  }

  // Set the external time provider
  setSyncProvider(getTime);
  // Set synch interval to 2 seconds till sync
  setSyncInterval(200);
  while (timeStatus() != timeSet)
  {
    ;
  }
  // Set synch interval to 6 hours
  setSyncInterval(6 * 3600);

  // Start up the temperature library
  dallasTemperature1st_pin0.begin();
  dallasTemperature1st_pin0.setResolution(RESOLUTION);
  dallasTemperature1st_pin0.setWaitForConversion(false);

  dallasTemperature2nd_pin2.begin();
  dallasTemperature2nd_pin2.setResolution(RESOLUTION);
  dallasTemperature2nd_pin2.setWaitForConversion(false);

  requestProgramming();

  saveRegister(master_server_ip, master_server_port, "/update.php");

  int first_read = millis() + 8 * CONVERSION_TIME;
  scheduler.add(startConversion_1, first_read - 4 * CONVERSION_TIME);
  scheduler.add(startConversion_3, first_read - 3 * CONVERSION_TIME);
  scheduler.add(startConversion_2, first_read - 2 * CONVERSION_TIME);
  scheduler.add(startConversion_4, first_read - 1 * CONVERSION_TIME);
  scheduler.add(updateTemperature, first_read);

  updatePxRunToday();
}

bool isRelayInitialized = false;

void relayInitialize()
{
  if (isRelayInitialized)
    return;
  for (byte i = 0; i < SENZOR_COUNT; ++i)
  {
    temperature[i] = 0;
    pinMode(relay[i], OUTPUT);
    digitalWrite(relay[i], HIGH);
    delay(25);
  }
  isRelayInitialized = true;
}

// the loop routine runs over and over again forever
void loop()
{
  relayInitialize();
  if (!got_programming && (millis() % (5 * SECOND)) == 0)
    requestProgramming();
  listen4HttpClient();
  scheduler.execute(millis());
}

void updatePxRunToday()
{
  for (byte i = 0; i < SENZOR_COUNT; ++i)
  {
    int delta_minute_p2 = hour() * 60 + minute()
        - (start_hour_p2[i] * 60 + start_minute_p2[i]);
    int delta_minute_p3 = hour() * 60 + minute()
        - (start_hour_p3[i] * 60 + start_minute_p3[i]);

    if ((programming[i] == P2_START_HOUR_1_INCREASE_05 && delta_minute_p2 > 0)
        || (delta_minute_p2 > 2 * 60))
    {
      if (!has_p2_run_today[i])
      {
        has_p2_run_today[i] = true;
//        writeLogger(String("[") + T_LOC + "] P2 marcat ca a rulat astazi pentru " + room_name[i + offset] + ": " + printProgramming(i));
      }
    }
    if ((programming[i] == P3_START_HOUR_2_INCREASE_05 && delta_minute_p3 > 0)
        || (delta_minute_p3 > 2 * 60))
    {
      if (!has_p3_run_today[i])
      {
        has_p3_run_today[i] = true;
//        writeLogger(String("[") + T_LOC + "] P3 marcat ca a rulat astazi pentru " + room_name[i + offset] + ": " + printProgramming(i));
      }
    }
  }
}

unsigned long secondsRunning()
{
  return millis() / SECOND;
}

void requestProgramming()
{
  connectWifi();
  String post_data = String("") + "t_ip=" + WiFi.localIP().toString();
  sendPostData(post_data, "/programming_send_back.php");
}

void parseRequest(const String &request, const String &field, String &value)
{
  int start_idx;
  if ((start_idx = request.indexOf(field)) < 0)
    return;
  start_idx += field.length();
  int stop_idx = request.indexOf("\n", start_idx);
  if (stop_idx < start_idx)
    value = request.substring(start_idx);
  else
    value = request.substring(start_idx, stop_idx);
  value.trim();
}

void parseRequest(const String &request, const String &field, int &value)
{
  String val_str;
  parseRequest(request, field, val_str);
  value = val_str.toInt();
}

String printProgramming(byte index, byte programm)
{
  if (programm >= P_AFTER_LAST)
    programm = programming[index];
  String program_str = "";
  switch (programm)
  {
    case P0_STOPPED:
      program_str = "P0";
      program_str += " - oprit";
    break;
    case P1_RUN_HOURS_MAKE_TEMP:
      program_str = "P1";
      program_str += " - merge intre ";
      program_str += start_hour_p1[index];
      program_str += ":";
      if (start_minute_p1[index] < 10)
      {
        program_str += "0";
      }
      program_str += start_minute_p1[index];
      program_str += " si ";
      program_str += stop_hour_p1[index];
      program_str += ":";
      if (stop_minute_p1[index] < 10)
      {
        program_str += "0";
      }
      program_str += stop_minute_p1[index];
      if (start_hour_p1[index] > stop_hour_p1[index]
          || (start_hour_p1[index] == stop_hour_p1[index]
              && start_minute_p1[index] >= stop_minute_p1[index]))
        program_str += " (ziua urmatoare)";
      program_str += " si face ";
      program_str += (1.0 * target_temperature_p1[index + offset] / 100);
      program_str += " &deg;C";
    break;
    case P2_START_HOUR_1_INCREASE_05:
      program_str += "P2";
      program_str += " - porneste ";
      if (has_p2_run_today[index])
      {
        program_str += "maine ";
      }
      program_str += "la ";
      program_str += start_hour_p2[index];
      program_str += ":";
      if (start_minute_p2[index] < 10)
      {
        program_str += "0";
      }
      program_str += start_minute_p2[index];
      if (target_temperature_p2[index])
      {
        program_str += String(" si face ")
            + (1.0 * target_temperature_p2[index] / 100) + " &deg;C (+ "
            + DELTA_TEMP_STR + " &deg;C)";
      }
      else
      {
        program_str += String(" si face temperatura de la pornire + ")
            + DELTA_TEMP_STR + " &deg;C";
      }
    break;
    case P3_START_HOUR_2_INCREASE_05:
      program_str += "P3";
      program_str += " - porneste ";
      if (has_p3_run_today[index])
      {
        program_str += "maine ";
      }
      program_str += "la ";
      program_str += start_hour_p3[index];
      program_str += ":";
      if (start_minute_p3[index] < 10)
      {
        program_str += "0";
      }
      program_str += start_minute_p3[index];
      if (target_temperature_p3[index])
      {
        program_str += String(" si face ")
            + (1.0 * target_temperature_p3[index] / 100) + " &deg;C (+ "
            + DELTA_TEMP_STR + " &deg;C)";
      }
      else
      {
        program_str += String(" si face temperatura de la pornire + ")
            + DELTA_TEMP_STR + " &deg;C";
      }
    break;
    case P4_START_NOW_INCREASE_05:
      program_str += "P4";
      program_str += " - porneste acum si face ";
      if (target_temperature_p4[index])
        program_str += (1.0 * target_temperature_p4[index] / 100);
      else
        program_str += (1.0 * temperature[index] / 100);
      program_str += String(" &deg;C (+ ") + DELTA_TEMP_STR + " &deg;C)";
    break;
    default:
      program_str += "EROARE";
      program_str += ": program necunoscut: ";
      program_str += programm;
    break;
  }
  return program_str;
}

void pritSerial()
{
  Serial.print(timeString());
  Serial.print(String(" - [") + T_LOC + "] ");
  for (byte i = 0; i < SENZOR_COUNT; ++i)
  {
    if (i)
      Serial.print(", ");
    Serial.print((1.0 * temperature[i]) / 100);
  }
  Serial.println(" [grd.C]");
}

void connectWifi()
{
  if (WiFi.status() == WL_CONNECTED)
    return;
  Serial.print("WiFi: connecting to ");
  Serial.println(SSID_t[(ssid_ix % SSID_SIZE)]);
  ssid_ix = ssid_ix % SSID_SIZE;
  WiFi.begin(SSID_t[ssid_ix], PASSWD_t[ssid_ix]);

  unsigned long mllis = millis();
  byte count = 1;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(10);
    if (millis() - mllis >= 10000)
    {
      Serial.println(" - 10 s timed out. Trying next SSID.");
      writeLogger(
          String("[") + T_LOC + "] WiFi: connectare la "
              + SSID_t[(ssid_ix % SSID_SIZE)]
              + " nereusita dupa 10 s; incearca urmatorul SSID");
      ssid_ix += 1;
      return connectWifi();
    }
    if ((++count) % 10 == 0)
    {
      Serial.print(". ");
    }
    if (count == 100)
    {
      Serial.println();
      count = 0;
    }
  }
  Serial.println();
  server.begin();
  sendIp();
  writeLogger(
      String("[") + T_LOC + "] WiFi: conectat la "
          + SSID_t[(ssid_ix % SSID_SIZE)] + " ("
          + (1.0 * (millis() - mllis) / SECOND) + " s), adresa: "
          + WiFi.localIP().toString());
}

void sendHttpIndex(WiFiClient &client)
{
// Serial.println("Start sendHttpIndex");
  String r =
      "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\nRefresh: 10\r\n";
  client.println(r);
  String h = String("<!DOCTYPE html>\n") + "<html>" + "<head>\n"
      + " <title>CLLS " + T_LOC + "</title>\n" + " <style>\n"
      + "  html { height:100%; min-height:100%; width:100%; min-width:100%; }\n"
      + "  body { font-size:large; }\ninput { font-size:large; }\n"
      + "  table { border-collapse:collapse; border-style:solid; }\n"
      + "  th { padding:5px; border-style:solid; border-width: thin; }\n"
      + "  td { padding:5px; border-style:solid; border-width: thin; }\n"
      + " </style>\n" + "</head>\n" + "<body>\n" + "<table>\n" + " <tr>\n"
      + "  <th>Data</th>\n" + "  <td align='center' colspan='4'>" + timeString()
      + "</td>\n" + " </tr>\n" + " <tr>\n" + "  <td></td>\n"
      + "  <th align='center'>Temperatura</th>\n"
      + "  <th align='center'>Merge</th>\n"
      + "  <th align='center' colspan='2'>Program</th>\n" + " </tr>\n"
      + " <tr><td colspan='5'></td></tr>\n";
  client.println(h);

  for (byte i = 0, j = offset; i < SENZOR_COUNT; ++i, ++j)
  {
    String p =
        String("<!-- room: ") + room_name[j] + " -->\n" + " <tr>\n"
            + "  <th align='left'>" + room_name[j] + "</th>\n"
            + "  <td align='center'>" + ((1.0 * temperature[i]) / 100)
            + " &deg;C</td>\n" + "  <td align='center'>"
            + ((is_running[i]) ?
                "<font color='red'>DA</font>" : "<font color='blue'>NU</font>")
            + "</td>\n" + "  <td align='left'>" + printProgramming(i)
            + "</td>\n" + "  <td align='center'><form action='programming" + i
            + ".html' method='get'><input type='submit' value='Programare'></form></td>\n"
            + " </tr>\n\n";
    client.println(p);
  }
  String b = String("</table>\n") + "<p>Log:</p>\n<p>\n";
  for (int i = log_index; i >= 0; --i)
  {
    if (logger[i])
      b += String("") + (*(logger[i])) + "<br>\n";
  }
  for (int i = (MAX_LOGGER - 1); i > log_index; --i)
  {
    if (logger[i])
      b += String("") + (*(logger[i])) + "<br>\n";
  }
  b += "</p></body></html>";
  client.println(b);
// Serial.println("Start sendHttpIndex");
}

void sendHttpProgramming(WiFiClient &client, byte index)
{
// Serial.println("Start sendHttpProgramming");
  if (index >= SENZOR_COUNT)
    return;
  String r = "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\r\n";
  client.println(r);
  String h = String("<!DOCTYPE html>\n") + "<html>\n" + "<head>\n"
      + " <title>CLLS " + T_LOC + " - Programare</title>\n" + " <style>\n"
      + "  html { height:100%; min-height:100%; width:100%; min-width:100%; }\n"
      + "  body { font-size:large; }\ninput { font-size:large; }\n"
      + "  table { border-collapse:collapse; border-style:solid; }\n"
      + "  td { padding:5px; border-style:solid; border-width: thin; }\n"
      + " </style>\n" + "</head>\n" + "<body>\n" + "<table>\n"
      + "<form action='.' method='post'>\n<input type='hidden' name='"
      + T_LOC_NAME + "' value='" + T_LOC + "'>\n";
  client.println(h);
  String p =
      String("<!-- room: ") + room_name[(index + offset)] + " -->\n" + " <tr>\n"
          + "  <td rowspan=5>" + room_name[(index + offset)] + "</td>\n\n"
          + "<!-- programm: 0 -->\n" + "  <td align='left'>\n"
          + "   <label style='padding:5px;'><input type='radio' name='"
          + PROGRAMMING + "_" + index + "' value='" + P0_STOPPED
          + ((programming[index] == P0_STOPPED) ? "' checked" : "'") + ">"
          + printProgramming(index, P0_STOPPED) + "</label>\n" + "  </td>\n"
          + " </tr>\n\n" + "<!-- programm: 1 -->\n" + " <tr>\n"
          + "  <td align='left'>\n"
          + "   <label style='padding:5px;'><input type='radio' name='"
          + PROGRAMMING + "_" + index + "' value='" + P1_RUN_HOURS_MAKE_TEMP
          + ((programming[index] == P1_RUN_HOURS_MAKE_TEMP) ? "' checked" : "'")
          + ">" + printProgramming(index, P1_RUN_HOURS_MAKE_TEMP)
          + "</label><br>\n" + "   Start\n"
          + "   <input type='text' style='text-align:center;' name='"
          + START_HOUR_P1 + "_" + index + "' maxlength='2' size='1' value='"
          + start_hour_p1[index] + "'>:\n"
          + "   <input type='text' style='text-align:center;' name='"
          + START_MINUTE_P1 + "_" + index + "' maxlength='2' size='1' value='"
          + ((start_minute_p1[index] < 10) ? "0" : "") + start_minute_p1[index]
          + "'>\n" + "   Stop\n"
          + "   <input type='text' style='text-align:center;' name='"
          + STOP_HOUR_P1 + "_" + index + "' maxlength='2' size='1' value='"
          + stop_hour_p1[index] + "'>:\n"
          + "   <input type='text' style='text-align:center;' name='"
          + STOP_MINUTE_P1 + "_" + index + "' maxlength='2' size='1' value='"
          + ((stop_minute_p1[index] < 10) ? "0" : "") + stop_minute_p1[index]
          + "'>\n" + "   &deg;C\n"
              "   <input type='text' style='text-align:center;' name='"
          + TARGET_TEMPERATURE_P1 + "_" + index
          + "' maxlength='5' size='1' value='"
          + (1.0 * target_temperature_p1[index + offset] / 100) + "'>\n"
          + "  </td>\n" + " </tr>\n\n"
              "<!-- programm: 2 -->\n" + " <tr>\n" + "  <td align='left'>\n"
          + "   <label style='padding:5px;'><input type='radio' name='"
          + PROGRAMMING + "_" + index + "' value='"
          + P2_START_HOUR_1_INCREASE_05
          + ((programming[index] == P2_START_HOUR_1_INCREASE_05) ?
              "' checked" : "'") + ">"
          + printProgramming(index, P2_START_HOUR_1_INCREASE_05)
          + "</label><br>\n" + "   Start\n"
          + "   <input type='text' style='text-align:center;' name='"
          + START_HOUR_P2 + "_" + index + "' maxlength='2' size='1' value='"
          + start_hour_p2[index] + "'>:\n"
          + "   <input type='text' style='text-align:center;' name='"
          + START_MINUTE_P2 + "_" + index + "' maxlength='2' size='1' value='"
          + ((start_minute_p2[index] < 10) ? "0" : "") + start_minute_p2[index]
          + "'>\n" + "   Urmatorul program: \n"
          + "   <label><input type='radio' name='" + NEXT_PROGRAMM_P2 + "_"
          + index + "' value='" + P0_STOPPED
          + ((next_programm_p2[index] == P0_STOPPED) ? "' checked" : "'")
          + ">P0</label>\n" + "   <label><input type='radio' name='"
          + NEXT_PROGRAMM_P2 + "_" + index + "' value='"
          + P1_RUN_HOURS_MAKE_TEMP
          + ((next_programm_p2[index] == P1_RUN_HOURS_MAKE_TEMP) ?
              "' checked" : "'") + ">P1</label>\n"
          + "   <label><input type='radio' name='" + NEXT_PROGRAMM_P2 + "_"
          + index + "' value='" + P2_START_HOUR_1_INCREASE_05
          + ((next_programm_p2[index] == P2_START_HOUR_1_INCREASE_05) ?
              "' checked" : "'") + ">P2</label>\n"
          + "   <label><input type='radio' name='" + NEXT_PROGRAMM_P2 + "_"
          + index + "' value='" + P3_START_HOUR_2_INCREASE_05
          + ((next_programm_p2[index] == P3_START_HOUR_2_INCREASE_05) ?
              "' checked" : "'") + ">P3</label>\n" + "  </td>\n" + " </tr>\n\n";
  client.print(p);
  p =
      String("") + "<!-- programm: 3 -->\n" + " <tr>\n"
          + "  <td align='left'>\n"
          + "   <label style='padding:5px;'><input type='radio' name='"
          + PROGRAMMING + "_" + index + "' value='"
          + P3_START_HOUR_2_INCREASE_05
          + ((programming[index] == P3_START_HOUR_2_INCREASE_05) ?
              "' checked" : "'") + ">"
          + printProgramming(index, P3_START_HOUR_2_INCREASE_05)
          + "</label><br>\n" + "   Start"
          + "   <input type='text' style='text-align:center;' name='"
          + START_HOUR_P3 + "_" + index + "' maxlength='2' size='1' value='"
          + start_hour_p3[index] + "'>:\n"
          + "   <input type='text' style='text-align:center;' name='"
          + START_MINUTE_P3 + "_" + index + "' maxlength='2' size='1' value='"
          + ((start_minute_p3[index] < 10) ? "0" : "") + start_minute_p3[index]
          + "'>\n" + "   Urmatorul program:\n"
          + "   <label><input type='radio' name='" + NEXT_PROGRAMM_P3 + "_"
          + index + "' value='" + P0_STOPPED
          + ((next_programm_p3[index] == P0_STOPPED) ? "' checked" : "'")
          + ">P0</label>\n" + "   <label><input type='radio' name='"
          + NEXT_PROGRAMM_P3 + "_" + index + "' value='"
          + P1_RUN_HOURS_MAKE_TEMP
          + ((next_programm_p3[index] == P1_RUN_HOURS_MAKE_TEMP) ?
              "' checked" : "'") + ">P1</label>\n"
          + "   <label><input type='radio' name='" + NEXT_PROGRAMM_P3 + "_"
          + index + "' value='" + P2_START_HOUR_1_INCREASE_05
          + ((next_programm_p3[index] == P2_START_HOUR_1_INCREASE_05) ?
              "' checked" : "'") + ">P2</label>\n"
          + "   <label><input type='radio' name='" + NEXT_PROGRAMM_P3 + "_"
          + index + "' value='" + P3_START_HOUR_2_INCREASE_05
          + ((next_programm_p3[index] == P3_START_HOUR_2_INCREASE_05) ?
              "' checked" : "'") + ">P3</label>\n" + "  </td>\n" + " </tr>\n\n"
          + "<!-- programm: 4 -->\n" + " <tr>\n" + "  <td>\n"
          + "   <label style='padding:5px;'><input type='radio' name='"
          + PROGRAMMING + "_" + index + "' value='" + P4_START_NOW_INCREASE_05
          + ((programming[index] == P4_START_NOW_INCREASE_05) ?
              "' checked" : "'") + ">"
          + printProgramming(index, P4_START_NOW_INCREASE_05) + "</label><br>\n"
          + "   Urmatorul program:\n" + "   <label><input type='radio' name='"
          + NEXT_PROGRAMM_P4 + "_" + index + "' value='" + P0_STOPPED
          + ((next_programm_p4[index] == P0_STOPPED) ? "' checked" : "'")
          + ">P0</label>\n" + "   <label><input type='radio' name='"
          + NEXT_PROGRAMM_P4 + "_" + index + "' value='"
          + P1_RUN_HOURS_MAKE_TEMP
          + ((next_programm_p4[index] == P1_RUN_HOURS_MAKE_TEMP) ?
              "' checked" : "'") + ">P1</label>\n"
          + "   <label><input type='radio' name='" + NEXT_PROGRAMM_P4 + "_"
          + index + "' value='" + P2_START_HOUR_1_INCREASE_05
          + ((next_programm_p4[index] == P2_START_HOUR_1_INCREASE_05) ?
              "' checked" : "'") + ">P2</label>\n"
          + "   <label><input type='radio' name='" + NEXT_PROGRAMM_P4 + "_"
          + index + "' value='" + P3_START_HOUR_2_INCREASE_05
          + ((next_programm_p4[index] == P3_START_HOUR_2_INCREASE_05) ?
              "' checked" : "'") + ">P3</label>\n" + "  </td>\n" + " </tr>\n";
  client.println(p);
  String f = String("") + " <tr>\n" + "  <td colspan=4 align='center'>\n"
      + "   <input type='submit' value='Salvare'>\n" + "  </td>\n" + " </tr>\n"
      + "</form>\n" + "<form action='.' method='get'>\n" + " <tr>\n"
      + "  <td colspan=4 align='center'>\n"
      + "   <input type='submit' value='Inapoi'>\n" + "  </td>\n" + " </tr>\n"
      + "</form>\n" + "</table>\n" + "</body></html>\n";
  client.println(f);
// Serial.println("End sendHttpProgramming");
}

int savePostData_Programming(const String &data, int programm)
{
  int i = data.indexOf("=");
  if (i < 0)
  {
    writeLogger(
        String("[") + T_LOC
            + "] EROARE salvare programare - date invalide (lipseste semnul '='): "
            + data);
    return programm;
  }
  String name = data.substring(0, i);
  if (name.equals(T_LOC_NAME))
  {
    // ignore
    return programm;
  }
  String value = data.substring(i + 1);
  i = name.lastIndexOf("_");
  if (i < 0)
  {
    writeLogger(
        String("[") + T_LOC
            + "] EROARE salvare programare - date invalide (lipseste semnul '_'): "
            + data);
    return programm;
  }
  byte index = name.substring(i + 1).toInt();
  name = name.substring(0, i);
  int int_val = 0;
  bool valid = true;
  if (name.equals(TARGET_TEMPERATURE_P1))
  {
    float float_val = value.toFloat();
    valid = String(float_val).equals(value)
        || String(float_val).equals(value + "0")
        || String(float_val).equals(value + ".00");
    int_val = float_val * 100;
  }
  else
  {
    int_val = value.toInt();
    valid = String(int_val).equals(value)
        || String(String("0") + int_val).equals(value);
  }

  // bool valid = value.length() > 0 && (int_val != 0 || value.substring(0,1).equals("0"));
  if (name.equals(PROGRAMMING))
  {
    programm = int_val;
    if (!valid || programm < 0 || programm >= P_AFTER_LAST)
    {
      // error case
      writeLogger(
          String("[") + T_LOC
              + "] EROARE salvare programare - program necunoscut:'" + value
              + "'");
      return -1;
    }
    if (programm != programming[index])
    {
      writeLogger(
          String("[") + T_LOC + "] " + room_name[(index + offset)]
              + " - programul s-a schimbat, program nou: " + programming[index]
              + ", program vechi: " + programm);
    }
    programming[index] = programm;
    return programm;
  }
  String bad_name = "", old_value = "", bad_room = room_name[(index + offset)];
  if (name.equals(START_HOUR_P1))
  {
    if (valid)
      start_hour_p1[index] = int_val;
    else if (programm == P1_RUN_HOURS_MAKE_TEMP)
    {
      bad_name = "ora de inceput";
      old_value += start_hour_p1[index];
    }
    else
      valid = true;
  }
  if (name.equals(START_MINUTE_P1))
  {
    if (valid)
      start_minute_p1[index] = int_val;
    else if (programm == P1_RUN_HOURS_MAKE_TEMP)
    {
      bad_name = "minutul de inceput";
      old_value += start_minute_p1[index];
    }
    else
      valid = true;
  }
  if (name.equals(STOP_HOUR_P1))
  {
    if (valid)
      stop_hour_p1[index] = int_val;
    else if (programm == P1_RUN_HOURS_MAKE_TEMP)
    {
      bad_name = "ora de sfarsit";
      old_value += stop_hour_p1[index];
    }
    else
      valid = true;
  }
  if (name.equals(STOP_MINUTE_P1))
  {
    if (valid)
      stop_minute_p1[index] = int_val;
    else if (programm == P1_RUN_HOURS_MAKE_TEMP)
    {
      bad_name = "minutul de sfarsit";
      old_value += stop_minute_p1[index];
    }
    else
      valid = true;
  }
  if (name.equals(TARGET_TEMPERATURE_P1))
  {
    if (valid)
      target_temperature_p1[index + offset] = int_val;
    else if (programm == P1_RUN_HOURS_MAKE_TEMP)
    {
      bad_name = "temperatura";
      old_value += (1.0 * target_temperature_p1[index + offset] / 100);
    }
    else
      valid = true;
  }
  if (name.equals(START_HOUR_P2))
  {
    if (valid)
      start_hour_p2[index] = int_val;
    else if (programm == P2_START_HOUR_1_INCREASE_05
        || programm == P3_START_HOUR_2_INCREASE_05)
    {
      bad_name = "ora de inceput";
      old_value += start_hour_p2[index];
    }
    else
      valid = true;
  }
  if (name.equals(START_MINUTE_P2))
  {
    if (valid)
      start_minute_p2[index] = int_val;
    else if (programm == P2_START_HOUR_1_INCREASE_05
        || programm == P3_START_HOUR_2_INCREASE_05)
    {
      bad_name = "minutul de inceput";
      old_value += start_minute_p2[index];
    }
    else
      valid = true;
  }
  if (name.equals(START_HOUR_P3))
  {
    if (valid)
      start_hour_p3[index] = int_val;
    else if (programm == P2_START_HOUR_1_INCREASE_05
        || programm == P3_START_HOUR_2_INCREASE_05)
    {
      bad_name = "ora de inceput";
      old_value += start_hour_p3[index];
    }
    else
      valid = true;
  }
  if (name.equals(START_MINUTE_P3))
  {
    if (valid)
      start_minute_p3[index] = int_val;
    else if (programm == P2_START_HOUR_1_INCREASE_05
        || programm == P3_START_HOUR_2_INCREASE_05)
    {
      bad_name = "minutul de inceput";
      old_value += start_minute_p3[index];
    }
    else
      valid = true;
  }
  if (name.equals(NEXT_PROGRAMM_P2))
  {
    if (valid)
      next_programm_p2[index] = int_val;
    else if (programm == P2_START_HOUR_1_INCREASE_05
        || programm == P3_START_HOUR_2_INCREASE_05)
    {
      bad_name = "urmatorul program";
      old_value += next_programm_p2[index];
    }
    else
      valid = true;
  }
  if (name.equals(NEXT_PROGRAMM_P3))
  {
    if (valid)
      next_programm_p3[index] = int_val;
    else if (programm == P2_START_HOUR_1_INCREASE_05
        || programm == P3_START_HOUR_2_INCREASE_05)
    {
      bad_name = "urmatorul program";
      old_value += next_programm_p3[index];
    }
    else
      valid = true;
  }
  if (name.equals(NEXT_PROGRAMM_P4))
  {
    if (valid)
      next_programm_p4[index] = int_val;
    else if (programm == P4_START_NOW_INCREASE_05)
    {
      bad_name = "urmatorul program";
      old_value += next_programm_p4[index];
    }
    else
      valid = true;
  }
  if (!valid)
  {
//    Serial.println(String("ERROR programm ") + programm + " : " + data + " -> " + name + "[" + index + "] = " + value);
    writeLogger(
        String("[") + T_LOC
            + "] EROARE salvare programare - valoare invalida pentru "
            + bad_name + ":'" + value + "' (apelat cu '" + data + "') pentru "
            + bad_room + "; ramane valoarea dinainte: " + old_value);
  }
//  Serial.print(data); Serial.print(" -> ");Serial.print(name); Serial.print("[");Serial.print(index);Serial.print("] = ");Serial.println(value);
  return programm;
}

void processPostData_Programming(const String &post_data)
{
  int programm = -1;
  int i = 0, j = 0;
  while ((j = post_data.indexOf("&", i)) >= 0)
  {
    programm = savePostData_Programming(post_data.substring(i, j), programm);
    i = j + 1;
  }
  if (i)
    programm = savePostData_Programming(post_data.substring(i, j), programm);
  updatePxRunToday();
}

bool savePostData_Register(const String &data)
{
  int i = data.indexOf("=");
  if (i < 0)
  {
    writeLogger(
        String("[") + T_LOC
            + "] EROARE inregistrare - date invalide (lipseste semnul '='): "
            + data);
    return false;
  }
  String name = data.substring(0, i);
  String value = data.substring(i + 1);
  if (name.equals(REQUESTER_IP))
  {
    byte addr[4];
    requester_ip[last_register] = IPAddress(getIpBytes(value, addr));
    writeLogger(
        String("[") + T_LOC + "] Inregistrare notificari temperatura: "
            + value);
  }
  if (name.equals(REQUESTER_PORT))
  {
    requester_port[last_register] = value.toInt();
  }
  if (name.equals(REQUESTER_PAGE))
  {
    requester_page[last_register] = value;
  }
  return true;
}

bool processPostData_Register(const String &post_data)
{
  if (last_register >= MAX_REGISTER)
  {
    writeLogger(
        String("[") + T_LOC
            + "WARNING: Inregistrare notificari temperatura: registru plin!");
    return false;
  }
  int i = 0, j = 0;
  bool ok = true;
  while (ok && (j = post_data.indexOf("&", i)) >= 0)
  {
    ok = savePostData_Register(post_data.substring(i, j));
    i = j + 1;
  }
  if (ok && i)
  {
    ok = savePostData_Register(post_data.substring(i, j));
  }
  if(ok)
  {
    writeLogger(
        String("[") + T_LOC
            + "] Inregistrare notificari temperatura: " + IPAddress(requester_ip[last_register]).toString()
            + "; registri liberi: " + (MAX_REGISTER - last_register) + " din: " + MAX_REGISTER);
    ++last_register;
  }
  else
  {
    writeLogger(
        String("[") + T_LOC + "] EROARE - Inregistrare notificari temperatura: "
            + post_data);
  }
  return ok;
}

bool processPostData_Unregister(const String &post_data)
{
  int i = post_data.indexOf("=");
  if (i < 0)
  {
    writeLogger(
        String("[") + T_LOC
            + "] EROARE deinregistrare - date invalide (lipseste semnul '='): "
            + post_data);
    return false;
  }
  int j = post_data.indexOf("&");
  String name = post_data.substring(0, i);
  if (name.equals(REQUESTER_IP))
  {
    ++i;
    String value = (j > i) ? post_data.substring(i, j) : post_data.substring(i);
    bool found = false;
    byte addr[4];
    IPAddress ip(getIpBytes(value, addr));
    for (int k = 0; k < last_register; ++k)
    {
      if (requester_ip[k] == ip)
        found = true;
      if (found)
      {
        requester_ip[k] = requester_ip[last_register - 1];
        requester_port[k] = requester_port[last_register - 1];
        requester_page[k] = requester_page[last_register - 1];
        --last_register;
        --k;
        writeLogger(
            String("[") + T_LOC + "] Deinregistrare notificari temperatura: "
                + value);
        writeLogger(
            String("[") + T_LOC
                + "] Deinregistrare notificari temperatura; registri liberi: "
                + (MAX_REGISTER - last_register) + " din: " + MAX_REGISTER);
      }
    }
    return true;
  }
  else
  {
    writeLogger(
        String("[") + T_LOC
            + "] EROARE deinregistrare - date invalide (lipseste "
            + REQUESTER_IP + "): " + post_data);
  }
  return false;
}

void saveRegister(const IPAddress & ip, const int port, const String & page)
{
  if (last_register >= MAX_REGISTER)
  {
    writeLogger(
        String("[") + T_LOC
            + "WARNING: Inregistrare notificari temperatura: toti registrii sunt plini!");
    return;
  }
  requester_ip[last_register] = ip;
  requester_port[last_register] = port;
  requester_page[last_register] = page;
  ++last_register;
  writeLogger(
      String("[") + T_LOC + "] Inregistrare notificari temperatura: "
          + IPAddress(ip).toString());
  writeLogger(
      String("[") + T_LOC
          + "] Inregistrare notificari temperatura; registri liberi: "
          + (MAX_REGISTER - last_register) + " din: " + MAX_REGISTER);
}

void sendPostData(const String &data, const String &page,
    const IPAddress & server, const int port)
{
  String post_data = String(T_LOC_NAME) + "=" + T_LOC + "&" + data;
  String post_req = String("") + "POST " + page + " HTTP/1.1\r\n" + "Host: "
      + WiFi.localIP().toString() + "\r\n" + "User-Agent: Arduino/1.0\r\n"
      + "Connection: close\r\n"
      + "Content-Type: application/x-www-form-urlencoded\r\n"
      + "Content-Length: " + post_data.length() + "\r\n";
  WiFiClient client;
  if (client.connect(server, port))
  {
    client.println(post_req);
    client.println(post_data);
    delay(10);
    client.stop();
  }
}

void sendCurrentTemperatures()
{
  String post_data = String("");
  for (byte i = 0; i < SENZOR_COUNT; ++i)
  {
    if (i)
      post_data += "&";
    post_data += String("") + "t_" + i + "=" + temperature[i] + "&t_" + i
        + "_r=" + (is_running[i] ? "1" : "0");
  }
//Serial.println(String("sendCurrentTemperatures post_data: ") + post_data);
  for (int i = 0; i < last_register; ++i)
  {
//Serial.println(String("sendCurrentTemperatures send to: ") + IPAddress(requester_ip[i]).toString());
    sendPostData(post_data, requester_page[i], requester_ip[i],
        requester_port[i]);
  }
}

void sendCurrentTemperatures(const String &page, const IPAddress & server,
    const int port)
{
  // send data to the server 
  String post_data = String("");
  for (byte i = 0; i < SENZOR_COUNT; ++i)
  {
    if (i)
      post_data += "&";
    post_data += String("") + "t_" + i + "=" + temperature[i] + "&t_" + i
        + "_r=" + (is_running[i] ? "1" : "0");
  }
//  Serial.println(String("Send post to ") + page + ": " + post_data);
  sendPostData(post_data, page, server, port);
}

void listen4HttpClient()
{
  if (WiFi.status() != WL_CONNECTED)
    connectWifi();
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("ERROR: WiFi connection failed.");
    return;
  }
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client)
  {
// Serial.println("new HTTP request");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean hasReadPostData = false;
    String req_str = "", post_data = "";
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        req_str += c;
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank)
        {
          if (req_str.indexOf("GET /programming0.html") >= 0)
          {
            sendHttpProgramming(client, 0);
            break;
          }
          if (req_str.indexOf("GET /programming1.html") >= 0)
          {
            sendHttpProgramming(client, 1);
            break;
          }
          if (req_str.indexOf("GET /programming2.html") >= 0)
          {
            sendHttpProgramming(client, 2);
            break;
          }
          if (req_str.indexOf("GET /programming3.html") >= 0)
          {
            sendHttpProgramming(client, 3);
            break;
          }
          if (req_str.indexOf("GET /") >= 0)
          {
            sendHttpIndex(client);
            break;
          }
          if (req_str.indexOf("POST") >= 0)
          {
            if (!hasReadPostData)
            {
              int content_length = 0;
              parseRequest(req_str, "Content-Length:", content_length);
              while (content_length-- > 0)
              {
                c = client.read();
                post_data += c;
              }
              hasReadPostData = true;
              String what;
              parseRequest(req_str, "Accept:", what);
//Serial.println(what); // programming
              if (what.compareTo("programming") == 0) // send programming
              {
                processPostData_Programming(post_data);
                got_programming = true;
                //TODO: nice to have: recompose programming, don't just send it back
                sendPostData(post_data, "/programming_update.php");
              }
              else if (what.compareTo("register") == 0) // register
              {
                // unregister first
//Serial.println(String("Register HTTP req:\n") + req_str + "\n\n post_data: " + post_data);
                if (processPostData_Unregister(post_data) &&
                    processPostData_Register(post_data))
                  sendHttpResponse(client, OK_CODE);
                else
                  sendHttpResponse(client, NOK_CODE);
                break;
              }
              else if (what.compareTo("unregister") == 0) // register
              {
                if(processPostData_Unregister(post_data))
                  sendHttpResponse(client, OK_CODE);
                else
                  sendHttpResponse(client, NOK_CODE);
                break;
              }
            }
            sendHttpIndex(client);
            break;
          }
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
// Serial.println(String("HTTP req: ") + req_str + " post_data: " + post_data);
    // close the connection:
    client.stop();
  }
}

byte* getIpBytes(String& host, byte *addr)
{
  int start_idx = 0;
  int stop_idx = 0;
  int count = -1;
  for (byte count = 0; count < 4; ++count)
  {
    stop_idx = host.indexOf(".", start_idx + 1);
    if (stop_idx >= 0)
      addr[count] = host.substring(start_idx, stop_idx).toInt();
    else if (start_idx >= 0)
      addr[count] = host.substring(start_idx).toInt();
    start_idx = stop_idx + 1;
  }
  return addr;
}

void startConversion_1()
{
  // DallasTemperature.h :: sends command for all devices on the bus to perform a temperature conversion
  byte i = 0;
  byte j = i + offset;
// Serial.println(String("request conversion i=") + i + " j=" + j + ", millis=" + millis());
  dallasTemperature1st_pin0.requestTemperaturesByAddress(SENZOR_ADDRESS[j]);
}

void startConversion_2()
{
  // DallasTemperature.h :: sends command for all devices on the bus to perform a temperature conversion
  byte i = 1;
  byte j = i + offset;
// Serial.println(String("request conversion i=") + i + " j=" + j + ", millis=" + millis());
  dallasTemperature1st_pin0.requestTemperaturesByAddress(SENZOR_ADDRESS[j]);
}

void startConversion_3()
{
  // DallasTemperature.h :: sends command for all devices on the bus to perform a temperature conversion
  byte i = 2;
  byte j = i + offset;
// Serial.println(String("request conversion i=") + i + " j=" + j + ", millis=" + millis());
  dallasTemperature2nd_pin2.requestTemperaturesByAddress(SENZOR_ADDRESS[j]);
}

void startConversion_4()
{
  // DallasTemperature.h :: sends command for all devices on the bus to perform a temperature conversion
  byte i = 3;
  byte j = i + offset;
// Serial.println(String("request conversion i=") + i + " j=" + j + ", millis=" + millis());
  dallasTemperature2nd_pin2.requestTemperaturesByAddress(SENZOR_ADDRESS[j]);
}

void updateTemperature()
{
  long int m = millis();
  m = m - (m % SECOND) + SECOND; // floor to second
  // scheduler next read
  long int delta =
      ((SECOND * TEMP_READ_INTERVAL) >= (4 * CONVERSION_TIME)) ?
          SECOND * TEMP_READ_INTERVAL : 4 * CONVERSION_TIME;
  long int next_read = m + delta; // seconds
  int temp = 0;
  // DallasTemperature.h::getTempC - returns temperature in degrees C for given address
  // read the temperature and store it as integer rounded to 0.05 grd C
  for (byte i = 0, j = i + offset; i < SENZOR_COUNT / 2; ++i, ++j)
  {
// Serial.println(String("read temperature i=") + i + " j=" + j);
    temp = floatToRound05Int(
        dallasTemperature1st_pin0.getTempC(SENZOR_ADDRESS[j]));
    if (temp <= -4000 || temp >= 7000)
    {
      Serial.println(String("ERROR: bad temperature: ") + i + " - " + temp);
      temp = 0;
    }
    if (temp)
      temperature[i] = temp;
  }
  for (byte i = SENZOR_COUNT / 2, j = i + offset; i < SENZOR_COUNT; ++i, ++j)
  {
// Serial.println(String("read temperature i=") + i + " j=" + j);
    temp = floatToRound05Int(
        dallasTemperature2nd_pin2.getTempC(SENZOR_ADDRESS[j]));
    if (temp <= -4000 || temp >= 7000)
    {
      Serial.println(String("ERROR: bad temperature: ") + i + " - " + temp);
      temp = 0;
    }
    if (temp)
      temperature[i] = temp;
  }
  // schedule conversion at'next_read' time, minus the time to wait for it
  scheduler.add(startConversion_1, next_read - 4 * CONVERSION_TIME);
  scheduler.add(startConversion_3, next_read - 3 * CONVERSION_TIME);
  scheduler.add(startConversion_2, next_read - 2 * CONVERSION_TIME);
  scheduler.add(startConversion_4, next_read - 1 * CONVERSION_TIME);
  // print it
  pritSerial();

  // schedule programm checking ofter 10 ms
  scheduler.add(checkProgramming, 10);
  // schedule read at'next_read' time
  scheduler.add(updateTemperature, next_read);
}

bool isNowBetween(byte start_h, byte start_m, byte stop_h, byte stop_m)
{
  int minutes_start = start_h * 60 + start_m;
  int minutes_end = stop_h * 60 + stop_m;
  // check continuous running
  if (minutes_start == minutes_end)
    return true;
  bool next_day = minutes_end < minutes_start;
  int minutes_now = hour() * 60 + minute();
  if (next_day)
    return minutes_start <= minutes_now || minutes_now < minutes_end;
  else
    return minutes_start <= minutes_now && minutes_now < minutes_end;
  return false;
}

bool isNowAfter(byte start_h, byte start_m)
{
  int minutes_now = hour() * 60 + minute();
  int minutes_start = start_h * 60 + start_m;
  return minutes_start <= minutes_now;
}

void checkProgramming()
{
// Serial.println("Start checkProgramming");
  bool should_run[] =
  { false, false, false, false, };
  byte original_programming[] =
  { P_NONE, P_NONE, P_NONE, P_NONE };
  bool send_programming_update = false;
  bool send_target_temperature_update = false;
  bool run_marked = false;
  bool one_delta_running = false;
  String programming_post = "";
  String target_temperature_post = "";
  for (byte i = 0; i < SENZOR_COUNT; ++i)
  {
    // save original programming
    original_programming[i] = programming[i];
    // reset forced running
    force_running[i] = false;
  }
  for (byte i = 0; i < SENZOR_COUNT; ++i)
  {
    if (!temperature[i])
      continue;
    // on a new day reset has_pX_run_today
    if (hour() == 0 && minute() == 0)
    {
      bool p2_run_today = has_p2_run_today[i];
      bool p3_run_today = has_p3_run_today[i];
      switch (programming[i])
      {
        case P2_START_HOUR_1_INCREASE_05:
        {
          if (!is_running[i])
            has_p2_run_today[i] = false;
          has_p3_run_today[i] = false;
          break;
        }
        case P3_START_HOUR_2_INCREASE_05:
        {
          if (!is_running[i])
            has_p3_run_today[i] = false;
          has_p2_run_today[i] = false;
          break;
        }
        default:
        {
          has_p2_run_today[i] = false;
          has_p3_run_today[i] = false;
          break;
        }
      }
      run_marked = run_marked || (p2_run_today != has_p2_run_today[i])
          || (p3_run_today != has_p3_run_today[i]);
    }
    switch (programming[i])
    {
      case P1_RUN_HOURS_MAKE_TEMP: // keep target temperature during time interval
      {
        int target_temp = target_temperature_p1[i + offset];
        // make a little more then desired to prevent bouncing
        if (is_running[i])
          target_temp += DELTA_P1_TEMP;
        should_run[i] = temperature[i] <= target_temp
            && isNowBetween(start_hour_p1[i], start_minute_p1[i],
                stop_hour_p1[i], stop_minute_p1[i]);
        break;
      }
      case P2_START_HOUR_1_INCREASE_05: // start at given time and increase temperature with DELTA_TEMP
      {
        should_run[i] = !has_p2_run_today[i]
            && isNowAfter(start_hour_p2[i], start_minute_p2[i]);
        if (should_run[i] && !target_temperature_p2[i])
        {
          target_temperature_p2[i] = temperature[i] + DELTA_TEMP;
          send_target_temperature_update = true;
          target_temperature_post += String("&") + TARGET_TEMPERATURE_P2 + "_"
              + i + "=" + ((1.0 * target_temperature_p2[i]) / 100);
        }
        should_run[i] = should_run[i]
            && temperature[i] <= target_temperature_p2[i];
        if (should_run[i])
        {
          one_delta_running = true;
        }
        else if (is_running[i])
        {
          has_p2_run_today[i] = true;
          target_temperature_p2[i] = 0;
          send_target_temperature_update = true;
          target_temperature_post += String("&") + TARGET_TEMPERATURE_P2 + "_"
              + i + "=0.0";
          if (programming[i] != next_programm_p2[i])
          {
            programming[i] = next_programm_p2[i];
            writeLogger(
                String("[") + T_LOC + "] " + room_name[(i + offset)]
                    + " - programul s-a schimbat de la "
                    + original_programming[i] + " la " + programming[i]);
            send_programming_update = true;
            programming_post += String("&") + PROGRAMMING + "_" + i + "="
                + programming[i];
          }
        }
        break;
      }
      case P3_START_HOUR_2_INCREASE_05: // start at given hour and increase temperature with DELTA_TEMP
      {
        should_run[i] = !has_p3_run_today[i]
            && isNowAfter(start_hour_p3[i], start_minute_p3[i]);
        if (should_run[i] && !target_temperature_p3[i])
        {
          target_temperature_p3[i] = temperature[i] + DELTA_TEMP;
          send_target_temperature_update = true;
          target_temperature_post += String("&") + TARGET_TEMPERATURE_P3 + "_"
              + i + "=" + ((1.0 * target_temperature_p3[i]) / 100);
        }
        should_run[i] = should_run[i]
            && temperature[i] <= target_temperature_p3[i];
        if (should_run[i])
        {
          one_delta_running = true;
        }
        else if (is_running[i])
        {
          has_p3_run_today[i] = true;
          target_temperature_p3[i] = 0;
          send_target_temperature_update = true;
          target_temperature_post += String("&") + TARGET_TEMPERATURE_P3 + "_"
              + i + "=0.0";
          if (programming[i] != next_programm_p3[i])
          {
            programming[i] = next_programm_p3[i];
            writeLogger(
                String("[") + T_LOC + "] " + room_name[(i + offset)]
                    + " - programul s-a schimbat de la "
                    + original_programming[i] + " la " + programming[i]);
            send_programming_update = true;
            programming_post += String("&") + PROGRAMMING + "_" + i + "="
                + programming[i];
          }
        }
        break;
      }
      case P4_START_NOW_INCREASE_05: // start now and increase temperature with DELTA_TEMP
      {
        if (!target_temperature_p4[i])
        {
          target_temperature_p4[i] = temperature[i] + DELTA_TEMP;
          send_target_temperature_update = true;
          target_temperature_post += String("&") + TARGET_TEMPERATURE_P4 + "_"
              + i + "=" + ((1.0 * target_temperature_p4[i]) / 100);
        }
        should_run[i] = temperature[i] <= target_temperature_p4[i];
        if (should_run[i])
        {
          one_delta_running = true;
        }
        else if (is_running[i])
        {
          target_temperature_p4[i] = 0;
          send_target_temperature_update = true;
          target_temperature_post += String("&") + TARGET_TEMPERATURE_P4 + "_"
              + i + "=0.0";
          if (programming[i] != next_programm_p4[i])
          {
            programming[i] = next_programm_p4[i];
            writeLogger(
                String("[") + T_LOC + "] " + room_name[(i + offset)]
                    + " - programul s-a schimbat de la "
                    + original_programming[i] + " la " + programming[i]);
            send_programming_update = true;
            programming_post += String("&") + PROGRAMMING + "_" + i + "="
                + programming[i];
          }
        }
        break;
      }
      default: // stopped
      {
        should_run[i] = false;
        break;
      }
    }
  }

  if (run_marked)
    writeLogger(
        String("[") + T_LOC + "]  P2 si P3 marcate ca nu au rulat astazi");

  if (send_programming_update || send_target_temperature_update)
  {
    String post_data = "";
    if (send_programming_update)
      post_data += programming_post;
    if (send_target_temperature_update)
      post_data += target_temperature_post;
    if (post_data.length() > 0)
      // remove leading "&"
      post_data = post_data.substring(1);
    // send it
    sendPostData(post_data, "/programming_update.php");
  }

  // if one delta temp is running start all P1 with current temperature less then MAX_DELTA centi grades greater then the target
  if (one_delta_running)
  {
    // start all P1 not running
    for (int i = 0; i < SENZOR_COUNT; ++i)
    {
      if (should_run[i] || programming[i] != P1_RUN_HOURS_MAKE_TEMP)
        continue;
      int candidate_temp = temperature[i] - target_temperature_p1[i + offset];
      // add a bonus to already running to prevent bouncing
      if (is_running[i])
        candidate_temp -= DELTA_P1_TEMP;
      force_running[i] = candidate_temp < 2 * MAX_DELTA;
    }
  }
  else
  {
    // start up to MIN_RUNNING
    byte count_running = 0;
    byte count_candidates = 0;
    byte candidates[] =
    { P_NONE, P_NONE, P_NONE, P_NONE, };
    const int MAX_TEMP = 10000;
    int candidates_temp[] =
    { MAX_TEMP, MAX_TEMP, MAX_TEMP, MAX_TEMP, };
    for (int i = 0; i < SENZOR_COUNT; ++i)
    {
      if (should_run[i])
        ++count_running;
      else if (programming[i] == P1_RUN_HOURS_MAKE_TEMP)
      {
        candidates_temp[i] = temperature[i] - target_temperature_p1[i + offset];
        // add a bonus to already running to prevent bouncing
        if (is_running[i])
          candidates_temp[i] -= DELTA_P1_TEMP;
        candidates[i] = i;
        ++count_candidates;
      }
    }

    // at least one is running, but less then minimum, and there are candidates
    if (count_running > 0 && count_running < MIN_RUNNING
        && count_candidates > 0)
    {
      // sort by delta temperature, highest delta first
      for (int i = 0; i < SENZOR_COUNT; ++i)
      {
        for (int j = i + 1; j < SENZOR_COUNT; ++j)
        {
          if (candidates_temp[j] < candidates_temp[i])
          {
            // swap
            int t_temp = candidates_temp[j];
            candidates_temp[j] = candidates_temp[i];
            candidates_temp[i] = t_temp;
            byte t = candidates[j];
            candidates[j] = candidates[i];
            candidates[i] = t;
          }
        }
      }
      byte forced_running = 0;
      for (int i = 0; i < SENZOR_COUNT; ++i)
      {
        if (candidates[i] != P_NONE && candidates_temp[i] < MAX_DELTA)
        {
          // start it
          force_running[candidates[i]] = true;
          ++forced_running;
          // count it
          if (++count_running >= MIN_RUNNING)
            break;
        }
      }
      // just one is running, no one forced -> start first candidate
      if (count_running == 1 && forced_running == 0)
      {
        force_running[candidates[0]] = true;
      }
    }
  }

  for (int i = 0, j = offset; i < SENZOR_COUNT; ++i, ++j)
  {
    if (should_run[i] && !is_running[i])
    {
      // start
      digitalWrite(relay[i], LOW);
      float temp = (1.0 * temperature[i] / 100);
      writeLogger(
          String("[") + T_LOC + "] " + room_name[j] + " - start program "
              + printProgramming(i) + "; temperatura curenta " + temp);
      delay(25);
    }
  }
  for (int i = 0, j = offset; i < SENZOR_COUNT; ++i, ++j)
  {
    if (force_running[i] && !is_running[i])
    {
      // start
      digitalWrite(relay[i], LOW);
      float temp = (1.0 * temperature[i] / 100);
      writeLogger(
          String("[") + T_LOC + "] " + room_name[j] + " - start fortat program "
              + printProgramming(i) + "; temperatura curenta " + temp);
      delay(25);
    }
  }
  for (int i = 0, j = offset; i < SENZOR_COUNT; ++i, ++j)
  {
    if (!should_run[i] && !force_running[i] && is_running[i])
    {
      // stop
      digitalWrite(relay[i], HIGH);
      float temp = (1.0 * temperature[i] / 100);
      writeLogger(
          String("[") + T_LOC + "] " + room_name[j] + " - stop program "
              + original_programming[i] + "; temperatura curenta " + temp);
      delay(25);
    }
  }
  for (int i = 0, j = offset; i < SENZOR_COUNT; ++i, ++j)
  {
    is_running[i] = should_run[i] || force_running[i];
  }

  // send data to the server 
  sendCurrentTemperatures();
}

int floatToRound05Int(float temp)
{
  int t = (int) (temp * 100);
  byte mod = t % 10;
  switch (mod)
  {
    case 1:
    case 2:
      t += (0 - mod);
    break;
    case 3:
    case 4:
    case 6:
    case 7:
      t += (5 - mod);
    break;
    case 8:
    case 9:
      t += (10 - mod);
    break;
    default:
    break;
  }
  return t;
}

String timeString(int day_t, int month_t, int year_t, int hour_t, int minute_t,
    int second_t)
{
  char buff[20];
  sprintf(buff, "%02d-%02d-%04d %02d:%02d:%02d", day_t, month_t, year_t, hour_t,
      minute_t, second_t);
  return String(buff);
}

time_t getTime()
{
  if (WiFi.status() != WL_CONNECTED)
    connectWifi();
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("ERROR: WiFi connection failed.");
    return 0;
  }

  unsigned long mllis = millis();
  // Initializes the WiFi UDP library and network settings. Starts WiFiUDP socket, listening at local port 12670
  static int udpInitialized = udp.begin(12670);
  if (0 == udpInitialized) // returns 0 if there are no sockets available to use
  {
    Serial.println("ERROR: there are no sockets available to use.");
    return 0;
  }
  static char timeServer[] = "ro.pool.ntp.org";  // the NTP server
  // static char timeServer[] = "time.nist.gov";  // the NTP server
  static long ntpFirstFourBytes = 0xEC0600E3; // the NTP request header

  udp.flush(); // Clear received data from possible stray received packets

  // Send an NTP request to timeserver on NTP port: 123
  if (!(udp.beginPacket(timeServer, 123)
      && udp.write((byte *) &ntpFirstFourBytes, PKT_LEN) == PKT_LEN
      && udp.endPacket()))
  {
    Serial.println("NTP ERROR: sending request failed");
    return 0; // sending request failed
  }

  int pktLen;               // received packet length
  // Wait for NTP server response; check every POLL_INTERVAL ms up to POLL_TIMES times
  byte j = 0;
  for (; j < POLL_TIMES; j++)
  {
    if ((pktLen = udp.parsePacket()) == PKT_LEN)
      break;
    delay(POLL_INTERVAL);
  }
  if (pktLen != PKT_LEN)
  {
    Serial.println();
    Serial.print("NTP ERROR: no correct packet received; pktLen = ");
    Serial.print(pktLen);
    Serial.println(", expected 48");
    return 0; // no correct packet received
  }

  // Read and discard the first useless bytes
  for (byte i = 0; i < USELES_BYTES; ++i)
    udp.read();

  // Read the integer part of sending time
  time_t t_time = udp.read();  // NTP time
  for (byte i = 1; i < 4; i++)
    t_time = t_time << 8 | udp.read();

  // Round to the nearest second if we want accuracy
  // The fractionary part is the next byte divided by 256: if it is
  // greater than 500ms we round to the next second; we also account
  // for an assumed network delay of 30ms, and (0.5-0.05)*256=120;
  // additionally, we account for how much we delayed reading the packet
  // since its arrival, which we assume on average to be POLL_INTERVAL/2.
  t_time += (udp.read() > 120 - POLL_INTERVAL / 8);

  // Discard the rest of the packet
  udp.flush();
  //  WiFi.disconnect();

  // convert NTP time to GMT time
  t_time -= 2208988800ul;
  // convert Unix to locale (EET | EEST)
  t_time = EasternEuropeanTime.toLocal(t_time, &tcr);
  // set offset
  if (!start_second)
    start_second = t_time - secondsRunning();

  defere_log(
      String("[") + T_LOC + "] NTP primit ora exacta (" + (millis() - mllis)
          + " ms).");
  scheduler.add(recal_log, 20);
  if (no_time)
    no_time = false;
  return t_time;
}

int lost_msg_count = 1;

void defere_log(const String &what)
{
  if (deferred_index >= MAX_LOGGER)
  {
    ++lost_msg_count;
    // clean old last position
    delete deferred_log[MAX_LOGGER - 1];
    deferred_log[MAX_LOGGER - 1] = new String(
        String("Deffered log overflow by ") + lost_msg_count + " messages!");
    deferred_time[MAX_LOGGER - 1] = secondsRunning();
    return;
  }
  deferred_log[deferred_index] = new String(what);
  deferred_time[deferred_index] = secondsRunning();
  ++deferred_index;
}

void recal_log()
{
  if (deferred_index > 0)
  {
    for (int i = 0; i < deferred_index; i++)
    {
      time_t t_time = start_second + deferred_time[i];
      String *t_df_log = deferred_log[i];
      String post_data = String(
          timeString(day(t_time), month(t_time), year(t_time), hour(t_time),
              minute(t_time), second(t_time))) + " - " + *t_df_log;
      log_index = (log_index + 1) % MAX_LOGGER;
      String *t_log = logger[log_index];
      logger[log_index] = new String(post_data);
      Serial.println(post_data);
      post_data = String("t_log=") + post_data;
      sendPostData(post_data, "/log_save.php");
      delete t_df_log;
      delete t_log;
      deferred_log[i] = 0;
    }
    deferred_index = 0;
  }
}

void writeLogger(String &what)
{
  if (no_time)
  {
    defere_log(what);
    return;
  }
  recal_log();
  String post_data = String(timeString()) + " - " + what;
  log_index = (log_index + 1) % MAX_LOGGER;
  String *o_log = logger[log_index];
  logger[log_index] = new String(post_data);
  Serial.println(post_data);
  post_data = String("t_log=") + post_data;
  sendPostData(post_data, "/log_save.php");
  delete o_log;
}

void sendHttpResponse(WiFiClient &client, const String &code)
{
  client.println(String("HTTP/1.1 ") + code + "\r\nConnection: close\r\n");
}

void sendIp()
{
  String post_data = String("t_ip=") + WiFi.localIP().toString();
  sendPostData(post_data, "/ip_save.php");
}

/*
 // libraries/LsuScheduler/LsuScheduler.h
 #ifndef LsuScheduler_h
 #define LsuScheduler_h

 #include "Arduino.h"

 class LsuScheduler
 {
 private:
 struct node
 {
 long int when;
 void (*funt)(void);
 struct node * next;
 };
 node * head;
 void destroy(node *);
 void destroy_r(node *, node *);
 public:
 LsuScheduler();
 virtual ~LsuScheduler();
 void execute(long int);
 void add(void (*)(), long int );
 };

 #endif // LsuScheduler_h
 // end LsuScheduler.h
 */

/*
 // libraries/LsuScheduler/LsuScheduler.cpp
 #include "Arduino.h"
 #include "LsuScheduler.h"

 void LsuScheduler::destroy(node *c)
 {
 if (c)
 destroy_r(c, c->next);
 }

 void LsuScheduler::destroy_r(node *c, node *n)
 {
 if (c)
 delete c;
 if (n)
 destroy_r(n, n->next);
 }

 LsuScheduler::LsuScheduler()
 {
 head = new node();
 head->when = 0;
 head->funt = 0;
 head->next = 0;
 }

 LsuScheduler::~LsuScheduler()
 {
 destroy(head);
 }

 void LsuScheduler::execute(long int current_time)
 {
 node * c = head;
 while (c->next)
 {
 node * n = c->next;
 if (n->when <= current_time)
 {
 // execute
 if (n->funt)
 n->funt();
 // remove
 c->next = n->next;
 delete n;
 n = 0;
 }
 // advance
 c = c->next;
 }
 }

 void LsuScheduler::execute(long int current_time)
 {
 node * c = head;
 while (c->next)
 {
 node * n = c->next;
 if (n->when <= current_time)
 {
 // execute
 if (n->funt)
 n->funt();
 // remove
 c->next = n->next;
 delete n;
 n = 0;
 }
 // advance
 c = c->next;
 }
 }

 void LsuScheduler::add(void (*funt)(), long int when)
 {
 node * n = new node();
 n->when = when;
 n->funt = funt;
 n->next = 0;
 // start at head
 node * last = head;
 // search the last one
 while (last->next)
 last = last->next;
 // link after last
 last->next = n;
 }
 // end LsuScheduler.cpp
 */

