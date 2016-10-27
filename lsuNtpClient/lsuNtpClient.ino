#include <time.h>
#include <WiFiUdp.h>      // http://playground.arduino.cc/Code/NTPclient
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <Time.h>         // https://github.com/PaulStoffregen/Time
#include <Timezone.h>     // https://github.com/JChristensen/Timezone
#include <LsuScheduler.h>


// one second, 1000 milliseconds
const int SECOND = 1000;

// Task scheduler
LsuScheduler scheduler;

// Eastern European Time (Timisoara)
// - Summer Time starts in last Sunday of March at 3 AM and is UTC + 3 hours (180 minutes)
const TimeChangeRule EEST = {"EEST", Last, Sun, Mar, 3, 180};
// - Standard (winter) Time starts in last Sunday of Octomber at 4 AM and is UTC + 2 hours (120 minutes)
const TimeChangeRule EET  = {"EET",  Last, Sun, Oct, 4, 120};
// The Timezone object
Timezone EasternEuropeanTime(EEST, EET);
TimeChangeRule *tcr;

// WiFi operation
const byte SSID_SIZE = 2;
const char* SSID_t[SSID_SIZE]   = {"cls-router", "cls-ap"};
const char* PASSWD_t[SSID_SIZE] = {"r4cD7TPG", "r4cD7TPG"};

byte ssid_ix = 0;

// UDP operation
//static WiFiUDP udp;
const char timeServer[] = "ro.pool.ntp.org";  // the NTP server
const long ntpFirstFourBytes = 0xEC0600E3; // the NTP request header
// Constants waiting for NTP server response; check every POLL_INTERVAL (ms) up to POLL_TIMES times
const byte POLL_INTERVAL = 10; // poll every this many ms
const byte POLL_TIMES = 100; // poll up to this many times
// NTP packet length
const byte PKT_LEN = 48;
// Useless bytes to be discarded; set useless to 32 for speed; set to 40 for accuracy.
const byte USELES_BYTES = 40;
// Next NTP request interval
const int NEXT_NTP_REQUEST = 30 * SECOND;
// read counts
byte pollCount;

// funtions prorotypes
void connectWifi();

void updateTime(time_t);

void sendNtpRequest();

void readNtpResponse();

void timeDisplay();

void setup() {
  // Starts Serial communication
  Serial.begin(115200);
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  scheduler.add(sendNtpRequest, SECOND);
}

void connectWifi()
{
  if (WiFi.status() == WL_CONNECTED )
    return;
  Serial.print("Connecting to ");
  Serial.print(SSID_t[(ssid_ix % SSID_SIZE)]);
  ssid_ix = ssid_ix % SSID_SIZE;
  WiFi.begin(SSID_t[ssid_ix], PASSWD_t[ssid_ix]);

  unsigned long mllis = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(10);
    if(millis() - mllis >= 10000)
    {
      Serial.println(" 10 s timed out. Trying next SSID.");
      ssid_ix += 1;
      return connectWifi();
    }
  }
  Serial.print(" done ("); Serial.print((millis() - mllis)); Serial.println(" ms)."); 
}

