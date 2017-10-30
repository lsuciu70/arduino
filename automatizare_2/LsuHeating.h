#ifndef _LsuHeating_H_
#define _LsuHeating_H_

#include <Arduino.h>

#include <WiFiUdp.h>

#include <OneWire.h>           // https://github.com/PaulStoffregen/OneWire
#include <DallasTemperature.h> // https://github.com/milesburton/Arduino-Temperature-Control-Library

#include <LsuWiFi.h>
#include <LsuNtpTime.h>
#include <LsuScheduler.h>

#define USE_SERIAL Serial

#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 2

namespace
{

// floor section
// parter
const char* T_LOC_PARTER = "parter";
const char* MAC_PARTER = "18:FE:34:D4:0D:EC"; // old
//const char* MAC_PARTER = "5C:CF:7F:EF:BE:50"; // new
// etaj
const char* T_LOC_ETAJ = "etaj";
const char* MAC_ETAJ = "5C:CF:7F:88:EE:49";

char T_LOC[7] = {'\0'};
// end floor section

// WiFi section
const uint8_t SSID_SIZE = 3;
const char* SSIDs[SSID_SIZE] =
{ "cls-router", "cls-ap", "lsu-tpr" };
const char* SSID_PASSWDs[SSID_SIZE] =
{ "r4cD7TPG", "r4cD7TPG", "r4cD7TPG" };

const IPAddress master_server_ip(192, 168, 100, 100);
const int master_server_port = 8081;
const char* page = "/update.php";
// end WiFi section

// NTP time section
char datetimebuff[LsuNtpTime::DMY_HMS_STR_LEN + 1];
// end NTP section

// reading temperature section
// Feather HUZZAH pins
const uint8_t GPIO_0 = 0; // 1st temperature sensor, OneWire, DallasTemperature
const uint8_t GPIO_2 = 2; // 2nd temperature sensor, OneWire, DallasTemperature
const uint8_t GPIO_12 = 12; // 1st room, Camera Luca | Bucatarie, relay 1
const uint8_t GPIO_13 = 13; // 2nd room, Dormitor matrimonial | Living, relay 2
const uint8_t GPIO_14 = 14; // 3rd room, Dormitor oaspeti | Birou, relay 3
const uint8_t GPIO_15 = 15; // 4th room, Baie etaj | Baie parter, relay 4

// one second, 1000 milliseconds
const int SECOND = 1000;
// The interval temperature is read
const uint8_t TEMP_READ_INTERVAL = 10;
// Temperature senzor resolution: 9, 10, 11, or 12 bits
const uint8_t RESOLUTION = 12;
// Calculate conversion time and add 10 ms
const int CONVERSION_TIME = 10 + 750 / (1 << (12 - RESOLUTION));
// Sensor count
const uint8_t SENZOR_COUNT = 4;

const uint8_t SENZOR_ADDRESS_LENGTH = 8;
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

int temperature[SENZOR_COUNT];

// Setup a oneWire instance to communicate with OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire1st_pin0(GPIO_0);
OneWire oneWire2nd_pin2(GPIO_2);

// Pass oneWire reference to Dallas Temperature.
DallasTemperature dallasTemperature1st_pin0(&oneWire1st_pin0);
DallasTemperature dallasTemperature2nd_pin2(&oneWire2nd_pin2);

uint8_t offset = 0;
// end reading temperature section

// relay section
// Feather HUZZAH relay's pins
const uint8_t relay[] =
{ GPIO_12, GPIO_13, GPIO_14, GPIO_15, };

bool isRelayInitialized = false;
// end relay section

// programming section
const uint8_t DELTA_TEMP = 30;
uint8_t start_hour_p2 = 3;
uint8_t start_minute_p2 = 30;
int target_temperature_p2 = 0;
bool has_p2_run_today = false;
bool is_running = false;

void relayInitialize()
{
  if (isRelayInitialized)
    return;
  for (uint8_t i = 0; i < SENZOR_COUNT; ++i)
  {
    temperature[i] = 0;
    pinMode(relay[i], OUTPUT);
    digitalWrite(relay[i], HIGH);
    delay(25);
  }
  isRelayInitialized = true;
}

bool isNowAfter(byte start_h, byte start_m)
{
  int minutes_now = hour() * 60 + minute();
  int minutes_start = start_h * 60 + start_m;
  return minutes_start <= minutes_now;
}

int floatToRound05Int(float temp)
{
  int t = (int) (temp * 100);
  uint8_t mod = t % 10;
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

void pritSerial()
{
#if DEBUG
  Serial.print(LsuNtpTime::datetimeString(datetimebuff));
  Serial.print(" - [");
  Serial.print(T_LOC);
  Serial.print("] ");
  for (uint8_t i = 0; i < SENZOR_COUNT; ++i)
  {
    if (i)
      Serial.print(", ");
    Serial.print((1.0 * temperature[i]) / 100);
  }
  Serial.println(" [grd.C]");
#endif
}

void startConversion(uint8_t i)
{
  dallasTemperature1st_pin0.requestTemperaturesByAddress(
      SENZOR_ADDRESS[i + offset]);
}

void startConversion_1()
{
  startConversion(0);
}

void startConversion_2()
{
  startConversion(1);
}

void startConversion_3()
{
  startConversion(2);
}

void startConversion_4()
{
  startConversion(3);
}

void sendPostData(const char* data, const char* page,
    const IPAddress & server, const int port)
{
  const char* post_data_fmt = "t_loc=%s&%s";
  const size_t post_data_len = strlen(post_data_fmt) - 4 + strlen(T_LOC) + strlen(data);
  char post_data[post_data_len + 1];
  sprintf(post_data, post_data_fmt, T_LOC, data);
//  String post_data = String(T_LOC_NAME) + "=" + T_LOC + "&" + data;

  const char* post_req_fmt =
      "POST %s HTTP/1.1\r\n"
      "Host: %s\r\n"
      "User-Agent: Arduino/1.0\r\n"
      "Connection: close\r\n"
      "Content-Type: application/x-www-form-urlencoded\r\n"
      "Content-Length: %d\r\n";
  const size_t post_req_len = strlen(post_req_fmt) - 6 + strlen(page) + 15 + 4;
  char post_req[post_req_len];
  char addr[IP_ADDR_STR_LEN + 1];
  sprintf(post_req, post_req_fmt, page, LsuWiFi::ipAddressStr(addr), post_data_len);

//  String post_req = String("") + "POST " + page + " HTTP/1.1\r\n" + "Host: "
//      + WiFi.localIP().toString() + "\r\n" + "User-Agent: Arduino/1.0\r\n"
//      + "Connection: close\r\n"
//      + "Content-Type: application/x-www-form-urlencoded\r\n"
//      + "Content-Length: " + post_data_len + "\r\n";
  WiFiClient client;
  if (client.connect(server, port))
  {
    client.println(post_req);
    client.println(post_data);
    delay(10);
    client.stop();
  }
  else
  {
//    writeLogger(
//        String("[") + T_LOC + "]  ERROR connecting server: " + IPAddress(server).toString());
  }
}

void sendCurrentTemperatures()
{
  const char* msg_fmt = "t_%d=%d&t_%d_r=%d&t_%d=%d&t_%d_r=%d&t_%d=%d&t_%d_r=%d&t_%d=%d&t_%d_r=%d";
  size_t msg_len = strlen(msg_fmt) - 32 + 24 + 4;
  char msg[msg_len + 1];
  short run_flag = is_running ? 1 : 0;
  sprintf(msg, msg_fmt
      ,0 ,temperature[0], 0, run_flag
      ,1 ,temperature[1], 1, run_flag
      ,2 ,temperature[2], 2, run_flag
      ,3 ,temperature[3], 3, run_flag);
  sendPostData(const_cast<const char*>(msg), page, master_server_ip, master_server_port);
}

void checkProgramming()
{
  uint8_t i = 0; // bucatarie
  bool should_run = !has_p2_run_today
      && isNowAfter(start_hour_p2, start_minute_p2);
  if (should_run && !target_temperature_p2)
  {
    target_temperature_p2 = temperature[i] + DELTA_TEMP;
  }
  should_run = should_run && temperature[i] <= target_temperature_p2;
  if (should_run != is_running)
  {
    // start / stop all
    for (uint8_t j = 0; j < SENZOR_COUNT; ++j)
    {
      digitalWrite(relay[j], should_run ? LOW : HIGH);
    }
  }
  if (!should_run && is_running)
  {
    has_p2_run_today = true;
    target_temperature_p2 = 0;
  }
  is_running = should_run;
  pritSerial();
}

void updateTemperature();

void scheduleAt(long int next_read)
{
  // schedule conversion at'next_read' time, minus the time to wait for it
  LsuScheduler::add(startConversion_1, next_read - 4 * CONVERSION_TIME);
  LsuScheduler::add(startConversion_3, next_read - 3 * CONVERSION_TIME);
  LsuScheduler::add(startConversion_2, next_read - 2 * CONVERSION_TIME);
  LsuScheduler::add(startConversion_4, next_read - 1 * CONVERSION_TIME);

  // schedule read at'next_read' time
  LsuScheduler::add(updateTemperature, next_read);
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
  for (uint8_t i = 0, j = i + offset; i < SENZOR_COUNT / 2; ++i, ++j)
  {
#if (DEBUG > 1)
//    Serial.println(String("read temperature i=") + (short) i + " j=" + (short) j);
#endif
    temp = floatToRound05Int(
        dallasTemperature1st_pin0.getTempC(SENZOR_ADDRESS[j]));
    if (temp <= -4000 || temp >= 7000)
    {
#if DEBUG
      Serial.print("ERROR: bad temperature [");
      Serial.print((short) i);
      Serial.print("]: ");
      Serial.println(temp);
#endif
      temp = 0;
    }
    if (temp)
      temperature[i] = temp;
  }
  for (uint8_t i = SENZOR_COUNT / 2, j = i + offset; i < SENZOR_COUNT; ++i, ++j)
  {
#if (DEBUG > 1)
//    Serial.println(String("read temperature i=") + (short) i + " j=" + (short) j);
#endif
    temp = floatToRound05Int(
        dallasTemperature2nd_pin2.getTempC(SENZOR_ADDRESS[j]));
    if (temp <= -4000 || temp >= 7000)
    {
#if DEBUG
      Serial.print("ERROR: bad temperature [");
      Serial.print((short) i);
      Serial.print("]: ");
      Serial.println(temp);
#endif
      temp = 0;
    }
    if (temp)
      temperature[i] = temp;
  }

  scheduleAt(next_read);
  // schedule program checking after 10 ms
  LsuScheduler::add(checkProgramming, 10);
}

} // anonymous namespace

void setup()
{
#if DEBUG
  Serial.begin(460800);
  Serial.println();
#endif
  for (uint8_t idx = 0; idx < SSID_SIZE; ++idx)
  {
    LsuWiFi::addAp(SSIDs[idx], SSID_PASSWDs[idx]);
  }
  LsuWiFi::connect();
  LsuNtpTime::begin();
#if DEBUG
  Serial.printf("Got NTP time: %s\n", LsuNtpTime::datetimeString(datetimebuff));
#endif

  char mac[MAC_ADDR_STR_LEN + 1];
  LsuWiFi::macAddressStr(mac);
  if (strcmp(mac, MAC_ETAJ) == 0)
  {
    offset = 0;
    strcpy(T_LOC, T_LOC_ETAJ);
  }
  else if (strcmp(mac, MAC_PARTER) == 0)
  {
    offset = SENZOR_COUNT;
    strcpy(T_LOC, T_LOC_PARTER);
  }
  else
    while (true)
    {
#if DEBUG
      Serial.println(String("Unknown MAC address: ") + mac);
#endif
      delay(20 * 60 * SECOND);
    }

  // update has run today
  int minutes_now = hour() * 60 + minute();
  int minutes_start = start_hour_p2 * 60 + start_minute_p2;
  has_p2_run_today = minutes_start - minutes_now > 60;
#if DEBUG
      Serial.printf("Mark has_tun_today: %s\n", (has_p2_run_today ? "true" : "false"));
#endif


  // Start up the temperature library
  dallasTemperature1st_pin0.begin();
  dallasTemperature1st_pin0.setResolution(RESOLUTION);
  dallasTemperature1st_pin0.setWaitForConversion(false);

  dallasTemperature2nd_pin2.begin();
  dallasTemperature2nd_pin2.setResolution(RESOLUTION);
  dallasTemperature2nd_pin2.setWaitForConversion(false);

  int first_read = millis() + 8 * CONVERSION_TIME;
  scheduleAt(first_read);
//  LsuScheduler::add(relayInitialize, millis() + 5);
}

void loop()
{
  if (!isRelayInitialized)
    relayInitialize();
  LsuScheduler::execute(millis());
}

#endif /* _LsuHeating_H_ */
