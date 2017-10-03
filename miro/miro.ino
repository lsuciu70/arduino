#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <ESP8266WebServer.h>
#include <DHT.h>

#define SECOND 1000

/* User specific area begin */

const char* ssid = "cls-router";
const char* passwd = "r4cD7TPG";

const char* HTTP_REFRESH = "10"; // 10 seconds


#define DHTPIN 2      // Digital pin DHT-22 is connected to
#define DHTTYPE DHT22 // DHT-22 type

const unsigned int MEASURE_INTERVAL = 20 * SECOND;

/* User specific area end */


// HTTP server
ESP8266WebServer server(80);
// Sensor
DHT dht(DHTPIN, DHTTYPE);

float temperature, humidity;
unsigned long nextMeasureTime;

void wifiConnect();
void doMeasurements();
void handleRoot();

void setup()
{
  // Start serial at 115200 baud
  Serial.begin(115200);
  Serial.println();

  // Start sensor
  dht.begin();
  nextMeasureTime = 0;
  
  // Set WiFi as client
  WiFi.mode(WIFI_STA);
  // Register handler (callback) for root
  server.on("/", handleRoot);
  // Start HTTP server
  server.begin();
}

void loop()
{
  wifiConnect();
  if(millis() > nextMeasureTime)
  {
  	nextMeasureTime = millis() + MEASURE_INTERVAL;
  	doMeasurements();
  }
  server.handleClient();
}

void wifiConnect()
{
  if(WiFi.status() == WL_CONNECTED)
    return;
  Serial.printf("Connecting to: %s\n", ssid);
  WiFi.begin(ssid, passwd);
  unsigned int count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(10);
    ++count;
    if ((count % 100) == 0)
      Serial.print(". ");
    if ((count % (1000)) == 0)
    {
      Serial.println("failed, retry");
      return wifiConnect();
    }
  }
  Serial.println("done");
  Serial.print(F("  MAC: "));
  Serial.println(WiFi.macAddress());
  Serial.print(F("  IP:  "));
  Serial.println(WiFi.localIP().toString());
}

void doMeasurements()
{
  float t = dht.readTemperature();
  if(!isnan(t))
    temperature = t;
  float h = dht.readHumidity();
  if(!isnan(h))
    humidity = h;
}

const char content_root_fmt[] =
{"<!DOCTYPE html>"
"<html>"
"<head><meta charset='UTF-8'></head>"
"<body>"
"<table cellpadding='5'>"
"<tr><td>Temperature</td><td>%d.%d</td><td> [&deg;C]</td></tr>"
"<tr><td>Humidity</td><td>%d.%d</td><td> [%]</td></tr>"
"</table>"
"</body>"
"</html>"};

void handleRoot()
{
  // There's not enough calculation power to sprintf floats,
  // we have to make them ints
  int t_int = (int)round(temperature * 100); // centigrades
  int t_d = t_int % 100; // decimal part
  int t_i = (t_int - t_d) / 100; // integral part
  if(t_d < 0)
    t_d *= -1;

  int h_int = (int)round(humidity * 100); // take two decimals
  int h_d = h_int % 100; // decimal part
  int h_i = (h_int - h_d) / 100; // integral part
  if(h_d < 0)
    h_d *= -1;
  
  // HTTP refresh every HTTP_REFRESH seconds
  server.sendHeader("Refresh", HTTP_REFRESH);
  
  char content_root[strlen(content_root_fmt) + 4];
  sprintf(content_root, content_root_fmt, t_i, t_d, h_i, h_d);
  server.send(200, "text/html", content_root);
}

