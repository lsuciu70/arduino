/*
 * LsuAlarmConstants.h
 *
 *  Created on: Feb 5, 2017
 *      Author: lsuciu
 */

#ifndef LSUALARMCONSTANTS_H_
#define LSUALARMCONSTANTS_H_

#include <Arduino.h>

byte SLAVE_ADDR = 8;

byte PIN_0 = 0;
byte PIN_1 = 1;
byte PIN_2 = 2;
byte PIN_3 = 3;
byte PIN_4 = 4;
byte PIN_5 = 5;
byte PIN_6 = 6;
byte PIN_7 = 7;
byte PIN_8 = 8;
byte PIN_9 = 9;
byte PIN_10 = 10;
byte PIN_11 = 11;
byte PIN_12 = 12;
byte PIN_13 = 13;

byte PINS_LEN = 14;

byte PINS[] =
{
    PIN_0,
    PIN_1,
    PIN_2,
    PIN_3,
    PIN_4,
    PIN_5,
    PIN_6,
    PIN_7,
    PIN_8,
    PIN_9,
    PIN_10,
    PIN_11,
    PIN_12,
    PIN_13
};

int ALARMS[] =
{
    (int) pow(2, (int) PIN_0),
    (int) pow(2, (int) PIN_1),
    (int) pow(2, (int) PIN_2),
    (int) pow(2, (int) PIN_3),
    (int) pow(2, (int) PIN_4),
    (int) pow(2, (int) PIN_5),
    (int) pow(2, (int) PIN_6),
    (int) pow(2, (int) PIN_7),
    (int) pow(2, (int) PIN_8),
    (int) pow(2, (int) PIN_9),
    (int) pow(2, (int) PIN_10),
    (int) pow(2, (int) PIN_11),
    (int) pow(2, (int) PIN_12),
    (int) pow(2, (int) PIN_13)
};

#endif /* LSUALARMCONSTANTS_H_ */
