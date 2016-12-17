#include <Time.h>         // https://github.com/PaulStoffregen/Time
#include <Timezone.h>     // https://github.com/JChristensen/Timezone
#include <WiFiServer.h>   // arduino/libraries/WiFi/src

#include <WiFiClient.h>   // arduino/libraries/WiFi/src
#include <WiFiUdp.h>      // arduino/libraries/WiFi/src
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino

#include <Wire.h>              // arduino-ide/hardware/arduino/avr/libraries/Wire/src
#include <LiquidCrystal_I2C.h> // https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/NewliquidCrystal_1.3.4.zip

#include <LsuScheduler.h> // https://github.com/lsuciu70/arduino/tree/master/libraries/LsuScheduler

// one second, 1000 milliseconds
const int SECOND = 1000;

// Temperatures section
// Sensor count per floor
const byte SENZOR_COUNT = 4;

// Room count
const byte ROOM_COUNT = 2 * SENZOR_COUNT;

// temperatures
int temperature[ROOM_COUNT];

// is running
bool is_running[ROOM_COUNT];

const String ROOM_NAMES[] =
{
	"DL: ", "Dm: ", "Do: ", "Be: ",
    "Bu: ", "Li: ", "Bi: ", "Bp: ",
};

// temperatures up to date
bool upToDateEtaj = false;
bool upToDateParter = false;
// Temperatures section - end

// WiFi section
const byte SSID_SIZE = 2;
const char* SSID_t[SSID_SIZE]   = {"cls-router", "cls-ap"};
const char* PASSWD_t[SSID_SIZE] = {"r4cD7TPG", "r4cD7TPG"};

byte ssid_ix = 0;

const String T_LOC_NAME = "t_loc";
// etaj
const String T_LOC_ETAJ = "etaj";
const byte ETAJ_OFFSET = 0;
const byte IP_ETAJ_BYTES[] = {192, 168, 100, 48};
const IPAddress IP_ETAJ(IP_ETAJ_BYTES);
// parter
const String T_LOC_PARTER = "parter";
const byte PARTER_OFFSET = SENZOR_COUNT;
const byte IP_PARTER_BYTES[] = {192, 168, 100, 20};
const IPAddress IP_PARTER(IP_PARTER_BYTES);

const int HTTP_PORT = 80;

WiFiServer server(HTTP_PORT);
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

const byte LCD_LINES = 4;
const byte LCD_CHARS_PER_LINE = 20;

// LiquidCrystal section - end

// Task scheduler section
LsuScheduler scheduler;
// Task scheduler section - end

// functions prototype section
void blink(int);

void connectWifi();

//byte* getIpBytes(String&, byte*);

byte* getIpBytes(String&, byte*);

time_t getTime();

void listen4HttpClient();

void parseRequest(const String &, const String &, String &);

void parseRequest(const String &, const String &, int &);

void processPostData_Programming(const String&);

int savePostData_Programming(const String&, int);

unsigned long secondsRunning();

void sendHttpResponse(WiFiClient &);

void sendPostData(const IPAddress&);

void sendTemperaturesRequest();

void sendTemperaturesRequestEtaj();

void sendTemperaturesRequestParter();

void showTemperaturesOnLcd();

void showTimeOnLcd();

String timeString();

String timeString(int, int, int, int, int, int);

void setup() {
  Serial.begin(115200);

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

  // initialize temperatures
  for (byte i = 0; i < ROOM_COUNT; ++i)
  {
    temperature[i] = 0;
    is_running[i] = false;
  }

  // initialize the lcd
  lcd.begin(LCD_CHARS_PER_LINE, LCD_LINES);
  lcd.setBacklight(HIGH);

//  scheduler.add(sendTemperaturesRequest, millis() + 3 * SECOND);
//  scheduler.add(showTimeOnLcd, millis() + 3 * SECOND);
}

void loop() {
  showTimeOnLcd();
  upToDateEtaj = false;
  upToDateParter = false;
  sendTemperaturesRequestParter();
  listen4HttpClient();
  sendTemperaturesRequestEtaj();
  lcd.clear();
  listen4HttpClient();
  blink(10 * SECOND);
  // scheduler.execute(millis());
}

//byte* getIpBytes(String& host, byte *addr)
//{
//  int start_idx = 0;
//  int stop_idx = 0;
//  int count = -1;
//  for(byte count = 0; count < 4; ++count)
//  {
//    stop_idx = host.indexOf(".", start_idx + 1);
//    if(stop_idx >= 0)
//      addr[count] = host.substring(start_idx, stop_idx).toInt();
//    else if(start_idx >= 0)
//      addr[count] = host.substring(start_idx).toInt();
//    start_idx = stop_idx + 1;
//  }
//  return addr;
//}

