/*
 * LsuBME280.cpp
 *
 *  Created on: Jan 24, 2017
 *      Author: lsuciu
 */

#include <Arduino.h>

#include "LsuBME280.h"

void LsuBME280::start(uint8_t commInterface, uint8_t I2CAddress,
        uint8_t chipSelectPin, uint8_t runMode, uint8_t tStandby,
        uint8_t filter, uint8_t tempOverSample, uint8_t pressOverSample,
        uint8_t humidOverSample)
{
    settings.commInterface = commInterface;
    if (commInterface == I2C_MODE)
        settings.I2CAddress = I2CAddress;
    if (commInterface == SPI_MODE)
        settings.chipSelectPin = chipSelectPin;
    settings.runMode = runMode;
    settings.tStandby = tStandby;
    settings.filter = filter;
    settings.tempOverSample = tempOverSample;
    settings.pressOverSample = pressOverSample;
    settings.humidOverSample = humidOverSample;
    // BME280 requires 2ms to start up. Make sure sensor had enough time to turn on.
    delay(5);
    // Calling .begin() causes the settings to be loaded
    begin();
}

