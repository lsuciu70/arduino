/*
 * NodeMcuPins.h
 *
 *  Created on: Mar 13, 2017
 *      Author: lsuciu
 */

#ifndef NODEMCUPINS_H_
#define NODEMCUPINS_H_

#include <stdint.h>

static const uint8_t PINS = 11;

static const uint8_t LED_BUILTIN = 16;

static const uint8_t SDA = 4;
static const uint8_t SCL = 5;

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

uint8_t pin[PINS] =
{
  D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10
};

#endif /* NODEMCUPINS_H_ */