byte* getIpBytes(String& host, byte *addr)
{
  int start_idx = 0;
  int stop_idx = 0;
  int count = -1;
  for(byte count = 0; count < 4; ++count)
  {
    stop_idx = host.indexOf(".", start_idx + 1);
    if(stop_idx >= 0)
      addr[count] = host.substring(start_idx, stop_idx).toInt();
    else if(start_idx >= 0)
      addr[count] = host.substring(start_idx).toInt();
    start_idx = stop_idx + 1;
  }
  return addr;
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
  msg += String(", adresa IP: ") + WiFi.localIP().toString();
  Serial.println(msg);
}

void listen4HttpClient()
{
//Serial.println("listen4HttpClient");
  if (WiFi.status() != WL_CONNECTED)
    connectWifi();
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("ERROR: WiFi connection failed.");
    return;
  }
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
Serial.println("listen4HttpClient - new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean hasReadPostData = false;
    int content_length = 0;
    String req_str = "", post_data="";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        req_str += c;
        if (c =='\n' && currentLineIsBlank)
        {
          if (req_str.indexOf("GET /") >= 0)
          {
          	String host;
          	parseRequest(req_str, "Host:", host);
          	int uir;
          	parseRequest(req_str, "Upgrade-Insecure-Requests:", uir);
          	Serial.println(host);
          	Serial.println(uir);
          	Serial.println();
            sendHttpResponse(client);
            break;
          }
          if (req_str.indexOf("POST") >= 0)
          {
            if(!hasReadPostData)
            {
              String host;
              parseRequest(req_str, "Host:", host);
              parseRequest(req_str, "Content-Length:", content_length);
              while(content_length-- > 0)
              {
                c = client.read();
                req_str += c;
                post_data += c;
              }
              hasReadPostData = true;
 	Serial.println(post_data);
             processPostData_Programming(post_data);
              byte addr[4];
              IPAddress sender(getIpBytes(host, addr));
              if(sender == IP_ETAJ)
                upToDateEtaj = true;
              if(sender == IP_PARTER)
                upToDateParter = true;
    Serial.println(String("") + upToDateEtaj + " + " + upToDateParter);
            }
            break;
          }
        }
        if (c =='\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c !='\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    client.stop();
    if(upToDateEtaj && upToDateParter)
    {
//      scheduler.add(sendTemperaturesRequest, millis() + 10 * SECOND);
//      scheduler.add(showTemperaturesOnLcd, millis() + 10);
    	showTemperaturesOnLcd();
    }
  }
}

