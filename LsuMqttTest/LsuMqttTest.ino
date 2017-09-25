#include <LsuWiFi.h>

#include "MqttClient.h"

#define DEBUG

const char* broker = "192.168.100.59";
//const char* broker = "92.81.33.180"; // lsuciu.no-ip.org
uint16_t mqtt_port = 1883;

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient espClient;

MqttClient client(broker, mqtt_port, callback, espClient);

long lastMsg = 0;
char msg[50];

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print(F("] "));
  for (int i = 0; i < length; i++)
  {
    Serial.print((char) payload[i]);
  }
  Serial.println();
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "MQTTClient";
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, subscribe
      client.subscribe("la_topic");
      // ... and publish an announcement
      client.publish("la_topic", "mere");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  LsuWiFi::connect(0, 10000);
  if (!client.connected())
    reconnect();
  client.loop();
//
//  long now = millis();
//  if (now - lastMsg > 10000)
//  {
//    lastMsg = now;
//    int value = analogRead(A0);
//    value = map(value, 265, 350, 0, 100);
//    snprintf(msg, 75, "level %d", value);
//    Serial.print(F("Publish message: "));
//    Serial.println(msg);
//    client.publish("mqtt", msg);
//  }
}
