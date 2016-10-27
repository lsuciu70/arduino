#include <time.h>
#include <WiFiUdp.h>      // http://playground.arduino.cc/Code/NTPclient
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <Time.h>         // https://github.com/PaulStoffregen/Time
#include <Timezone.h>     // https://github.com/JChristensen/Timezone

// Eastern European Time (Timisoara)
const TimeChangeRule EEST = {"EEST", Last, Sun, Mar, 3, 180}; // Eastern European Summer Time
const TimeChangeRule EET  = {"EET",  Last, Sun, Oct, 4, 120}; // Eastern European Standard Time
Timezone EasternEuropeanTime(EEST, EET);

TimeChangeRule *tcr;

const byte SSID_SIZE = 2;
const char* SSID_t[]   = {"cls-router", "cls-ap"};
const char* PASSWD_t[] = {"r4cD7TPG", "r4cD7TPG"};

byte ssid_ix = 0;

// Constants waiting for NTP server response; check every POLL_INTERVAL (ms) up to POLL_TIMES times
const byte POLL_INTERVAL = 10; // poll every this many ms
const byte POLL_TIMES = 100;  // poll up to this many times
const byte PKT_LEN = 48; // NTP packet length
const byte USELES_BYTES = 40; // Useless bytes to be discarded; set useless to 32 for speed; set to 40 for accuracy.

time_t getTime();

void timeDisplay();

WiFiUDP udp;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  // Set the external time provider
  setSyncProvider(getTime);
  // Set synch interval in seconds
  setSyncInterval(30);
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
  static int udpInitialized = udp.begin(12670); // Initializes the WiFi UDP library and network settings. Starts WiFiUDP socket, listening at local port 123
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
  if (! (udp.beginPacket(timeServer, 123)
         && udp.write((byte *)&ntpFirstFourBytes, PKT_LEN) == PKT_LEN
         && udp.endPacket()))
  {
    Serial.println("ERROR: sending request failed");
    return 0; // sending request failed
  }
  unsigned long took = millis() - mllis;
  Serial.print("sent ("); Serial.print(took); Serial.println(" ms)."); 
  mllis = millis();

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
    return 0; // no correct packet received
  }

  took = millis() - mllis;
  Serial.print("response ("); Serial.print(took); Serial.print(" ms), "); Serial.print(j); Serial.print(" rounds x "); Serial.print(POLL_INTERVAL);  Serial.println(" ms.");
  mllis = millis();

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

  took = millis() - mllis;
  Serial.print("done ("); Serial.print(took); Serial.println(" ms).");
  return t_time;
}

String timeString()
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
  return timeStr;
}

void timeDisplay()
{
  Serial.println(timeString());
}

