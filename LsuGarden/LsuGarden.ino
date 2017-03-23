#include <Arduino.h>
#include <pins_arduino.h>

#define OFF HIGH
#define ON  LOW

uint8_t out = OFF;
const uint8_t NB_PINS = 8;
uint8_t pin[NB_PINS] =
{
	D1, D2, D3, D4, D5, D6, D7, D8
};

void setup()
{
  for(uint8_t i = 0; i < NB_PINS; ++i)
  {
  	pinMode(pin[i], OUTPUT);
  	digitalWrite(pin[i], OFF);
  }
  out = OFF;
}

void loop()
{
  delay(5000);
  out = (++out) % 2;
  for(uint8_t i = 0; i < NB_PINS; ++i)
  {
  	digitalWrite(pin[i], out);
  }
}
