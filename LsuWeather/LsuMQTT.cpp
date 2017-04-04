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

uint8_t LsuMQTT::connect(const char* clientId,
    const char* username, const char* password, uint16_t keepAlive)
{
  uint8_t error = 0;
  // open TCP socket
//  if((error = _4G.openSocketClient(Wasp4G::CONNECTION_1, Wasp4G::TCP, (char*)host, port)) != 0)
//    return error;
  // allocate message buffer
  uint8_t buffer[MQTT_MAX_PACKET_SIZE];
  uint8_t idx = 0;
  memset(buffer, 0, MQTT_MAX_PACKET_SIZE);

  // Fixed header
  buffer[idx = 0] = 0x10; // CONNECT
  // byte at position 1 will be filler at the end with the length
  // Variable header
  // - Protocol Name
  buffer[idx = 2] = 0x00; // length MSB
  buffer[++idx] = 0x04;   // length LSB
  buffer[++idx] = 'M';
  buffer[++idx] = 'Q';
  buffer[++idx] = 'T';
  buffer[++idx] = 'T';
  // - Protocol Level, 4 = 3.1.1
  buffer[++idx] = 0x04;
  // - Connect Flags
  uint8_t cf = 0;
  if(username)
    cf |= (1 << 7);
  if(password)
    cf |= (1 << 6);
  buffer[++idx] = cf;
  // - Keep Alive
  buffer[++idx] = (keepAlive & 0xFF00) >> 8; // MSB
  buffer[++idx] = (keepAlive & 0x00FF);      // LSB
  // Payload
  // starts at next position (advance one)
  idx = writeString(clientId, buffer, (++idx));
  if(username)
    idx = writeString(username, buffer, (++idx));
  if(password)
    idx = writeString(password, buffer, (++idx));
  // fill in the length
  buffer[1] = (idx - 2);
  // terminate the buffer (really necessary?)
  buffer[idx + 1] = '\0';

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
  strcpy((char*)(&buffer[idx]), from);
  // exclude the null termination
  return idx + from_len - 1;
}

uint8_t LsuMQTT::encodeLength(uint32_t X, uint8_t* buff)
{
  if(X > 0x7F7F7F7F)
    return 0;
  uint8_t len;
  if(X > 0x7F7F7F)
    len = 4;
  else if(X > 0x7F7F)
    len = 3;
  else if(X > 0x7F)
    len = 2;
  else
    len = 1;
  uint8_t pos = len - 1;
  do
  {
    uint8_t encodedByte = X & 0x7F;
    *(buff + pos) = encodedByte;
    if(pos < len - 1)
    {
      *(buff + pos) |= 0x80;
    }
    --pos;
    X = X >> 7;
  } while (X > 0);
  return len;
}
