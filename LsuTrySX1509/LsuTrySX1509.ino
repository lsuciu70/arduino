#include <Arduino.h>
#include <SparkFunSX1509.h>

const byte SX1509_ADDRESS = 0x3E;
SX1509 io;
const byte SX1509_LED_PIN = 0;

void setup() {
  if (!io.begin(SX1509_ADDRESS))
  {
    while (1)
      ;
  }

  io.clock(INTERNAL_CLOCK_2MHZ, 4);
  // 1
  // io.pinMode(SX1509_LED_PIN, OUTPUT); // Set LED pin to OUTPUT

  // 2
  io.pinMode(SX1509_LED_PIN, ANALOG_OUTPUT);
}

void loop() {
// 1
//    io.digitalWrite(SX1509_LED_PIN, HIGH);
//    delay(200);
//    io.digitalWrite(SX1509_LED_PIN, LOW);
//    delay(200);

// 2
  for (int i=0; i<256; i++)
  {
    // PWM the LED from 0 to 255
    io.analogWrite(SX1509_LED_PIN, i);
    delay(2); // Delay 2ms between each
  }
  delay(100); // Delay half-second at the top.
  for (int i=255; i>=0; i--)
  {
    // PWM the LED from 255 to 0
    io.analogWrite(SX1509_LED_PIN, i);
    delay(2); // Delay 2ms between each
  }
  delay(100); // Delay half-second at the bottom.
}
