#include <ESP8266WiFi.h>
#include <LsuWiFi.h>

#include "MqttClient.h"

const char* broker = "192.168.100.11";
uint16_t port = 1883;
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient espClient;

MqttClient client(broker, port, callback, espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print(F("] "));
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "LsuMqttClient";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("mqtt", "hello world");
      // ... and resubscribe
      client.subscribe("mqtt");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  LsuWiFi::connect();
  if(!client.loop())
    reconnect();

  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    Serial.print(F("Publish message: "));
    Serial.println(msg);
    client.publish("mqtt", msg);
  }
}
