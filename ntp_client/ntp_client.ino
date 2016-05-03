#include <WiFiUdp.h>



unsigned long getTime();

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  getTime();
  delay(10000);
}

unsigned long getTime()
{
  // thanks to (http) playground.arduino.cc/Code/NTPclient
  static WiFiUDP udp;
  static int udpInitialized = udp.begin(123); // Initializes the WiFi UDP library and network settings. Starts WiFiUDP socket, listening at local port 123
  if (0 == udpInitialized) // returns 0 if there are no sockets available to use
    return 0;
  static char timeServer[] = "ro.pool.ntp.org";  // the NTP server
  static long ntpFirstFourBytes = 0xEC0600E3; // the NTP request header

  udp.flush(); // Clear received data from possible stray received packets

  // Send an NTP request to timeserver on NTP port: 123
  if (! (udp.beginPacket(timeServer, 123)
      && udp.write((byte *)&ntpFirstFourBytes, 48) == 48
      && udp.endPacket()))
    return 0; // sending request failed

  // Wait for response; check every pollIntv ms up to maxPoll times
  const int pollIntv = 150; // poll every this many ms
  const byte maxPoll = 15;  // poll up to this many times
  int pktLen;               // received packet length
  for (byte i=0; i<maxPoll; i++) {
    if ((pktLen = udp.parsePacket()) == 48)
      break;
    delay(pollIntv);
  }
  if (pktLen != 48)
    return 0; // no correct packet received

  // Read and discard the first useless bytes
  // Set useless to 32 for speed; set to 40 for accuracy.
  const byte useless = 40;
  for (byte i = 0; i < useless; ++i)
    udp.read();

  // Read the integer part of sending time
  unsigned long time = udp.read();  // NTP time
  for (byte i = 1; i < 4; i++)
    time = time << 8 | udp.read();

  // Round to the nearest second if we want accuracy
  // The fractionary part is the next byte divided by 256: if it is
  // greater than 500ms we round to the next second; we also account
  // for an assumed network delay of 50ms, and (0.5-0.05)*256=115;
  // additionally, we account for how much we delayed reading the packet
  // since its arrival, which we assume on average to be pollIntv/2.
  time += (udp.read() > 115 - pollIntv/8);

  // Discard the rest of the packet
  udp.flush();

  return time - 2208988800ul;   // convert NTP time to Unix time
}

