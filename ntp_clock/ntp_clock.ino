#include <Time.h>         // https://github.com/PaulStoffregen/Time
#include <Timezone.h>     // https://github.com/JChristensen/Timezone

#include <WiFiUdp.h>      // arduino/libraries/WiFi/src
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino

#include <LiquidCrystal_I2C.h> // https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/NewliquidCrystal_1.3.4.zip

// one second, 1000 milliseconds
const int SECOND = 1000;

// WiFi section
const byte SSID_SIZE = 2;
const char* SSID_t[SSID_SIZE]   = {"cls-router", "cls-ap"};
const char* PASSWD_t[SSID_SIZE] = {"r4cD7TPG", "r4cD7TPG"};

byte ssid_ix = 0;
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
// Set the LCD I2C address and pinouts
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

const byte LCD_LINES = 2;
const byte LCD_CHARS_PER_LINE = 16;
// LiquidCrystal section - end


// functions prototype section
void connectWifi();

time_t getTime();

void showTimeOnLcd();

String timeString();

String timeString(int, int, int, int, int, int);
// functions prototype section - end


void setup() {
  Serial.begin(115200);

  // Set the external time provider
  setSyncProvider(getTime);
  // Set synch interval to 2 seconds till sync
  setSyncInterval(200);
  // wait to synch clock
  while (timeStatus() != timeSet)
  {
    ;
  }
  // Set synch interval to 6 hours
  setSyncInterval(6 * 3600);

  // initialize the lcd
  lcd.begin(LCD_CHARS_PER_LINE, LCD_LINES);
  lcd.setBacklight(HIGH);
}

void loop() {
  showTimeOnLcd();
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
    Serial.println(String("NTP ERROR: no correct packet received; pktLen = ") + pktLen + ", expected 48");
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

  Serial.println(String("NTP: primit ora exacta (") + (millis() - mllis) + " ms)");
  return t_time;
}

void connectWifi()
{
  if (WiFi.status() == WL_CONNECTED )
    return;
  ssid_ix = ssid_ix % SSID_SIZE;

  Serial.println(String("WiFi: conectare la ") + SSID_t[ssid_ix]);
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
  Serial.println(String("WiFi: conectat la ") + SSID_t[ssid_ix] + ", adresa IP: " + WiFi.localIP().toString());
}

void showTimeOnLcd()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  String timeStr = timeString();
  lcd.print(timeStr);
  Serial.println(timeStr);
}

String timeString()
{
  int day_t = day(), month_t = month(), year_t = year();
  int hour_t = hour(), minute_t = minute(), second_t = second();
  return timeString(day_t, month_t, year_t, hour_t, minute_t, second_t);
}

String timeString(int day_t, int month_t, int year_t, int hour_t, int minute_t, int second_t)
{
  char buff[LCD_CHARS_PER_LINE];
  if(LCD_CHARS_PER_LINE < 20)
    sprintf(buff, "%02d:%02d:%02d", hour_t, minute_t, second_t);
  else
    sprintf(buff, "%02d-%02d-%04d %02d:%02d:%02d", day_t, month_t, year_t, hour_t, minute_t, second_t);
  return String(buff);
}

