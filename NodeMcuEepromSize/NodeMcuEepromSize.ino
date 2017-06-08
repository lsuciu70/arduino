#include <EEPROM.h>

typedef struct prog_s
{
  uint16_t mow;
  uint8_t zone;
  uint8_t time;
  uint8_t skip;
} __attribute__((packed)) programm;

uint16_t eepromWrite(uint16_t addr, const char* value)
{
  uint8_t i = 0;
  char c;
  while((c = *(value + (i++))))
    EEPROM.write(addr++, c);
  EEPROM.write(addr++, 0);
  return addr;
}
/**
 * Reads the EEPROM starting at given address into given char* up to and
 * including terminating 0. Returns the next position; the one after 0.
 */
uint16_t eepromRead(uint16_t addr, char* value, uint16_t size)
{
  uint16_t addr0 = addr;
  char c;
  while((c = EEPROM.read(addr)))
  {
    if((addr - addr0) < size)
      *(value + (addr - addr0)) = c;
    else
      ++addr0;
    ++addr;
  }
  *(value + (addr - addr0)) = '\0';
  return ++addr;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  EEPROM.begin(4096);
  Serial.println();
  Serial.println(EEPROM.size());
  Serial.println(sizeof(programm));

  uint16_t addr = 128;
  addr = eepromWrite(addr, "u_name");
  addr = eepromWrite(addr, "u_pwd");
  addr = eepromWrite(addr, "ap1_name");
  addr = eepromWrite(addr, "ap1_pwd");

  addr = 128;
  char rd[8];
  addr = eepromRead(addr, rd, 10);
  Serial.println(rd);
  addr = eepromRead(addr, rd, 10);
  Serial.println(rd);
  addr = eepromRead(addr, rd, 10);
  Serial.println(rd);
  addr = eepromRead(addr, rd, 10);
  Serial.println(rd);
}

void loop() {
  // put your main code here, to run repeatedly:

}