void parseRequest(const String &request, const String &field, String &value)
{
  int start_idx;
  if((start_idx = request.indexOf(field)) < 0)
    return;
  start_idx += field.length();
  int stop_idx = request.indexOf("\n", start_idx);
  if(stop_idx < start_idx)
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

void processPostData_Programming(const String &post_data)
{
	Serial.println("processPostData");
  int i = 0, j=0;
  int offset = -1;
  while((j = post_data.indexOf("&", i)) >= 0)
  {
    offset = savePostData_Programming(post_data.substring(i, j), offset);
    i = j + 1;
  }
  if(i)
    offset = savePostData_Programming(post_data.substring(i, j), offset);
}

int savePostData_Programming(const String &data, int offset)
{
Serial.println(String("savePostData - offset: ") + offset + ", data: " + data);
  int eq_idx = data.indexOf("=");
  if(eq_idx < 0)
  {
Serial.println(String("EROARE salvare - date invalide (lipseste semnul'='); ") + data);
    return offset;
  }
  String name = data.substring(0, eq_idx);
  name.trim();
  String value = data.substring(eq_idx + 1);
  value.trim();
  if(name.equals(T_LOC_NAME) && value.equals(T_LOC_PARTER))
  {
    return PARTER_OFFSET;
  }
  if(name.equals(T_LOC_NAME) && value.equals(T_LOC_ETAJ))
  {
    return ETAJ_OFFSET;
  }
  if(offset != PARTER_OFFSET && offset != ETAJ_OFFSET)
  {
 Serial.println(String("EROARE salvare - offset necunoscut: ") + offset + "; " + data);
    return offset;
  }
Serial.println(String("savePostData - offset: ") + offset + ", data: " + data);
  int index_start_idx = name.indexOf("_");
  int index_stop_idx = name.lastIndexOf("_");
  if(index_start_idx < 0)
  {
Serial.println(String("EROARE salvare - date invalide (lipseste semnul'_'); ") + data);
    return offset;
  }
  byte index;
  bool runn = index_start_idx != index_stop_idx;
//Serial.println(String("savePostData - offset: ") + offset + ", data: " + data);
  if(!runn)
  {
    index = name.substring(index_start_idx + 1).toInt();
    temperature[index + offset] = value.toInt();
  }
  else
  {
    index = name.substring(index_start_idx + 1, index_stop_idx).toInt();
    is_running[index + offset] = (value.toInt() == 0) ? false : true;
  }
  return offset;
}

unsigned long secondsRunning()
{
  return millis() / SECOND;
}

void sendHttpResponse(WiFiClient &client)
{
  String header = "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\nRefresh: 10\r\n";
  client.println(header);
  String response = String("<!DOCTYPE html>\n") +
      "<html>" +
      "<head>\n" +
      " <title>CLLS Clock</title>\n" +
      "</head>\n" +
	  "<body>\n" +
	  " " + timeString() +
      "</body>\n" +
      "</html>\n";
  client.println(response);
}

void sendPostData(const IPAddress & server)
{
Serial.println("sendPostData");
  String post_data = "temperatures";
  String post_req = String("") +
  "POST / HTTP/1.1\r\n" +
  "Host: " + WiFi.localIP().toString() + "\r\n" +
  "User-Agent: Arduino/1.0\r\n" +
  "Accept: temperatures\r\n" +
  "Connection: close\r\n" +
  "Content-Type: application/x-www-form-urlencoded\r\n" +
  "Content-Length: " + post_data.length() + "\r\n";
  WiFiClient client;
Serial.println("sendPostData - connect");
  if(client.connect(server, HTTP_PORT))
  {
Serial.println("sendPostData - connected");
    client.println(post_req);
    client.println(post_data);
Serial.println("sendPostData - sent");
    delay(100);
    client.stop();
  }
Serial.println("sendPostData - done");
}

void sendTemperaturesRequest()
{
  upToDateEtaj = false;
  upToDateParter = false;
  scheduler.add(sendTemperaturesRequestParter, millis() + 10);
  scheduler.add(sendTemperaturesRequestEtaj, millis() + 500);
}

void sendTemperaturesRequestEtaj()
{
  connectWifi();
  sendPostData(IP_ETAJ);
}

void sendTemperaturesRequestParter()
{
  connectWifi();
  sendPostData(IP_PARTER);
}

void blink(int mills)
{
  int count = 0;
  bool show_it = false;
  while(count < mills)
  {
    delay(500);
    count += 500;
    for(int i = 0; i < LCD_LINES; ++i)
    {
      // first and second index for temperatures, room names, etc. in i'th line
      int i1 = 2 * i;
      int i2 = 2 * i + 1;
      if(is_running[i1])
      {
        lcd.setCursor(2, i);
        if(!show_it)
          lcd.print(" ");
        else
          lcd.print(":");
      }
      if(is_running[i2])
      {
        lcd.setCursor(12, i);
        if(!show_it)
          lcd.print(" ");
        else
          lcd.print(":");
      }
    }
    show_it = !show_it;
  }
}

void showTemperaturesOnLcd()
{
Serial.println("showTemperaturesOnLcd");
  String lines[LCD_LINES];
  // fill lines ROOM_NAMES
  for(int i = 0; i < LCD_LINES; ++i)
  {
    // first and second index for temperatures, room names, etc. in i'th line
    int i1 = 2 * i;
    int i2 = 2 * i + 1;

    char buff[5];
    int temp = temperature[i1];
Serial.println(String("temp[") + i1 + "] = " + temp);
    sprintf(buff, "%2d.%02d", temp / 100, temp % 100);
    String t1_str(buff);
    temp = temperature[i2];
Serial.println(String("temp[") + i2 + "] = " + temp);
    sprintf(buff, "%2d.%02d", temp / 100, temp % 100);
    String t2_str(buff);
    lines[i] = String(ROOM_NAMES[i1]) + t1_str + " " + ROOM_NAMES[i2] + t2_str + " ";
  }
  lcd.clear();
  lcd.home();
  for(int i = 0; i < LCD_LINES; ++i)
  {
  	lcd.setCursor(0, i);
    lcd.print(lines[i]);
  }
}

void showTimeOnLcd()
{
	Serial.println("showTimeOnLcd");
  String timeStr = timeString();
  Serial.println(timeStr);
  lcd.clear();
  lcd.home();
  lcd.setCursor(0, 0);
  lcd.print(timeStr);
//  scheduler.add(showTimeOnLcd, millis() + SECOND);
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
//  sprintf(buff, "%02d-%02d-%04d %02d:%02d:%02d", day_t, month_t, year_t, hour_t, minute_t, second_t);
  sprintf(buff, "  %02d-%02d-%04d %02d:%02d", day_t, month_t, year_t, hour_t, minute_t);
  return String(buff);
}

