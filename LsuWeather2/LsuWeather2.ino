#include "LsuBME280_I2C.h"

void setup() {
  // initialize BME280
  bme280_begin();
}

void loop() {
  u32 press_t;
  s32 temp_t;
  u32 humid_t;
  bme280_read(&press_t, &temp_t, &humid_t);
}
