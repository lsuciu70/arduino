/*
 * LsuLogger.h
 *
 *  Created on: Jan 19, 2017
 *      Author: lsuciu
 */

#ifndef LSUWEBLOGGER_H_
#define LSUWEBLOGGER_H_

#include <algorithm>

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <LsuWiFi.h>

class LsuWebLogger
{
private:
    static const String T_LOC_NAME;
    static const String T_LOG_NAME;
    static const String EMPTY;

    static const int MAX_LOGGER;

    String* deferred_log[];
    unsigned long deferred_time[];
    int deferred_index;

    const String loc;
    const String page;
    const IPAddress server;
    const unsigned int port;
public:
    LsuWebLogger(const String &loc_t = EMPTY, const String &page_t = EMPTY,
            const IPAddress server_t =
            LsuWiFi::NO_IP, const unsigned int port_t = 0) :
            deferred_index(0), loc(loc_t), page(page_t), server(server_t), port(
                    port_t)
    {
        std::fill(deferred_log, deferred_log + MAX_LOGGER, (String *) 0);
        std::fill(deferred_time, deferred_time + MAX_LOGGER, 0);
    }
    virtual ~LsuWebLogger()
    {
    }

    void writeLogger(const String &);
private:
    void sendPostData(const String &);
    void defere_log(const String &);
    void recal_log();
};

#endif /* LSUWEBLOGGER_H_ */
