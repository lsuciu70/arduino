#include <Wire.h>
#include <LsuAlarmConstants.h>

int value;

bool isSet(byte);
void listen(int);

void setup()
{
    Serial.begin(115200);
    value = 0;
    // start Wire as slave
    Wire.begin(SLAVE_ADDR);
    Wire.onReceive(listen);
}

void loop()
{
}

bool isSet(int idx)
{
    int i = (int) pow(2, idx);
    return (value & i) == i;
}

void listen(int len)
{
    byte h = 0, l = 0;
    int b = 0;
    while(Wire.available())
    {
        h = l;
        if((b = Wire.read()) >= 0)
            l = b;
    }
    value = word(h, l);
}
