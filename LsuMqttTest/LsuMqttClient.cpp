/*
 * LsuMqttClient.cpp
 *
 *  Created on: May 2, 2017
 *      Author: lsuciu
 */

#include "LsuMqttClient.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

#ifdef REMAINING_LENGTH_BUFF_LEN
#undef REMAINING_LENGTH_BUFF_LEN
#endif
#define REMAINING_LENGTH_BUFF_LEN 4

namespace
{

MQTT::read read_funct = 0;
MQTT::write write_fct = 0;

/**
 * Writes
 */
uint8_t writeString(const char* source, uint8_t* target, const uint32_t& target_pos,
    bool write_source_length = true)
{
  uint32_t idx = target_pos, source_len = strlen(source);
  if (write_source_length)
  {
    // write the string length
    target[idx++] = (source_len & 0xFF00) >> 8; // MSB of length
    target[idx++] = source_len & 0xFF;          // LSB of length
  }
  strcpy((char*) (&target[idx]), source);
  // exclude the null termination
  return idx + source_len - 1;
}

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

void begin(read read_funct_ptr, write write_fct_ptr)
{
  read_funct = read_funct_ptr;
  write_fct = write_fct_ptr;
}

uint8_t sendConnect(const char* clientId, const char* username,
    const char* password, uint16_t keepAlive)
{
  // calculate Remaining Length
  // Variable header = 10 + ...
  uint32_t remaining_length = 10 + strlen(clientId) + 2;
  if (username)
  {
    remaining_length += strlen(username) + 2;
    if (password)
      remaining_length += strlen(password) + 2;
  }
  // allocate buffer and fill the encoded remaining length (max 4 bytes)
  uint8_t rl_buff[4];
  memset(rl_buff, 0, 4);
  uint8_t rl_buff_len = encodeLength(remaining_length, rl_buff);

  // calculate message length
  uint32_t length = 1 + rl_buff_len + remaining_length;
  // allocate message buffer
  uint8_t buffer[length];
  uint32_t idx = 0;
  memset(buffer, 0, length);

  // Fixed header
  buffer[idx = 0] = 0x10; // CONNECT
  memcpy(buffer + 1, rl_buff, rl_buff_len); // Remaining Length
  idx += rl_buff_len;
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
  cf |= (1 << 1); // clean session
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
  return write_fct(buffer, length);
}

uint8_t receiveConnack()
{
  size_t sz = 4;
  uint8_t buffer[sz + 1];
  read_funct(buffer, sz);
  uint8_t errorCode;// = _4G.receive(Wasp4G::CONNECTION_1, 20000);
//  if (0 != errorCode)
//  {
//    USB.print(F("MQTT::receive CONNACK error ["));
//    USB.print((int)errorCode);
//    USB.print(F("]: "));
//    switch (errorCode)
//    {
//    case 1:
//      USB.println(F("no data received"));
//      break;
//    case 2:
//      USB.println(F("error getting socket info"));
//      break;
//    case 3:
//      USB.println(F("timeout waiting for data"));
//      break;
//    case 4:
//      USB.println(F("error receiving data from module"));
//      break;
//    case 5:
//      USB.println(F("error parsing length of data"));
//      break;
//    case 6:
//      USB.println(F("error reading incoming bytes"));
//      break;
//    default:
//      USB.println(F("unknown"));
//      break;
//    }
//    return 0xFF;
//  }
//  // OK
//  // allocate message buffer
//  uint16_t length = _4G._length;
//  uint8_t buffer[length];
//  memcpy(buffer, _4G._buffer, length);
//  if (print)
//  {
//    USB.print(F("MQTT::received CONNACK msg: "));
//    printBuff(buffer, length);
//  }
//  // byte 0: 0x20 - CONNACK
//  if (buffer[0] != 0x20)
//  {
//    USB.println(F("MQTT::receive CONNACK error: not a CONNACK message"));
//    return 0xFE;
//  }
//  // byte 1: 0x02 - Remaining Length
//  if (buffer[1] != 0x02)
//  {
//    USB.println(F("MQTT::receive CONNACK error: wrong Remaining Length"));
//    return 0xFD;
//  }
  // byte 3: Connect Return code:
  //  0x00 Connection Accepted
  //  0x01 Connection Refused, unacceptable protocol version
  //  0x02 Connection Refused, identifier rejected
  //  0x03 Connection Refused, Server unavailable
  //  0x04 Connection Refused, bad user name or password
  //  0x05 Connection Refused, not authorized
  //  0x06-0xFF Reserved for future use
  if ((errorCode = buffer[3]) != 0x00)
  {
//    USB.print(F("MQTT::receive CONNACK error ["));
//    USB.print((int)errorCode);
//    USB.print(F("]: "));
    switch (errorCode)
    {
    case 1:
//      USB.println(F("unacceptable protocol version"));
      break;
    case 2:
//      USB.println(F("identifier rejected"));
      break;
    case 3:
//      USB.println(F("server unavailable"));
      break;
    case 4:
//      USB.println(F("bad user name or password"));
      break;
    case 5:
//      USB.println(F("not authorized"));
      break;
    default:
//      USB.println(F("reserved for future use"));
      break;
    }
  }
  return errorCode;
}

uint8_t publish(const char* topic, const char* payload, bool retained,
    bool print)
{
  // calculate Remaining Length
  // Variable header = 2 + ...
// No Packet Identifier on QoS 0
  uint32_t remaining_length = strlen(topic) + 2 /*+ 2*/;
  if (payload)
  {
    remaining_length += strlen(payload);
  }
  // allocate buffer and fill the encoded remaining length (max 4 bytes)
  uint8_t rl_buff[4];
  memset(rl_buff, 0, 4);
  uint8_t rl_buff_len = encodeLength(remaining_length, rl_buff);

  // calculate message length
  uint32_t length = 1 + rl_buff_len + remaining_length;
  // allocate message buffer
  uint8_t buffer[length];
  uint32_t idx = 0;
  memset(buffer, 0, length);

  // Fixed header
  buffer[idx = 0] = 0x31; // PUBLISH | RETAIN
  memcpy(buffer + 1, rl_buff, rl_buff_len); // Remaining Length
  idx += rl_buff_len;
  // Variable header
  idx = writeString(topic, buffer, (++idx)); // Topic Name
// No Packet Identifier on QoS 0
//  buffer[++idx] = 0x00;
//  buffer[++idx] = 0x01; // Packet Identifier = 1
  // Payload
  idx = writeString(payload, buffer, (++idx), false);
  return write_fct(buffer, length);
}

uint8_t sendDisconnect(bool print)
{
  // calculate message length
  uint8_t length = 2;
  // allocate message buffer
  uint8_t buffer[length];
  uint8_t idx = 0;
  memset(buffer, 0, length);
  buffer[idx] = 0xE0; // DISCONNECT
  buffer[++idx] = 0x00;
  return write(buffer, length);
}

} /* namespace MQTT */
