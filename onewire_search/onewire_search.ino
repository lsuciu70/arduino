#include <OneWire.h>

// DS18B20 temperature senzor family code
#define DS18B20 0x28

// Arduino pin 7
#define ONE_WIRE_BUS 7

typedef uint8_t OneWireAddress[8];

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

void setup() {
  Serial.begin(9600);

  OneWireAddress address;
  oneWire.reset_search();
  while (oneWire.search(address))
  {
    switch (address[0]) {
      case DS18B20:
        {
          Serial.print("const uint8_t ADDRESS [] = {0x");
          Serial.print(address[0], HEX); Serial.print(", 0x");
          Serial.print(address[1], HEX); Serial.print(", 0x");
          Serial.print(address[2], HEX); Serial.print(", 0x");
          Serial.print(address[3], HEX); Serial.print(", 0x");
          Serial.print(address[4], HEX); Serial.print(", 0x");
          Serial.print(address[5], HEX); Serial.print(", 0x");
          Serial.print(address[6], HEX); Serial.print(", 0x");
          Serial.print(address[7], HEX); Serial.println("}");
          if (oneWire.crc8(address, 7) == address[7])
            Serial.println("CRC OK");
          else
            Serial.println("CRC NOK");
          break;
        }
      default:
        {
          Serial.print("unknown family code: 0x");
          Serial.println(address[0], HEX);
          break;
        }
    }
  }
}

void loop() {
}
