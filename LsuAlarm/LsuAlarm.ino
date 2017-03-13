#include <Arduino.h>

static const uint8_t PINS = 10;

static const uint8_t pin[PINS] =
{
  D1, D2, D3, D4, D5, D6, D7, D8, D9, D10
};

volatile uint16_t value;

void handleInterrupt();

void setup() {
  value = 0;
  // make all pins as INPUT_PULLUP
  for(uint8_t i = 0; i < PINS ; ++i)
  {
    pinMode(pin[i], INPUT);
    attachInterrupt(digitalPinToInterrupt(pin[i]), handleInterrupt, CHANGE);
  }
}

void loop()
{
}

void handleInterrupt()
{
  for(uint8_t i = 0; i < PINS ; ++i)
  {
    if(HIGH == digitalRead(pin[i]))
      bitSet(value, i);
    else
      bitClear(value, i);
  }  
}

