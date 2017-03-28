/*
 * LsuBME280_I2C.h
 *
 *  Created on: Mar 27, 2017
 *      Author: lsuciu
 */

#ifndef LSUBME280_I2C_H_
#define LSUBME280_I2C_H_

#include <Arduino.h>
#include <Wire.h>

#include <bme280.h>

namespace
{
struct bme280_t bme;

s8 BME280_I2C_bus_write(u8 dev_addr, u8 reg_addr,
    u8 *reg_data, u8 cnt = 1)
{
  u8 pos = 0;
  Wire.beginTransmission(dev_addr);
  while (pos < cnt)
  {
    Wire.write(reg_addr + cnt);
    Wire.write(*(reg_data + cnt));
    ++cnt;
  }
  Wire.endTransmission();
  return 0;
}

s8 BME280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
  u8 pos = 0;
  Wire.beginTransmission(dev_addr);
  Wire.write(reg_addr);
  Wire.endTransmission();
  Wire.requestFrom(dev_addr, cnt);
  while (pos < cnt)
  {
    *(reg_data + cnt) = Wire.read();
    ++cnt;
  }
  return 0;
}

void BME280_delay_msek(u32 msek)
{
  delay(msek);
}

} // namespace

void bme280_begin()
{
  // initialize BME280
  bme.dev_addr = BME280_I2C_ADDRESS2;

  bme.bus_write = BME280_I2C_bus_write;
  bme.bus_read = BME280_I2C_bus_read;
  bme.delay_msec = BME280_delay_msek;

  bme280_init(&bme);

  // Suggested settings for weather monitoring:
  // - Sensor mode: forced mode, 1 sample / minute
  // - Oversampling settings: pressure ×1, temperature ×1, humidity ×1
  // - IIR filter settings: filter off
  bme280_set_oversamp_humidity(BME280_OVERSAMP_1X);
  bme280_set_oversamp_pressure(BME280_OVERSAMP_1X);
  bme280_set_oversamp_temperature(BME280_OVERSAMP_1X);
  bme280_set_filter(BME280_FILTER_COEFF_OFF);
  bme280_set_standby_durn(BME280_STANDBY_TIME_1_MS);
}

void bme280_read(u32 *press_t, s32 *temp_t, u32 *humid_t)
{
  bme280_set_power_mode(BME280_FORCED_MODE);
  bme280_read_pressure_temperature_humidity(press_t, temp_t, humid_t);
}

#endif /* LSUBME280_I2C_H_ */
