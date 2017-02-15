#include <Arduino.h>

#include <SPI.h>
#include <Ethernet.h>

#include <SparkFunBME280.h> // https://github.com/sparkfun/SparkFun_BME280_Arduino_Library

#include <LsuBME280.h>
#include <LsuScheduler.h>

// one second, 1000 milliseconds
const int SECOND = 1000;

void ethernetMaintain();

void httpListen();

void httpResponse(EthernetClient &client);

void readSensor();

void serialPrint();

float temperature;

float pressure;

float altitude;

float humidity;

LsuScheduler scheduler;

LsuBME280 sensor;

EthernetServer server(80);

bool etherOk;

uint8_t MAC[] =
{ 0x12, 0x46, 0x06, 0x0C, 0x00, 0x14 };

void setup()
{
    Serial.begin(57600);
    Serial.println("setup()");

    sensor.start();
    scheduler.add(readSensor, millis() + SECOND);

    Serial.flush();
    pinMode(10, OUTPUT);

	SPI.begin();
    etherOk = Ethernet.begin(MAC) != 0;
    if (etherOk)
    {
        Serial.print("Got IP address: ");
        Serial.println(Ethernet.localIP());
	    scheduler.add(ethernetMaintain, millis() + SECOND);
    Serial.flush();
	    server.begin();
    }
    else
        Serial.println("Ethernet connection failed!");
}

void loop()
{
    scheduler.execute(millis());
    httpListen();
}

void readSensor()
{
    temperature = sensor.readTempC();
    pressure = sensor.readFloatPressure();
    altitude = sensor.readFloatAltitudeMeters();
    humidity = sensor.readFloatHumidity();

    serialPrint();

    scheduler.add(readSensor, millis() + 10 * SECOND);
}

void serialPrint()
{
    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.println(F(" degrees C"));

    Serial.print(F("Pressure: "));
    Serial.print(pressure);
    Serial.println(F(" Pa"));

    Serial.print(F("Altitude: "));
    Serial.print(altitude);
    Serial.println(F("m"));

    Serial.print(F("%RH: "));
    Serial.print(humidity);
    Serial.println(F(" %"));

    Serial.println();
}

void httpListen()
{
    if (!etherOk)
        return;
    EthernetClient client = server.available();
    if (client)
    {
        // an http request ends with a blank line
        boolean currentLineIsBlank = true;
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                Serial.write(c);
                // if you've gotten to the end of the line (received a newline
                // character) and the line is blank, the http request has ended,
                // so you can send a reply
                if (c == '\n' && currentLineIsBlank)
                {
                    // send the http response
                    httpResponse(client);
                    break;
                }
                if (c == '\n')
                {
                    // you're starting a new line
                    currentLineIsBlank = true;
                }
                else if (c != '\r')
                {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }
        }
        // give the web browser time to receive the data
        delay(5);
        // close the connection
        client.stop();
    }
}

void httpResponse(EthernetClient &client)
{
    client.println(F("HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\nRefresh: 10\r\n"));
    client.println(F("<!DOCTYPE html>\n<html><head>\n<title>CLLS Weather</title>\n</head>\n<body>\n"));
    client.print(F("<p>Temperatura: "));
    client.print(temperature);
    client.print(F(" &deg;C</p>\n"));
    client.println();
    client.print(F("<p>Presiunea: "));
    client.print(pressure);
    client.print(F(" Pa</p>\n"));
    client.println();
    client.print(F("<p>Umiditatea: "));
    client.print(humidity);
    client.print(F(" %</p>\n"));
    client.println();
    client.print(F("<p>Altitudinea: "));
    client.print(altitude);
    client.print(F(" m</p>\n"));
    client.println();
    client.println(F("</body>\n</html>\n"));
}

void ethernetMaintain()
{
    /*
     * Ethernet.maintain() returns
     * 0: nothing happened
     * 1: renew failed
     * 2: renew success
     * 3: rebind fail
     * 4: rebind success
     */
    int mntRes = Ethernet.maintain() != 0;
    if (mntRes == 2)
    {
        Serial.print("Got IP address: ");
        Serial.println(Ethernet.localIP());
    }
    etherOk = mntRes == 0 || mntRes == 2 || mntRes == 4;
    scheduler.add(ethernetMaintain, millis() + SECOND);
}
