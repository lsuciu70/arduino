/*
 * LsuMqttClient.h
 *
 *  Created on: Sep 18, 2017
 *      Author: lsuciu
 */

#ifndef LSUMQTTCLIENT_H_
#define LSUMQTTCLIENT_H_

#include <Client.h>
#include <stdint.h>

#include "MqttClient.h"

#define MQTT_CALLBACK(name) void (*name)(char*, uint8_t*, unsigned int)
#define MQTT_ON_CONNECT(name) void (*name)(MqttClient&)

#define MQTT_CLIENT_ID_LEN 16

namespace
{

MqttClient client;

MQTT_ON_CONNECT(onConnectCallback);

char clientId[MQTT_CLIENT_ID_LEN + 1];

void connect()
{
  while (!client.connected())
  {
    if (client.connect(clientId))
    {
      if (onConnectCallback)
        onConnectCallback(client);
    }
  }
}

} // end anonymous namespace

namespace LsuMqtt
{

/**
 * Initialize the MQTT client
 * @param ip - the MQTT broker's IP or domain
 * @param port - the MQTT port, use 1883 for default
 * @param id - the MQTT client ID
 * @param wifiClient - the WiFi client, i.e. ESP8266's WiFiClient instance, see below
 * @param callback - the callback function pointer, see below
 * @param onConnect - the on connect function pointer, see below
 *
 * Usage:
 * #include <LsuWiFi.h>
 * #include "MqttClient.h"
 *
 * WiFiClient espClient; // the WiFi client
 * const char* broker = "192.168.100.60"; // the broker's IP (RaspPi)
 * uint16_t mqtt_port = 1883; // MQTT default port
 * const char* clientId = "MQTTClient"; // Client ID
 *
 * // the callback function
 * void callback(char* topic, byte* payload, unsigned int length)
 * {
 *   // called for subscription made
 * }
 *
 * void onConnect(MqttClient &client)
 * {
 *   // called after connection was established, good place to subscribe
 *   client.subscribe("topic_name");
 * }
 */
void init(const char* ip, uint16_t port, const char* id, Client& wifiClient,
    MQTT_CALLBACK(callback), MQTT_ON_CONNECT(onConnect))
{
  client.setServer(ip, port);
  client.setCallback(callback);
  client.setClient(wifiClient);
  onConnectCallback = onConnect;
  strncpy(clientId, id, MQTT_CLIENT_ID_LEN);
  clientId[MQTT_CLIENT_ID_LEN] = '\0';
}

bool publish(const char* topic, const char* payload)
{
  connect();
  return client.publish(topic, payload);
}

void loop()
{
  connect();
  client.loop();
}

} // end namespace LsuMqtt

#endif /* LSUMQTTCLIENT_H_ */
