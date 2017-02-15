/*
 * LsuBME280.h
 *
 *  Created on: Jan 24, 2017
 *      Author: lsuciu
 */

#include <SparkFunBME280.h> // https://github.com/sparkfun/SparkFun_BME280_Arduino_Library

#ifndef LSUBME280_H_
#define LSUBME280_H_

class LsuBME280: public BME280
{
public:
    LsuBME280()
    {
    }
    virtual ~LsuBME280()
    {
    }

    /**
     * Starts the BME280 sensor.
     * commInterface one of I2C_MODE or SPI_MODE, default I2C_MODE
     * I2CAddress default 0x77, used for I2C mode
     * chipSelectPin default 10, used for SPI mode
     * runMode can be: 0, Sleep mode; 1 or 2, Forced mode; 3, Normal mode; default 3
     * tStandby can be: 0, 0.5ms; 1, 62.5ms; 2, 125ms; 3, 250ms; 4, 500ms; 5, 1000ms; 6, 10ms; 7, 20ms; default 0
     * filter can be off or number of FIR coefficients to use: 0, filter off; 1, coefficients = 2; 2, coefficients = 4; 3, coefficients = 8; 4, coefficients = 1; default 0
     * tempOverSample can be: 0, skipped; 1 through 5, oversampling *1, *2, *4, *8, *16 respectively; default 5
     * pressOverSample can be: 0, skipped; 1 through 5, oversampling *1, *2, *4, *8, *16 respectively; default 5
     * humidOverSample can be: 0, skipped; 1 through 5, oversampling *1, *2, *4, *8, *16 respectively; default 5
     */
    virtual void start(uint8_t commInterface = I2C_MODE, uint8_t I2CAddress =
            0x77, uint8_t chipSelectPin = 10, uint8_t runMode = 3,
            uint8_t tStandby = 0, uint8_t filter = 0,
            uint8_t tempOverSample = 5, uint8_t pressOverSample = 5,
            uint8_t humidOverSample = 5);
};

#endif /* LSUBME280_H_ */
