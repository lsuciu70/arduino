/*
 * LsuMqttClient.h
 *
 *  Created on: May 2, 2017
 *      Author: lsuciu
 */

#ifndef LSUMQTTCLIENT_H_
#define LSUMQTTCLIENT_H_

#include <functional>

namespace MQTT
{
typedef std::function<size_t(const uint8_t*, size_t)> write;
typedef std::function<int(uint8_t*, size_t)> read;

void begin(read, write);

uint8_t sendConnect(const char*, const char* username = 0,
    const char* password = 0, uint16_t keepAlive = 15, bool print = false);

uint8_t receiveConnack(bool print = false);

uint8_t publish(const char*, const char*, bool retained = false, bool print = false);

uint8_t sendDisconnect(bool print = false);

} /* namespace MQTT */

#endif /* LSUMQTTCLIENT_H_ */
