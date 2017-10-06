#include <Arduino.h>

const uint8_t PINS = 8;
const uint8_t pins[PINS] =
{
	2, 3, 4, 5, 6, 7, 8, 9
};

volatile uint16_t value;

void readValue()
{
  for (uint8_t i = 0; i < PINS; ++i)
  {
    if (HIGH == digitalRead(pins[i]))
      bitSet(value, i);
    else
      bitClear(value, i);
  }
}
void printValue()
{
  Serial.print("value: ");
  Serial.println((value | 0x100), BIN);
}

void handleInterrupt()
{
  readValue();
  printValue();
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  Serial.println("Started");
}

bool initialized = false;

void lateInit()
{
  if(initialized)
    return;
  initialized = true;
  for (uint8_t i = 0; i < PINS; ++i)
  {
    pinMode(pins[i], INPUT);
  }
  Serial.println("Initialized");
  readValue();
  printValue();
}

void loop()
{
  lateInit();
}
