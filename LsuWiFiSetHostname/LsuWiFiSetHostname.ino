#ifndef DEBUG
#define DEBUG 2
#endif

#ifdef DEBUG
#undef DEBUG
#define DEBUG 2
#endif

#include <LsuWiFiC.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer server(80);

void handleRoot()
{
  server.send(200, "text/html", "<h1>Mere</h1>");
}

void setup() {
  Serial.begin(115200);
  if(!WiFi.hostname("irigatie"))
  {
    Serial.println("\nset hostname failed");
    return;
  }
  Serial.println("set hostname ok");
  LsuWiFiC::connect(0, 10000, true, false);

  //Start mDNS
  if (MDNS.begin("irigatie")) {
    Serial.println("MDNS started");
  }
  
  server.on("/", handleRoot);
  server.begin();
  Serial.println("http server started");
}

void loop() {
  server.handleClient();
}
