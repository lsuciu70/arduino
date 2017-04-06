/*
 * LsuMQTT.cpp
 *
 *  Created on: Mar 30, 2017
 *      Author: lsuciu
 */

#include <stdbool.h>
#include <string.h>

#include "LsuMQTT.h"

LsuMQTT::LsuMQTT(const char* host_t, uint16_t port_t) :
    host(host_t), port(port_t)
{
}

LsuMQTT::~LsuMQTT()
{
}

uint8_t LsuMQTT::connect(const char* clientId, const char* username,
    const char* password, uint16_t keepAlive)
{
  // calculate Remaining Length
  // Variable header = 10 + ...
  uint32_t remaining_length = 10 + strlen(clientId) + 1;
  if (username)
  {
    remaining_length += strlen(username) + 1;
    if (password)
      remaining_length += strlen(password) + 1;
  }
  // allocate buffer for encoded remaining length (max 4 bytes)
  uint8_t rl_buff[4];
  uint8_t rl_len = encodeLength(remaining_length, rl_buff);
  // calculate message length
  uint8_t buffer_len = 1 + rl_len + remaining_length;
  // allocate message buffer
  uint8_t buffer[buffer_len];
  uint8_t idx = 0;
  memset(buffer, 0, buffer_len);

  // Fixed header
  buffer[idx = 0] = 0x10; // CONNECT
  memcpy(buffer + 1, rl_buff, rl_len); // Remaining Length
  idx += rl_len;
  // byte at position 1 will be filler at the end with the length
  // Variable header
  // - Protocol Name
  buffer[++idx] = 0x00; // Protocol Name length MSB
  buffer[++idx] = 0x04; // Protocol Name length LSB
  buffer[++idx] = 'M';
  buffer[++idx] = 'Q';
  buffer[++idx] = 'T';
  buffer[++idx] = 'T';
  // - Protocol Level, 4 = 3.1.1
  buffer[++idx] = 0x04;
  // - Connect Flags
  uint8_t cf = 0;
  if (username)
  {
    cf |= (1 << 7);
    if (password)
      cf |= (1 << 6);
  }
  buffer[++idx] = cf;
  // - Keep Alive
  buffer[++idx] = (keepAlive & 0xFF00) >> 8; // MSB
  buffer[++idx] = (keepAlive & 0x00FF);      // LSB
  // Payload
  // starts at next position (advance one)
  idx = writeString(clientId, buffer, (++idx));
  if (username)
  {
    idx = writeString(username, buffer, (++idx));
    if (password)
      idx = writeString(password, buffer, (++idx));
  }
  return 0;
}

/**
 *
 */
uint8_t LsuMQTT::writeString(const char* from, uint8_t* buffer, uint8_t buf_pos)
{
  uint8_t idx = buf_pos, from_len = strlen(from);
  // write the string length
  buffer[idx++] = (from_len & 0xFF00) >> 8; // MSB of length
  buffer[idx++] = from_len & 0xFF;          // LSB of length
  strcpy((char*) (&buffer[idx]), from);
  // exclude the null termination
  return idx + from_len - 1;
}

uint8_t LsuMQTT::encodeLength(uint32_t X, uint8_t* buff)
{
  uint8_t len = 0;
  if (X > 0x7F7F7F7F)
    return len;
  else if (X > 0x7F7F7F)
    len = 4;
  else if (X > 0x7F7F)
    len = 3;
  else if (X > 0x7F)
    len = 2;
  else
    len = 1;
  uint8_t pos = len - 1;
  do
  {
    uint8_t encodedByte = X & 0x7F;
    *(buff + pos) = encodedByte;
    if (pos < len - 1)
    {
      *(buff + pos) |= 0x80;
    }
    --pos;
    X = X >> 7;
  } while (X > 0);
  return len;
}
