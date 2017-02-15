#include <Arduino.h>
#include <Wire.h>
#include <LsuAlarmConstants.h>

int readAllPins();
void sendValue(int);

void setup()
{
    // join i2c bus (address optional for master)
    Wire.begin();

    // sets the digital pins as input
    for (int i = 0; i < PINS_LEN; ++i)
    {
        pinMode(PINS[i], INPUT);
    }
}

void loop()
{
    sendValue(readAllPins());
}

int readAllPins()
{
    int val = 0;
    // read all pins
    for (int i = 0; i < PINS_LEN; ++i)
    {
        if(digitalRead(PINS[i]) == HIGH)
            val |= ALARMS[i];
    }
    return val;
}

void sendValue(int val)
{
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(highByte(val));
    Wire.write(lowByte(val));
    Wire.endTransmission();
    delay(5);
}

void received(int val)
{
    // check alarm is set
    for (int i = 0; i < PINS_LEN; ++i)
    {
        bool is_i_set = (val & ALARMS[i]) == ALARMS[i];
    }
}

void onReceiveImpl(int len)
{
    byte h = 0, l = 0;
    int b = 0;
    while(Wire.available())
    {
        h = l;
        if((b = Wire.read()) >= 0)
            l = b;
    }
    received(word(h, l));
}
