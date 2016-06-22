#include <time.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <Time.h>
#include <Timezone.h>

// Eastern European Time (Timisoara)
TimeChangeRule EEST = {"EEST", Last, Sun, Mar, 3, 120}; // Eastern European Summer Time
TimeChangeRule EET  = {"EET",  Last, Sun, Oct, 4,  60}; // Eastern European Standard Time
Timezone EE(EEST, EET);

TimeChangeRule *tcr;

const char* ssid     = "lsu-phone";
const char* password = "r4cD7TPG";

time_t getTime();
void timeDisplay();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  if(timeStatus() == timeNotSet)
    Serial.println("Time not synched");
  setSyncProvider(getTime);// Set the external time provider
  setSyncInterval(10); 
  //
  //  // We start by connecting to a WiFi network
  //
  //  Serial.println();
  //  Serial.println();
  //  Serial.print("Connecting to ");
  //  Serial.println(ssid);
  //
  //  WiFi.begin(ssid, password);
  //
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(500);
  //    Serial.print(".");
  //  }
  //
  //  Serial.println("");
  //  Serial.println("WiFi connected");
}

void loop() {
//  // put your main code here, to run repeatedly:
//  unsigned long time = getTime();
//  Serial.println(time);
  if (timeStatus() == timeSet)
  {
    timeDisplay();
//    struct tm * timeinfo;
//    time_t rawtime(time);
//    timeinfo = localtime (&rawtime);
//    Serial.println(asctime(timeinfo));
  }
  else 
  {
    Serial.println("Time not synched");
  }
  delay(5000);
}

void connectWifi()
{
  if (WiFi.status() == WL_CONNECTED )
    return;
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Done.");
}

time_t getTime()
{
  if (WiFi.status() != WL_CONNECTED)
    connectWifi();
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Connectin failed.");
    delay(1000);
    return 0;
  }
  // thanks to (http) playground.arduino.cc/Code/NTPclient
  static WiFiUDP udp;
  static int udpInitialized = udp.begin(12399); // Initializes the WiFi UDP library and network settings. Starts WiFiUDP socket, listening at local port 123
  if (0 == udpInitialized) // returns 0 if there are no sockets available to use
  {
    Serial.println("ERROR: there are no sockets available to use");
    return 0;
  }
  static char timeServer[] = "pool.ntp.org";  // the NTP server
  // static char timeServer[] = "time.nist.gov";  // the NTP server
  static long ntpFirstFourBytes = 0xEC0600E3; // the NTP request header

  udp.flush(); // Clear received data from possible stray received packets

  // Send an NTP request to timeserver on NTP port: 123
  if (! (udp.beginPacket(timeServer, 123)
         && udp.write((byte *)&ntpFirstFourBytes, 48) == 48
         && udp.endPacket()))
  {
    Serial.println("ERROR: sending request failed");
    return 0; // sending request failed
  }

  // Wait for response; check every pollIntv ms up to maxPoll times
  const int pollIntv = 150; // poll every this many ms
  const byte maxPoll = 15;  // poll up to this many times
  int pktLen;               // received packet length
  for (byte i = 0; i < maxPoll; i++) {
    if ((pktLen = udp.parsePacket()) == 48)
      break;
    delay(pollIntv);
  }
  if (pktLen != 48)
  {
    Serial.print("ERROR: no correct packet received; pktLen = ");
    Serial.println(pktLen);
    return 0; // no correct packet received
  }

  // Read and discard the first useless bytes
  // Set useless to 32 for speed; set to 40 for accuracy.
  const byte useless = 40;
  for (byte i = 0; i < useless; ++i)
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
  // since its arrival, which we assume on average to be pollIntv/2.
  time += (udp.read() > 115 - pollIntv / 8);

  // Discard the rest of the packet
  udp.flush();

  time -= 2208988800ul;   // convert NTP time to Unix time
  Serial.print("Got Unix time: ");
  Serial.println(time);
//  time_t utc;
  time = EE.toLocal(time, &tcr);
  Serial.print("As local time: ");
  Serial.println(time);
  return time;
}

void timeDisplay()
{
  int day_t = day(), month_t = month(), year_t = year();
  int hour_t = hour(), minute_t = minute(), second_t = second();
  if(day_t < 10) Serial.print("0");
  Serial.print(day_t);
  Serial.print("-");
  if(month_t < 10) Serial.print("0");
  Serial.print(month_t);
  Serial.print("-");
  Serial.print(year_t);
  Serial.print(" ");
  Serial.print(hour_t);
  Serial.print(":");
  if(minute_t < 10) Serial.print("0");
  Serial.print(minute_t);
  Serial.print(":");
  if(second_t < 10) Serial.print("0");
  Serial.print(second_t);
  Serial.println();
}

