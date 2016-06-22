#include <time.h>
#include <WiFiUdp.h>      // http://playground.arduino.cc/Code/NTPclient
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <Time.h>         // https://github.com/PaulStoffregen/Time
#include <Timezone.h>     // https://github.com/JChristensen/Timezone

// Eastern European Time (Timisoara)
const TimeChangeRule EEST = {"EEST", Last, Sun, Mar, 3, 120}; // Eastern European Summer Time
const TimeChangeRule EET  = {"EET",  Last, Sun, Oct, 4,  60}; // Eastern European Standard Time
Timezone EasternEuropeanTime(EEST, EET);

TimeChangeRule *tcr;

const char* SSID_t   = "lsu-phone";
const char* PASSWD_t = "r4cD7TPG";

// Constants waiting for NTP server response; check every POLL_INTERVAL (ms) up to POLL_TIMES times
const int POLL_INTERVAL = 150; // poll every this many ms
const byte POLL_TIMES = 15;  // poll up to this many times
// NTP packet length
const int PKT_LEN = 48;
// Useless bytes to be discarded; set useless to 32 for speed; set to 40 for accuracy.
const byte USELES_BYTES = 40;

time_t getTime();
void timeDisplay();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  // Set the external time provider
  setSyncProvider(getTime);
  // Set synch interval in seconds
  setSyncInterval(20);
}

void loop() {
  if (timeStatus() == timeNotSet)
  {
    Serial.println("Time not synced");
  }
  else
  {
    if (timeStatus() == timeNeedsSync)
      Serial.println("Time need sync");
    timeDisplay();
  }
  delay(5000);
}

void connectWifi()
{
  if (WiFi.status() == WL_CONNECTED )
    return;
  Serial.print("Connecting to ");
  Serial.print(SSID_t);
  Serial.print(" ");

  WiFi.begin(SSID_t, PASSWD_t);

  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    if ((count++) && (count % 10) == 0)
    {
      Serial.print(" ");
      count = 0;
    }
  }
  Serial.println(" done.");
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

  static WiFiUDP udp;
  static int udpInitialized = udp.begin(12670); // Initializes the WiFi UDP library and network settings. Starts WiFiUDP socket, listening at local port 123
  if (0 == udpInitialized) // returns 0 if there are no sockets available to use
  {
    Serial.println("ERROR: there are no sockets available to use.");
    return 0;
  }
  static char timeServer[] = "pool.ntp.org";  // the NTP server
  // static char timeServer[] = "time.nist.gov";  // the NTP server
  static long ntpFirstFourBytes = 0xEC0600E3; // the NTP request header

  udp.flush(); // Clear received data from possible stray received packets

  // Send an NTP request to timeserver on NTP port: 123
  if (! (udp.beginPacket(timeServer, 123)
         && udp.write((byte *)&ntpFirstFourBytes, PKT_LEN) == PKT_LEN
         && udp.endPacket()))
  {
    Serial.println("ERROR: sending request failed");
    return 0; // sending request failed
  }

  int pktLen;               // received packet length
  // Wait for NTP server response; check every POLL_INTERVAL ms up to POLL_TIMES times
  for (byte i = 0; i < POLL_TIMES; i++) {
    if ((pktLen = udp.parsePacket()) == PKT_LEN)
      break;
    delay(POLL_INTERVAL);
  }
  if (pktLen != PKT_LEN)
  {
    Serial.print("ERROR: no correct packet received; pktLen = ");
    Serial.print(pktLen);
    Serial.println(", expected 48");
    return 0; // no correct packet received
  }

  // Read and discard the first useless bytes
  for (byte i = 0; i < USELES_BYTES; ++i)
    udp.read();

  // Read the integer part of sending time
  time_t time = udp.read();  // NTP time
  for (byte i = 1; i < 4; i++)
    time = time << 8 | udp.read();

  // Round to the nearest second if we want accuracy
  // The fractionary part is the next byte divided by 256: if it is
  // greater than 500ms we round to the next second; we also account
  // for an assumed network delay of 50ms, and (0.5-0.05)*256=115;
  // additionally, we account for how much we delayed reading the packet
  // since its arrival, which we assume on average to be POLL_INTERVAL/2.
  time += (udp.read() > 115 - POLL_INTERVAL / 8);

  // Discard the rest of the packet
  udp.flush();
  WiFi.disconnect();

  // convert NTP time to GMT time
  time -= 2208988800ul;
  Serial.print("Got time: ");
  Serial.print(time); Serial.print(" (Unix:GMT); ");
  // convert Unix to locale (EET | EEST)
  time = EasternEuropeanTime.toLocal(time, &tcr);
  Serial.print(time); Serial.print(" (Unix:"); Serial.print(tcr -> abbrev); Serial.println(")");

  return time;
}

void timeDisplay()
{
  int day_t = day(), month_t = month(), year_t = year();
  int hour_t = hour(), minute_t = minute(), second_t = second();
  if (day_t < 10) Serial.print("0");
  Serial.print(day_t);
  Serial.print("-");
  if (month_t < 10) Serial.print("0");
  Serial.print(month_t);
  Serial.print("-");
  Serial.print(year_t);
  Serial.print(" ");
  Serial.print(hour_t);
  Serial.print(":");
  if (minute_t < 10) Serial.print("0");
  Serial.print(minute_t);
  Serial.print(":");
  if (second_t < 10) Serial.print("0");
  Serial.print(second_t);
  Serial.println();
}

