#ifndef _LsuHeating_H_
#define _LsuHeating_H_

#include <Arduino.h>

#include <WiFiUdp.h>

#include <OneWire.h>           // https://github.com/PaulStoffregen/OneWire
#include <DallasTemperature.h> // https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <PubSubClient.h>

#include <LsuWiFi.h>
#include <LsuNtpTime.h>
#include <LsuScheduler.h>

#define OTA 0
#ifndef OTA
#define OTA 0
#endif

#if OTA
#include <LsuOta.h>
#endif

#define MQTT 1
#ifndef MQTT
#define MQTT 0
#endif

#define DEBUG 0
#ifndef DEBUG
#define DEBUG 0
#endif

namespace
{
#if OTA
// version
const uint8_t version = 1;
#endif

// floor section
// parter
const char* T_LOC_PARTER = "parter";
//const char* MAC_PARTER = "18:FE:34:D4:0D:EC"; // old
//const char* MAC_PARTER = "5C:CF:7F:EF:BE:50"; - automatizare_2
const char* MAC_PARTER = "5C:CF:7F:EF:B4:0B";

// etaj
const char* T_LOC_ETAJ = "etaj";
const char* MAC_ETAJ = "5C:CF:7F:88:EE:49";

char t_loc[7] = "-";

const size_t room_name_max_len = 11;
const char* room_name[] =
{ "Bucatarie", "Living", "Birou", "Baie parter", };
// end floor section

// WiFi section
const uint8_t SSID_SIZE = 3;
const char* SSIDs[SSID_SIZE] =
{ "cls-router", "cls-ap", "lsu-tpr" };
const char* SSID_PASSWDs[SSID_SIZE] =
{ "r4cD7TPG", "r4cD7TPG", "r4cD7TPG" };

const IPAddress master_server_ip(192, 168, 100, 100);

const int master_server_port = 8081;
const char* update_page = "/update.php";
const char* log_page = "/log_save.php";
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
const uint8_t TEMP_READ_INTERVAL = 30;
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

int16_t temperature[SENZOR_COUNT];

DallasTemperature* dallasTemperature1st_pin0;
DallasTemperature* dallasTemperature2nd_pin2;

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
// program 2
uint8_t start_hour_p2 = 3;
uint8_t start_minute_p2 = 30;
int target_temperature_p2 = 0;
bool has_p2_run_today = false;
bool is_running_p2 = false;

// program 3
uint8_t start_hour_p3 = 17;
uint8_t start_minute_p3 = 0;
int target_temperature_p3 = 0;
bool has_p3_run_today = false;
bool is_running_p3 = false;

bool lateStarted = false;

#if MQTT
const char* ROOMS[] =
{ "dl", "dm", "do", "be", "bu", "li", "bi", "bp" };
const char* ROOMS_R[] =
{ "dlr", "dmr", "dor", "ber", "bur", "lir", "bir", "bpr" };
// heating/etaj | heating/parter
const char* publish_topic_fmt = "heating/%s";
// {"bu":20.00,"li":21.00,"bi":22.00,"bp":23.00,"r":["bur","lir","bir","bpr"]}
const char* publish_message_fmt = "{"
    "\"%s\":%d.%02d,"
    "\"%s\":%d.%02d,"
    "\"%s\":%d.%02d,"
    "\"%s\":%d.%02d,"
    "\"r\":[\"%s\",\"%s\",\"%s\",\"%s\"]}";
const char* mqtt_broker = "192.168.100.60";
uint16_t mqtt_port = 1883;
void callback(char* topic, byte* payload, unsigned int length)
{
}
WiFiClient wifiClient;
PubSubClient mqttClient(mqtt_broker, mqtt_port, callback, wifiClient);
bool mqttConnect()
{
  uint8_t count = 5;
  // Loop at most 5 times or until we're reconnected
  while (!mqttClient.connected() && (count--))
  {
    // Attempt to connect and subscribe
    if (mqttClient.connect(t_loc))
      return true;
    delay(1000);
  }
#if DEBUG
  Serial.print("MQTT connected: ");
  Serial.println(mqttClient.connected() ? "true" : "false");
#endif
  return mqttClient.connected();
}
#endif // MQTT

void scheduleAt(unsigned long);

void lateSetup()
{
  if (lateStarted)
    return;
  dallasTemperature1st_pin0 = new DallasTemperature(new OneWire(GPIO_0));
  dallasTemperature2nd_pin2 = new DallasTemperature(new OneWire(GPIO_2));
  // Start up the temperature library
  dallasTemperature1st_pin0->begin();
  dallasTemperature1st_pin0->setResolution(RESOLUTION);
  dallasTemperature1st_pin0->setWaitForConversion(false);

  dallasTemperature2nd_pin2->begin();
  dallasTemperature2nd_pin2->setResolution(RESOLUTION);
  dallasTemperature2nd_pin2->setWaitForConversion(false);
#if DEBUG
  Serial.println("Temperature sensors initialized");
#endif
  for (uint8_t i = 0; i < SENZOR_COUNT; ++i)
  {
    temperature[i] = 0;
    pinMode(relay[i], OUTPUT);
    digitalWrite(relay[i], HIGH);
    delay(25);
  }
#if DEBUG
  Serial.println("Relays initialized");
#endif
  lateStarted = true;
  scheduleAt(millis() + 8 * CONVERSION_TIME);
#if DEBUG
  Serial.println("Late start done");
#endif
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
  Serial.print(t_loc);
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

void startConversion_1()
{
  dallasTemperature1st_pin0->requestTemperaturesByAddress(
      SENZOR_ADDRESS[0 + offset]);
}

void startConversion_2()
{
  dallasTemperature1st_pin0->requestTemperaturesByAddress(
      SENZOR_ADDRESS[1 + offset]);
}

void startConversion_3()
{
  dallasTemperature2nd_pin2->requestTemperaturesByAddress(
      SENZOR_ADDRESS[2 + offset]);
}

void startConversion_4()
{
  dallasTemperature2nd_pin2->requestTemperaturesByAddress(
      SENZOR_ADDRESS[3 + offset]);
}

void sendPostData(const char* data, const char* page)
{
  const char* post_data_fmt = "t_loc=%s&%s";
  const size_t post_data_len = strlen(post_data_fmt) - 4 + strlen(t_loc)
      + strlen(data);
  char post_data[post_data_len + 1];
  sprintf(post_data, post_data_fmt, t_loc, data);

  const char* post_req_fmt = "POST %s HTTP/1.1\r\n"
      "Host: %s\r\n"
      "User-Agent: Arduino/1.0\r\n"
      "Connection: close\r\n"
      "Content-Type: application/x-www-form-urlencoded\r\n"
      "Content-Length: %d\r\n";
  const size_t post_req_len = strlen(post_req_fmt) - 6 + strlen(page)
      + IP_ADDR_STR_LEN + 3;
  char post_req[post_req_len + 1];
  char addr[IP_ADDR_STR_LEN + 1];
  sprintf(post_req, post_req_fmt, page, LsuWiFi::ipAddressStr(addr),
      post_data_len);

#if (DEBUG > 1)
  Serial.print("Send header: ");Serial.println(post_req);
  Serial.print("Send data  : ");Serial.println(post_data);
#endif
  WiFiClient client;
  if (client.connect(master_server_ip, master_server_port))
  {
    client.println(post_req);
    client.println(post_data);
    delay(10);
    client.stop();
  }
#if DEBUG
  else
  {
    char addr_str[IP_ADDR_STR_LEN + 1];
    LsuWiFi::ipAddressStr(master_server_ip, addr_str);
  }
#endif
}

/**
 * Adds date and time, and location to given message and sends it to log_save page.
 */
void writeLogger(const char* what)
{
  char* datetime = LsuNtpTime::datetimeString(datetimebuff);
  const char* post_data_fmt = "t_log=%s - [%s] %s";
  size_t post_data_len = strlen(post_data_fmt) - 6 + strlen(datetime)
      + strlen(t_loc) + strlen(what);
  char post_data[post_data_len + 1];
  sprintf(post_data, post_data_fmt, datetime, t_loc, what);
  sendPostData(post_data, log_page);
}

void sendCurrentTemperatures()
{
  const char* msg_fmt =
      "t_%d=%d&t_%d_r=%d&t_%d=%d&t_%d_r=%d&t_%d=%d&t_%d_r=%d&t_%d=%d&t_%d_r=%d";
  size_t msg_len = strlen(msg_fmt) - 32 + 24 + 4;
  char msg[msg_len + 1];
  short run_flag = is_running_p2 || is_running_p3 ? 1 : 0;
  sprintf(msg, msg_fmt, 0, temperature[0], 0, run_flag, 1, temperature[1], 1,
      run_flag, 2, temperature[2], 2, run_flag, 3, temperature[3], 3, run_flag);
  sendPostData(const_cast<const char*>(msg), update_page);
}

void checkProgramming()
{
  uint8_t target_room = 3; // baie
  // p2
  bool should_run_p2 = !has_p2_run_today
      && isNowAfter(start_hour_p2, start_minute_p2);
  if (should_run_p2 && !target_temperature_p2)
  {
    target_temperature_p2 = temperature[target_room] + DELTA_TEMP;
  }
  should_run_p2 = should_run_p2
      && temperature[target_room] <= target_temperature_p2;
  // p3
  bool should_run_p3 = !has_p3_run_today
      && isNowAfter(start_hour_p3, start_minute_p3);
  if (should_run_p3 && !target_temperature_p3)
  {
    target_temperature_p3 = temperature[target_room] + DELTA_TEMP;
  }
  should_run_p3 = should_run_p3
      && temperature[target_room] <= target_temperature_p3;
  if (should_run_p2 != is_running_p2 || should_run_p3 != is_running_p3)
  {
    for (uint8_t i = 0; i < SENZOR_COUNT; ++i)
    {
      if (should_run_p2 || should_run_p3)
        digitalWrite(relay[i], LOW);
      else
        digitalWrite(relay[i], HIGH);
    }
  }
  if (should_run_p2 != is_running_p2)
  {
    // start / stop all
    for (uint8_t i = 0; i < SENZOR_COUNT; ++i)
    {
      short t_d = temperature[i] % 100;
      short t_i = (temperature[i] - t_d) / 100;
      if (should_run_p2)
      {
        const char* msg_fmt =
            (i == target_room) ?
                "%s [%d.%02d] - start program P2 - porneste la %d:%02d si face %d.%02d" :
                "%s [%d.%02d] - start fortat program P2";
        const size_t msg_len = strlen(msg_fmt) + room_name_max_len - 6;/* - 20 + 14 = - 6 */
        char msg[msg_len + 1];
        digitalWrite(relay[i], LOW);
        if (i == target_room)
        {
          short tt_d = target_temperature_p2 % 100;
          short tt_i = (target_temperature_p2 - tt_d) / 100;
          sprintf(msg, msg_fmt, room_name[i], t_i, t_d, start_hour_p2,
              start_minute_p2, tt_i, tt_d);
        }
        else
          sprintf(msg, msg_fmt, room_name[i], t_i, t_d, start_hour_p2,
              start_minute_p2);
        writeLogger(msg);
#if DEBUG
        Serial.println(msg);
#endif
      }
      else
      {
        digitalWrite(relay[i], HIGH);
        const char* msg_fmt = "%s [%d.%02d] - stop program P2";
        const size_t msg_len = strlen(msg_fmt) + strlen(room_name[i]) - 4;/* - 14 + 10 = - 4 */
        char msg[msg_len + 1];
        sprintf(msg, msg_fmt, room_name[i], t_i, t_d);
        writeLogger(msg);
#if DEBUG
        Serial.println(msg);
#endif
      }
      delay(50);
    }
  }
  if (should_run_p3 != is_running_p3)
  {
    // start / stop all
    for (uint8_t i = 0; i < SENZOR_COUNT; ++i)
    {
      short t_d = temperature[i] % 100;
      short t_i = (temperature[i] - t_d) / 100;
      if (should_run_p3)
      {
        const char* msg_fmt =
            (i == target_room) ?
                "%s [%d.%02d] - start program P3 - porneste la %d:%02d si face %d.%02d" :
                "%s [%d.%02d] - start fortat program P3";
        const size_t msg_len = strlen(msg_fmt) + room_name_max_len - 6;/* - 20 + 14 = - 6 */
        char msg[msg_len + 1];
        digitalWrite(relay[i], LOW);
        if (i == target_room)
        {
          short tt_d = target_temperature_p3 % 100;
          short tt_i = (target_temperature_p3 - tt_d) / 100;
          sprintf(msg, msg_fmt, room_name[i], t_i, t_d, start_hour_p3,
              start_minute_p3, tt_i, tt_d);
        }
        else
          sprintf(msg, msg_fmt, room_name[i], t_i, t_d, start_hour_p3,
              start_minute_p3);
        writeLogger(msg);
#if DEBUG
        Serial.println(msg);
#endif
      }
      else
      {
        digitalWrite(relay[i], HIGH);
        const char* msg_fmt = "%s [%d.%02d] - stop program P3";
        const size_t msg_len = strlen(msg_fmt) + strlen(room_name[i]) - 4;/* - 14 + 10 = - 4 */
        char msg[msg_len + 1];
        sprintf(msg, msg_fmt, room_name[i], t_i, t_d);
        writeLogger(msg);
#if DEBUG
        Serial.println(msg);
#endif
      }
      delay(50);
    }
  }
  if (!should_run_p2 && is_running_p2)
  {
    has_p2_run_today = true;
    target_temperature_p2 = 0;
  }
  if (!should_run_p3 && is_running_p3)
  {
    has_p3_run_today = true;
    target_temperature_p3 = 0;
  }
  is_running_p2 = should_run_p2;
  is_running_p3 = should_run_p3;
  sendCurrentTemperatures();
#if DEBUG
  Serial.println("MQTT send ... ");
#endif
#if MQTT
#if DEBUG
  Serial.println("MQTT starting ");
#endif
 LsuWiFi::connect();
  if (mqttConnect())
  {
    char publish_topic[strlen(publish_topic_fmt) + strlen(t_loc) - 1];
    sprintf(publish_topic, publish_topic_fmt, t_loc);
    char publish_message[strlen(publish_message_fmt) + 1];
    uint8_t i = 0;
    short t0_d = temperature[i] % 100, t0_i = (temperature[i] - t0_d) / 100;
    short t1_d = temperature[i + 1] % 100, t1_i = (temperature[i + 1] - t1_d) / 100;
    short t2_d = temperature[i + 2] % 100, t2_i = (temperature[i + 2] - t2_d) / 100;
    short t3_d = temperature[i + 3] % 100, t3_i = (temperature[i + 3] - t3_d) / 100;
    bool runs = is_running_p2 || is_running_p3;
    i = offset;
    sprintf(publish_message, publish_message_fmt,
        ROOMS[i], t0_i, t0_d,
        ROOMS[(i + 1)], t1_i, t1_d,
        ROOMS[(i + 2)], t2_i, t2_d,
        ROOMS[(i + 3)], t3_i, t3_d,
        runs ? ROOMS_R[i] : "",
        runs ? ROOMS_R[(i + 1)] : "",
        runs ? ROOMS_R[(i + 2)] : "",
        runs ? ROOMS_R[(i + 3)] : "");
#if DEBUG
  Serial.print("MQTT publish: ");
  Serial.println(publish_message);
#endif
    mqttClient.publish(publish_topic, publish_message);
    delay(50);
    mqttClient.loop();
    delay(50);
    mqttClient.disconnect();
  }
#endif // MQTT
#if DEBUG
  Serial.println("MQTT done. ");
#endif
  pritSerial();
}

void updateTemperature();

void scheduleAt(unsigned long next_read)
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
    temp = floatToRound05Int(
        dallasTemperature1st_pin0->getTempC(SENZOR_ADDRESS[j]));
#if (DEBUG > 1)
    Serial.print("read temperature [");
    Serial.print((short) j);
    Serial.print("]");
    Serial.println(temp);
#endif
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
    temp = floatToRound05Int(
        dallasTemperature2nd_pin2->getTempC(SENZOR_ADDRESS[j]));
#if (DEBUG > 1)
    Serial.print("read temperature [");
    Serial.print((short) j);
    Serial.print("]");
    Serial.println(temp);
#endif
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
//  LsuScheduler::add(checkProgramming, 10);
  checkProgramming();
}

void check_new_day()
{
  if (hour() == 0 && (has_p2_run_today || has_p3_run_today))
  {
    has_p2_run_today = false;
    writeLogger("Reset P2 a rulat astazi.");
    has_p3_run_today = false;
    writeLogger("Reset P3 a rulat astazi.");

#if OTA
    const char* ota_msg_fmt = "OTA ruleaza; versiune aplicatie: %d";
    const size_t ota_msg_len = strlen(ota_msg_fmt) + 1; /* -2 + 3 = + 1 */
    char ota_msg[ota_msg_len + 1];
    sprintf(ota_msg, ota_msg_fmt, version);
    writeLogger(ota_msg);

    const char* ota_url_fmt = "OTA - URL versiune urmatoare: %s";
    const size_t ota_url_len = strlen(ota_url_fmt) - 2 + LsuOta::otaUrlLen();
    char ota_url[ota_url_len + 1];
    sprintf(ota_url, ota_url_fmt, LsuOta::otaUrl());
    writeLogger(ota_url);
#endif
  }
}

} // anonymous namespace

