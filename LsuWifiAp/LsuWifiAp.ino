#include <ESP8266WiFi.h>

const char *ssid = "lsu-mmcu-ap";
const char *password = "764FB61CC6";

void onWiFiEventCb(WiFiEvent_t);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
    if (!WiFi.softAP(ssid, password))
    {
        Serial.println("Starting AP failed");
    }
    else
    {
        Serial.println("Starting AP succeeded");
//        WiFi.onEvent(onWiFiEventCb, WIFI_EVENT_ANY);
    }
}

void loop() {
  // put your main code here, to run repeatedly:

}

void onWiFiEventCb(WiFiEvent_t evt)
{
    Serial.print(String("WiFiEvent: ") + evt);
    switch (evt)
    {
        case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
            Serial.println(" WIFI_EVENT_SOFTAPMODE_STACONNECTED");
            break;
        case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
            Serial.println(" WIFI_EVENT_SOFTAPMODE_STADISCONNECTED");
            break;
        case WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED:
            Serial.println(" WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED");
            break;
        default:
            Serial.println(" ** UNKNOWN **");
            break;
    }
}
