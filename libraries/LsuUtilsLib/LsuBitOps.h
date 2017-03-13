/*
 * lsuBitOps.h
 *
 *  Created on: Mar 13, 2017
 *      Author: lsuciu
 */

#ifndef LSUBITOPS_H_
#define LSUBITOPS_H_

#include <stdint.h>
#include <cmath>

/**
 * Returns the value that masks given bit, 2 ^ bit.
 * It works for first 16 bits (0 - 15); if bit is greater than 15 return 0 (zero).
 */
inline uint16_t maskBit(const uint8_t bit)
{
  if(bit > 15)
    return 0;
  return (uint16_t)pow(2, bit);
}

/**
 * Sets given bit into given 'to' number and returns the new number.
 * It works for first 16 bits (0 - 15); if bit is greater than 15 return 0xFFFF.
 */
inline uint16_t setBit(const uint8_t bit, const uint16_t to)
{
  return (to | maskBit(bit));
}

/**
 * Returns true if 'bit' bit is set into 'to' number; false otherwise.
 */
inline bool isBitSet(const uint8_t bit, const uint16_t to)
{
  return to & maskBit(bit);
}

#endif /* LSUBITOPS_H_ */
