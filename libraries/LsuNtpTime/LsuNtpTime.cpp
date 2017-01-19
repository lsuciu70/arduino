/*
 * LsuNtpTime.cpp
 *
 *  Created on: Jan 19, 2017
 *      Author: lsuciu
 */

#include "LsuNtpTime.h"

const int LsuNtpTime::SECOND = 1000;
bool LsuNtpTime::timeAvailable = false;
unsigned long LsuNtpTime::startSecond = 0;

void LsuNtpTime::start(time_t seconds)
{
    // Set the external time provider
    setSyncProvider(getTime);
    // Set synch interval to one second till sync
    setSyncInterval(1);
    while (timeStatus() != timeSet)
    {
        ;
    }
    // Set synch interval
    setSyncInterval(seconds);
}

char * LsuNtpTime::timeString(int day_t, int month_t, int year_t, int hour_t,
        int minute_t, int second_t)
{
    static char buff[20];
    sprintf(buff, "%02d-%02d-%04d %02d:%02d:%02d", day_t, month_t, year_t,
            hour_t, minute_t, second_t);
    return buff;
}

time_t getTime()
{
    LsuWiFi::connect();
    WiFiUDP udp;
    const byte POLL_INTERVAL = 10; // poll every this many ms
    const byte POLL_TIMES = 100;  // poll up to this many times
    const byte PKT_LEN = 48; // NTP packet length
    const byte USELES_BYTES = 40; // Useless bytes to be discarded; set useless to 32 for speed; set to 40 for accuracy.

    // Eastern European Time (Timisoara)
    const TimeChangeRule EEST =
    { "EEST", Last, Sun, Mar, 3, 180 }; // Eastern European Summer Time
    const TimeChangeRule EET =
    { "EET", Last, Sun, Oct, 4, 120 }; // Eastern European Standard Time
    Timezone EasternEuropeanTime(EEST, EET);

    TimeChangeRule *tcr;

    {
        // if fails (returns 0 in between) timeAvailable will be kept as is
        LsuNtpTime::NoTimeAvailableSetter(
                LsuNtpTime::timeAvailable ? true : false);
        unsigned long mllis = millis();

        // Initializes the WiFi UDP library and network settings. Starts WiFiUDP socket, listening at local port 12670
        static int udpInitialized = udp.begin(12670);
        if (0 == udpInitialized) // returns 0 if there are no sockets available to use
        {
            Serial.println("ERROR: there are no sockets available to use.");
            return 0;
        }
        static char timeServer[] = "ro.pool.ntp.org";  // the NTP server
        static long ntpFirstFourBytes = 0xEC0600E3; // the NTP request header

        udp.flush(); // Clear received data from possible stray received packets

        // Send an NTP request to timeserver on NTP port: 123
        if (!(udp.beginPacket(timeServer, 123)
                && udp.write((byte *) &ntpFirstFourBytes, PKT_LEN) == PKT_LEN
                && udp.endPacket()))
        {
            Serial.println("NTP ERROR: sending request failed");
            return 0; // sending request failed
        }

        int pktLen;               // received packet length
        // Wait for NTP server response; check every POLL_INTERVAL ms up to POLL_TIMES times
        byte j = 0;
        for (; j < POLL_TIMES; j++)
        {
            if ((pktLen = udp.parsePacket()) == PKT_LEN)
                break;
            delay(POLL_INTERVAL);
        }
        if (pktLen != PKT_LEN)
        {
            Serial.println();
            Serial.print("NTP ERROR: no correct packet received; pktLen = ");
            Serial.print(pktLen);
            Serial.println(", expected 48");
            return 0; // no correct packet received
        }

        // Read and discard the first useless bytes
        for (byte i = 0; i < USELES_BYTES; ++i)
            udp.read();
    }
    // timeAvailable will be set to true immediately after return
    LsuNtpTime::NoTimeAvailableSetter(true);
    // Read the integer part of sending time
    time_t t_time = udp.read();  // NTP time
    for (byte i = 1; i < 4; i++)
        t_time = t_time << 8 | udp.read();

    // Round to the nearest second if we want accuracy
    // The fractionary part is the next byte divided by 256: if it is
    // greater than 500ms we round to the next second; we also account
    // for an assumed network delay of 30ms, and (0.5-0.05)*256=120;
    // additionally, we account for how much we delayed reading the packet
    // since its arrival, which we assume on average to be POLL_INTERVAL/2.
    t_time += (udp.read() > 120 - POLL_INTERVAL / 8);

    // Discard the rest of the packet
    udp.flush();
    //  WiFi.disconnect();

    // convert NTP time to GMT time
    t_time -= 2208988800ul;
    // convert Unix to locale (EET | EEST)
    t_time = EasternEuropeanTime.toLocal(t_time, &tcr);
    // set offset
    if (LsuNtpTime::startSecond == 0)
        LsuNtpTime::startSecond = t_time - LsuNtpTime::secondsRunning();
    return t_time;
}
