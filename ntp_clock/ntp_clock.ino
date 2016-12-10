#include <Time.h>         // https://github.com/PaulStoffregen/Time
#include <Timezone.h>     // https://github.com/JChristensen/Timezone
#include <WiFiUdp.h>      // http://playground.arduino.cc/Code/NTPclient
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino

#include <Wire.h> 
#include <LiquidCrystal_I2C.h> // https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/NewliquidCrystal_1.3.4.zip

// one second, 1000 milliseconds
const int SECOND = 1000;

// WiFi section
const byte SSID_SIZE = 2;
const char* SSID_t[SSID_SIZE]   = {"cls-router", "cls-ap"};
const char* PASSWD_t[SSID_SIZE] = {"r4cD7TPG", "r4cD7TPG"};

byte ssid_ix = 0;

WiFiServer server(80);
// WiFi section - end

// NTP section
// Eastern European Time (Timisoara)
const TimeChangeRule EEST = {"EEST", Last, Sun, Mar, 3, 180}; // Eastern European Summer Time
const TimeChangeRule EET  = {"EET",  Last, Sun, Oct, 4, 120}; // Eastern European Standard Time
Timezone EasternEuropeanTime(EEST, EET);

TimeChangeRule *tcr;

// Constants waiting for NTP server response; check every POLL_INTERVAL (ms) up to POLL_TIMES times
const byte POLL_INTERVAL = 10; // poll every this many ms
const byte POLL_TIMES = 100;  // poll up to this many times
const byte PKT_LEN = 48; // NTP packet length
const byte USELES_BYTES = 40; // Useless bytes to be discarded; set useless to 32 for speed; set to 40 for accuracy.

WiFiUDP udp;
// NTP section - end

// LiquidCrystal section
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
// LiquidCrystal section - end

time_t getTime();

void connectWifi();

unsigned long secondsRunning();

String timeString();

String timeString(int, int, int, int, int, int);

void setup() {
  Serial.begin(115200);

  // initialize the lcd
  lcd.begin(20, 4);
  lcd.setBacklight(HIGH);

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

}

void loop() {
  Serial.println(timeString());
//  lcd.home();
  lcd.clear();
  lcd.print(timeString());
  delay(SECOND);
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
  if (! (udp.beginPacket(timeServer, 123)
         && udp.write((byte *)&ntpFirstFourBytes, PKT_LEN) == PKT_LEN
         && udp.endPacket()))
  {
    Serial.println("NTP ERROR: sending request failed");
    return 0; // sending request failed
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

  String msg = String("NTP: primit ora exacta (") + (millis() - mllis) + " ms)";
  lcd.clear();
  lcd.print(msg);
  return t_time;
}

void connectWifi()
{
  if (WiFi.status() == WL_CONNECTED )
    return;
  ssid_ix = ssid_ix % SSID_SIZE;
  String msg = String("WiFi: conectare la ") + SSID_t[ssid_ix];
  lcd.clear();
  lcd.print(msg);

  Serial.println(msg);
  WiFi.begin(SSID_t[ssid_ix], PASSWD_t[ssid_ix]);

  unsigned long mllis = millis();
  byte count = 1;
  while (WiFi.status() != WL_CONNECTED) {
    delay(10);
    if (millis() - mllis >= 10000)
    {
      Serial.println(" - 10 s timed out. Trying next SSID.");
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
  msg = String("WiFi: conectat la ") + SSID_t[ssid_ix];
  lcd.clear();
  lcd.print(msg);
}

unsigned long secondsRunning()
{
  return millis() / SECOND;
}

String timeString()
{
  int day_t = day(), month_t = month(), year_t = year();
  int hour_t = hour(), minute_t = minute(), second_t = second();
  return timeString(day_t, month_t, year_t, hour_t, minute_t, second_t);
}

String timeString(int day_t, int month_t, int year_t, int hour_t, int minute_t, int second_t)
{
  char buff[20];
  sprintf(buff, "%02d-%02d-%04d %02d:%02d:%02d", day_t, month_t, year_t, hour_t, minute_t, second_t);
  return String(buff);
//  String timeStr;
//  if (day_t < 10) timeStr += "0";
//  timeStr += day_t;
//  timeStr += "-";
//  if (month_t < 10) timeStr += "0";
//  timeStr += month_t;
//  timeStr += "-";
//  timeStr += year_t;
//  timeStr += " ";
//  if (hour_t < 10) timeStr += "0";
//  timeStr += hour_t;
//  timeStr += ":";
//  if (minute_t < 10) timeStr += "0";
//  timeStr += minute_t;
//  timeStr += ":";
//  if (second_t < 10) timeStr += "0";
//  timeStr += second_t;
//  return timeStr;
}