void setup()
{
  Serial.begin(115200);
  delay(0);
#if DEBUG
  Serial.println();
#endif

  pinMode(GPIO_0, OUTPUT);
  digitalWrite(GPIO_0, HIGH);
  pinMode(GPIO_2, OUTPUT);
  digitalWrite(GPIO_2, HIGH);

  char mac[MAC_ADDR_STR_LEN + 1];
  LsuWiFi::macAddressStr(mac);
  if (strcmp(mac, MAC_ETAJ) == 0)
  {
    offset = 0;
    strcpy(t_loc, T_LOC_ETAJ);
  }
  else if (strcmp(mac, MAC_PARTER) == 0)
  {
    offset = SENZOR_COUNT;
    strcpy(t_loc, T_LOC_PARTER);
  }
  else
  {
    const char* msg_fmt = "Unknown MAC address: %s";
    const size_t msg_len = strlen(msg_fmt) - 2 + strlen(mac);
    char msg[msg_len + 1];
    sprintf(msg, msg_fmt, mac);
    writeLogger(msg);

    while (true)
    {
#if DEBUG
      Serial.println(msg);
#endif
      delay(20 * 60 * SECOND);
    }
  }

  for (size_t idx = 0; idx < SSID_SIZE; ++idx)
  {
    LsuWiFi::addAp(SSIDs[idx], SSID_PASSWDs[idx]);
  }
  LsuWiFi::connect();
  LsuNtpTime::begin();
#if DEBUG
  Serial.printf("Got NTP time: %s\n", LsuNtpTime::datetimeString(datetimebuff));
#endif

  const char* SSID = LsuWiFi::currentSSID();
  const char* ip_msg_fmt = "WiFi: conectat la %s, adresa IP: %s";
  const size_t ip_msg_len = strlen(ip_msg_fmt) - 4 + strlen(SSID)
      + IP_ADDR_STR_LEN;
  char ip_msg[ip_msg_len + 1];
  char addr_str[IP_ADDR_STR_LEN + 1];
  sprintf(ip_msg, ip_msg_fmt, SSID, LsuWiFi::ipAddressStr(addr_str));
  writeLogger(ip_msg);

#if OTA
  LsuOta::begin((version + 1));
  const char* ota_msg_fmt = "OTA pornit; versiune aplicatie: %d";
  const size_t ota_msg_len = strlen(ota_msg_fmt) + 1; /* -2 + 3 = + 1 */
  char ota_msg[ota_msg_len + 1];
  sprintf(ota_msg, ota_msg_fmt, version);
  writeLogger(ota_msg);
  const char* ota_url_fmt = "OTA - URL versiune urmatoare: %s";
  const size_t ota_url_len = strlen(ota_url_fmt) - 2 + LsuOta::otaUrlLen();
  char ota_url[ota_url_len + 1];
  sprintf(ota_url, ota_url_fmt, LsuOta::otaUrl());
  writeLogger(ota_url);
#endif

#if DEBUG
  Serial.println(ota_msg);
  Serial.printf(ota_url);
#endif

  // update has run today
  int minutes_now = hour() * 60 + minute();
  int minutes_start_p2 = start_hour_p2 * 60 + start_minute_p2;
  has_p2_run_today = minutes_now - minutes_start_p2 > 60;
  int minutes_start_p3 = start_hour_p3 * 60 + start_minute_p3;
  has_p3_run_today = minutes_now - minutes_start_p3 > 60;
#if DEBUG
  Serial.printf("Mark has_p2_run_today: %s, minutes now: %d, minutes start: %d\n",
      (has_p2_run_today ? "true" : "false"), minutes_now, minutes_start_p2);
  Serial.printf("Mark has_p3_run_today: %s, minutes now: %d, minutes start: %d\n",
      (has_p3_run_today ? "true" : "false"), minutes_now, minutes_start_p3);
#endif

#if DEBUG
  Serial.println("Setup done");
#endif
}

void loop()
{
  lateSetup();
  LsuWiFi::connect();
  LsuScheduler::execute(millis());
  check_new_day();
#if OTA
  LsuOta::loop();
#endif
}

#endif /* _LsuHeating_H_ */
