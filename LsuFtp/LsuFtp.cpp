/*
 * LsuFtp.cpp
 *
 *  Created on: Jul 6, 2017
 *      Author: lsuciu
 */

#include "LsuFtp.h"

namespace
{


char outBuf[OUTBUF_LEN + 1];
char outCount;

WiFiClient* client = 0;

void failed()
{

}

bool receive()
{
  uint8_t respCode, thisByte;
  while (!client->available())
  {
    delay(1);
  }
  respCode = client->peek();
  outCount = 0;
  while (client->available())
  {
    thisByte = client->read();
    Serial.write(thisByte);
    if(outCount < OUTBUF_LEN)
    {
      outBuf[outCount] = thisByte;
      outBuf[++outCount] = '\0';
    }
  }
  if (respCode >= '4')
  {
    failed();
    return false;
  }
  return true;
}

} /* anonymous namespace */


namespace LsuFtp
{

void begin(WiFiClient* clientPtr)
{
  client = clientPtr;
}

bool connect(const char* server, const char* user, const char* password, uint16_t port)
{
  if(!client || !client->connect(server, port))
  {
    return false;
  }
  return true;
}

} /* namespace LsuFtp */