void sendNtpRequest()
{
  pollCount = 0;
  if (WiFi.status() != WL_CONNECTED)
    connectWifi();
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("ERROR: WiFi connection failed.");
    // reschedule after one second
    scheduler.add(sendNtpRequest, SECOND);
    return;
  }

  static WiFiUDP udp;
  // Initializes the WiFi UDP library and starts WiFiUDP socket; listening at local 12670
  static int udpInitialized = udp.begin(12670);
  if (0 == udpInitialized) // returns 0 if there are no sockets available to use
  {
    Serial.println("ERROR: there are no sockets available to use.");
    // reschedule after one second
    scheduler.add(sendNtpRequest, SECOND);
    return;
  }
  // Clear received data from possible stray received packets
  udp.flush();
  // Send an NTP request to timeserver on NTP port: 123
  if (! (udp.beginPacket(timeServer, 123)
         && udp.write((byte *)&ntpFirstFourBytes, PKT_LEN) == PKT_LEN
         && udp.endPacket()))
  {
    // sending request failedone second
    Serial.println("ERROR: sending request failed");
    // reschedule after one second
    scheduler.add(sendNtpRequest, SECOND);
    return;
  }
  int pktLen;               // received packet length
  // Wait for NTP server response; check every POLL_INTERVAL ms up to POLL_TIMES times
  byte j = 0;
  for (; j < POLL_TIMES; j++) {
    if ((pktLen = udp.parsePacket()) == PKT_LEN)
      break;
    delay(POLL_INTERVAL);
  }
  if (pktLen != PKT_LEN)
  {
    Serial.print("ERROR: no correct packet received; pktLen = ");
    Serial.print(pktLen);
    Serial.println(", expected 48");
    // reschedule after one second
    scheduler.add(sendNtpRequest, SECOND);
    return;
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
  // for an assumed network delay of 50ms, and (0.5-0.05)*256=115;
  // additionally, we account for how much we delayed reading the packet
  // since its arrival, which we assume on average to be POLL_INTERVAL/2.
  t_time += (udp.read() > 115 - POLL_INTERVAL / 8);

  // Discard the rest of the packet
  udp.flush();
//  WiFi.disconnect();

  // convert NTP time to GMT time
  t_time -= 2208988800ul;
  Serial.print("Got time: ");
  Serial.print(t_time); Serial.print(" (Unix:GMT); ");
  // convert Unix to locale (EET | EEST)
  t_time = EasternEuropeanTime.toLocal(t_time, &tcr);
  Serial.print(t_time); Serial.print(" (Unix:"); Serial.print(tcr -> abbrev); Serial.println(")");

 // updateTime(t_time);

  scheduler.add(sendNtpRequest, NEXT_NTP_REQUEST);
}

void readNtpResponse()
{
  static WiFiUDP udp;
  // increase pollCount
  ++pollCount;
  // received packet length
  int pktLen;
  // Wait for NTP server response; check every POLL_INTERVAL ms up to POLL_TIMES times
  if ((pktLen = udp.parsePacket()) != PKT_LEN)
  {
    // not a valid response, check poll times
    if(pollCount <= POLL_TIMES)
      // schedule a new read after poll interval
      scheduler.add(readNtpResponse, POLL_INTERVAL);
      return;
  }
  
  // Valid response, process it
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
  // for an assumed network delay of 50ms, and (0.5-0.05)*256=115;
  // additionally, we account for how much we delayed reading the packet
  // since its arrival, which we assume on average to be POLL_INTERVAL/2.
  t_time += (udp.read() > 115 - POLL_INTERVAL / 8);

  // Discard the rest of the packet
  udp.flush();

  // convert NTP time to GMT time
  t_time -= 2208988800ul;
  // convert Unix to locale (EET | EEST)
  t_time = EasternEuropeanTime.toLocal(t_time, &tcr);

//  updateTime(t_time);
}

void updateTime(time_t t_time)
{
  setTime(t_time);
  timeDisplay();
}

void timeDisplay()
{
  int day_t = day(), month_t = month(), year_t = year();
  int hour_t = hour(), minute_t = minute(), second_t = second();
  String timeStr;
  if (day_t < 10) timeStr += "0";
  timeStr += day_t;
  timeStr += "-";
  if (month_t < 10) timeStr += "0";
  timeStr += month_t;
  timeStr += "-";
  timeStr += year_t;
  timeStr += " ";
  timeStr += hour_t;
  timeStr += ":";
  if (minute_t < 10) timeStr += "0";
  timeStr += minute_t;
  timeStr += ":";
  if (second_t < 10) timeStr += "0";
  timeStr += second_t;
  Serial.println(timeStr);
}

