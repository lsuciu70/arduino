/*
 * LsuMQTT.h
 *
 *  Created on: Mar 30, 2017
 *      Author: lsuciu
 */

#ifndef LSUMQTT_H_
#define LSUMQTT_H_

#include <stdint.h>

#include <Wasp4G.h>

#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 127
#endif

class LsuMQTT
{
public:
  LsuMQTT(const char*, uint16_t);
  ~LsuMQTT();

  uint8_t connect(const char* clientId, const char* username = 0,
      const char* password = 0, uint16_t keepAlive = 0);
private:
  const char* host;
  uint16_t port;

  uint8_t writeString(const char*, uint8_t*, uint8_t);
  uint8_t encodeLength(uint32_t, uint8_t*);
};

#endif /* LSUMQTT_H_ */
