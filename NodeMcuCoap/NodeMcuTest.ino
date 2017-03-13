#include <LsuWiFi.h>
#include <WiFiUdp.h>

#include "coap.h"

#define PORT 5683

//WiFiClient client;

WiFiUDP udp;

uint8_t packetbuf[256];

static uint8_t scratch_raw[32];

static coap_rw_buffer_t scratch_buf =
{ scratch_raw, sizeof(scratch_raw) };

void udp_send(const uint8_t*, int);

void setup()
{
  Serial.begin(115200);

  LsuWiFi::connect();

  udp.begin(PORT);

  coap_setup();
  endpoint_setup();
}

void loop()
{
  int sz;
  int rc;
  coap_packet_t pkt;
  int i;

  if ((sz = udp.parsePacket()) > 0)
  {
//    Serial.print("received ");
//    Serial.print(sz);
//    Serial.println(" bytes");

    udp.read(packetbuf, sizeof(packetbuf));

//    Serial.print("read packetbuf[");
//    Serial.print(sizeof(packetbuf));
//    Serial.print("]: ");
//    for (i = 0; i < sz; i++)
//    {
//      Serial.print(packetbuf[i], HEX);
//      Serial.print(" ");
//    }
//    Serial.println("");

    if (0 != (rc = coap_parse(&pkt, packetbuf, sz)))
    {
//      Serial.print("Bad packet rc=");
//      Serial.println(rc, DEC);
    }
    else
    {
      size_t rsplen = sizeof(packetbuf);
      coap_packet_t rsppkt;
      coap_handle_req(&scratch_buf, &pkt, &rsppkt);

      memset(packetbuf, 0, UDP_TX_PACKET_MAX_SIZE);
      if (0 != (rc = coap_build(packetbuf, &rsplen, &rsppkt)))
      {
//        Serial.print("coap_build failed rc=");
//        Serial.println(rc, DEC);
      }
      else
      {
        udp_send(packetbuf, rsplen);
      }
    }
  }
}

void udp_send(const uint8_t* buf, int buflen)
{
  udp.beginPacket(udp.remoteIP(), udp.remotePort());
  while (buflen--)
    udp.write(*buf++);
  udp.endPacket();
}

