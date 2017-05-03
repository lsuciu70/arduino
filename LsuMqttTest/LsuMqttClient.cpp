/*
 * LsuMqttClient.cpp
 *
 *  Created on: May 2, 2017
 *      Author: lsuciu
 */

#include "LsuMqttClient.h"

#include <stdint.h>
#include <math.h>

#ifdef REMAINING_LENGTH_BUFF_LEN
#undef REMAINING_LENGTH_BUFF_LEN
#endif
#define REMAINING_LENGTH_BUFF_LEN 4

namespace
{

uint8_t encodeLength(uint32_t number, uint8_t* buff)
{
  if (number > (pow(2, (REMAINING_LENGTH_BUFF_LEN * 7)) - 1))
    // needs more then (7 * REMAINING_LENGTH_BUFF_LEN) bits -> error
    return 0;
  uint8_t pos = 0, i;
  do
  {
    *(buff + pos) = number & 0x7F;
  } while ((++pos) && (number = number >> 7) > 0);
  for (i = 0; i < pos - 1; ++i)
  {
    *(buff + i) |= 0x80;
  }
  return pos;
}

uint32_t decodeLength(uint8_t* buff)
{
  uint32_t number = 0, multiplier = 1;
  uint8_t byte = 0, byte_pos = 0;
  while((byte = *(buff + byte_pos)) && ++byte_pos)
  {
    number += ((byte & 0x7F) * multiplier);
    if(byte_pos >= REMAINING_LENGTH_BUFF_LEN)
      // read all REMAINING_LENGTH_BUFF_LEN possible positions
      break;
    multiplier *= 0x80;
  }
  return number;
}

} /* anonymous namespace */

namespace MQTT
{

} /* namespace MQTT */
