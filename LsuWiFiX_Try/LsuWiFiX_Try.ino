#include <LsuWiFiX.h>

void setup()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);
  LsuWiFi::addWiFi("cls-router", "r4cD7TPG");
  LsuWiFi::addWiFi("cls-ap", "r4cD7TPG");
  LsuWiFi::addWiFi("lsu-tpr", "r4cD7TPG");

  // asynch mode, will send the connect and return imediately
  LsuWiFi::connect(false);
  unsigned long timeout = 10000, start = millis();
  while(!WiFi.isConnected())
  {
    delay(100);
    if((millis() - start) > timeout)
      break;
  }
  if(!WiFi.isConnected())
  {
    // ERROR: not connected
    Serial.println("not connected");
  }
  else
  {
    Serial.println("connected");
  }
}

void loop()
{
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is acive low on the ESP-01)
  if(WiFi.isConnected())
  {
  	delay(250);                      // Wait for a second
  }
  else
  {
  	delay(250);
  	digitalWrite(LED_BUILTIN, HIGH);
  	delay(250);
  	digitalWrite(LED_BUILTIN, LOW);
  	delay(250);
  }
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(3000);                      // Wait for two seconds (to demonstrate the active low LED)
}
