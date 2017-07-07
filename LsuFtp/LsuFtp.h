/*
 * LsuFtp.h
 *
 *  Created on: Jul 6, 2017
 *      Author: lsuciu
 */

#ifndef LSUFTP_H_
#define LSUFTP_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>

#define OUTBUF_LEN 127

namespace LsuFtp
{

void begin(WiFiClient*);

bool connect(const char* server, const char* user = 0, const char* password = 0, uint16_t port = 21);

} /* namespace LsuFtp */

#endif /* LSUFTP_H_ */
