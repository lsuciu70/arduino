
#include <ESP8266WiFi.h>
#include <LsuScheduler.h>

const char* SSID_t   = "cls-router";
const char* PASSWD_t = "r4cD7TPG";

byte ssid_ix = 0;

/*
 One of:         
 - WL_NO_SHIELD = 255,
 - WL_IDLE_STATUS = 0,
 - WL_NO_SSID_AVAIL,
 - WL_SCAN_COMPLETED,
 - WL_CONNECTED,
 - WL_CONNECT_FAILED,
 - WL_CONNECTION_LOST,
 - WL_DISCONNECTED
 */

enum
{
  WIFI_NOT_INITIALIZED,
  WIFI_NOT_CONNECTED,
  WIFI_CONNECTED,
};

byte progress = WIFI_NOT_INITIALIZED;

void connectWifi(void);
void waitWifiConnected(void);
void ntpSendRequest(void);
void ntpWaitResponse(void);

void registerTimeListener(void (*)(time_t));

LsuScheduler scheduler;







void setup() {
  // put your setup code here, to run once:
  progress = WIFI_NOT_CONNECTED;
}

void loop() {
  // put your main code here, to run repeatedly:

}

void connectWifi(void)
{
  if(WiFi.status() == WL_CONNECTED)
  {
    progress = WIFI_CONNECTED;
    scheduler.add(waitWifiConnected, 0);
    return;
  }
  if(progress == WIFI_NOT_INITIALIZED)
  {
    WiFi.begin(SSID_t, PASSWD_t);
    progress = WIFI_NOT_CONNECTED;
  }
  scheduler.add(connectWifi, 10);
}

void waitWifiConnected(void)
{
  
}

