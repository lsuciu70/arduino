/*
 * LsuLogger.cpp
 *
 *  Created on: Jan 19, 2017
 *      Author: lsuciu
 */

#include <LsuNtpTime.h>

#include "LsuLogger.h"

const String LsuLogger::T_LOC_NAME = "t_loc";

const String LsuLogger::T_LOG_NAME = "t_log";

const String LsuLogger::EMPTY = "";

const int LsuLogger::MAX_LOGGER = 15;

void LsuLogger::writeLogger(const String &what)
{
    if (!LsuNtpTime::isTimeAvailable())
    {
        defere_log(what);
        return;
    }
    recal_log();
    String timeString = String(LsuNtpTime::timeString());
    String post_data;
    if (loc.length() > 0)
        post_data = timeString + " - [" + loc + "] " + what;
    else
        post_data = timeString + " - " + what;
    Serial.println(post_data);
    sendPostData(post_data);
}

void LsuLogger::defere_log(const String &what)
{
    if (deferred_index >= MAX_LOGGER)
    {
        ++deferred_index;
        // clean old last position
        delete deferred_log[MAX_LOGGER - 1];
        deferred_log[MAX_LOGGER - 1] = new String(
                String("Deferred log overflowed by ")
                        + (deferred_index - MAX_LOGGER) + "!");
        deferred_time[MAX_LOGGER - 1] = LsuNtpTime::secondsRunning();
        return;
    }
    deferred_log[deferred_index] = new String(what);
    deferred_time[deferred_index] = LsuNtpTime::secondsRunning();
    ++deferred_index;
}

void LsuLogger::recal_log()
{
    if (deferred_index <= 0)
        return;
    if (deferred_index >= MAX_LOGGER)
        deferred_index = MAX_LOGGER;
    for (int i = 0; i < deferred_index; i++)
    {
        time_t t_time = LsuNtpTime::getStartSecond() + deferred_time[i];
        String timeString = LsuNtpTime::timeString(day(t_time), month(t_time),
                year(t_time), hour(t_time), minute(t_time), second(t_time));
        String *t_df_log = deferred_log[i];
        String post_data;
        if (loc.length() > 0)
            post_data = timeString + " - [" + loc + "] " + *t_df_log;
        else
            post_data = timeString + " - " + *t_df_log;
        Serial.println(post_data);
        sendPostData(post_data);
        delete t_df_log;
        deferred_log[i] = 0;
    }
    deferred_index = 0;
}

void LsuLogger::sendPostData(const String &data)
{
    if (server == LsuWiFi::NO_IP || page.length() <= 0 || port == 0)
        return;
    String post_data = String(T_LOG_NAME) + "=" + data;
    if (loc.length() > 0)
        post_data = String(T_LOC_NAME) + "=" + loc + "&" + post_data;
    String post_req = String("") + "POST " + page + " HTTP/1.1\r\n" + "Host: "
            + WiFi.localIP().toString() + "\r\n" + "User-Agent: Arduino/1.0\r\n"
            + "Connection: close\r\n"
            + "Content-Type: application/x-www-form-urlencoded\r\n"
            + "Content-Length: " + post_data.length() + "\r\n";
    LsuWiFi::connect();
    WiFiClient client;
    if (client.connect(server, port))
    {
        client.println(post_req);
        client.println(post_data);
        delay(10);
        client.stop();
    }
}
