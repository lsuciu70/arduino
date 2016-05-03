/* termometru */
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

#define ONE_WIRE_BUS 2
#define DEBUG true

// one second, 1000 milliseconds
const int SECOND = 1000;

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature tempSensor(&oneWire);

// Make RX Arduino line is pin 10, make TX Arduino line is pin 11.
// Connect the TX line from the esp to the Arduino's pin 10 (RX)
// and the RX line from the esp to the Arduino's pin 11 (TX)
SoftwareSerial wifi(10, 11);

// last read temperature
float currTemp = 0;

// split of a second
int split = 10;

// step, SECOND / split
int step = SECOND / split;

// read temperature counter
int count = 0;

char IPD[] = "+IPD,";

// the setup routine runs once when starts
void setup() {
  Serial.begin(9600);

  wifi.begin(76800);
  wifi.setTimeout(5*SECOND);
/*V2*/
  sendData("AT+RST\r\n",10000,DEBUG); // reset module
  if(sendData("AT+GMR\r\n",3000,DEBUG).length() <= 0)
  {
    Serial.println("no response");
    return;
  }
/* V1 */
//  // Check module is ready
//  Serial.println("send AT+RST");
//  delay(10);
//  wifi.println("AT+RST");
//  delay(SECOND);
//  if(wifi.find("ready"))
//  {
//    Serial.println(" OK");
//  }
//  else
//  {
//    Serial.println(" NOK");
//    // while(1);
//  }
/* V0 */
  sendData("AT+RST\r\n",2000,DEBUG); // reset module
  sendData("AT+CWMODE=2\r\n",1000,DEBUG); // configure as access point
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80

  // Start up the library
  tempSensor.begin();
  tempSensor.setResolution(12);
}

// prints the temperature on serial
void printTemp(float temp)
{
  Serial.println(temp);
}

float readTemperature(DallasTemperature& tempSensor, int index = 0) {
  // Send the command to get temperatures
  tempSensor.requestTemperatures();
  // Return the measured value
  return tempSensor.getTempCByIndex(index);
}

// the loop routine runs over and over again forever
void loop() {
  if(wifi.available()) // check if the esp is sending a message 
  {
    Serial.println("wifi.available() == true");
    if(wifi.find(IPD))
    {
      delay(1000);
      int connectionId = wifi.read()-48; // subtract 48 because the read() function returns 
                                           // the ASCII decimal value and 0 (the first decimal number) starts at 48
      currTemp = readTemperature(tempSensor);
      // show current temperature
      printTemp(currTemp);
      String webpage = "<h1>Temp:"; webpage += currTemp; webpage += "</h1>";
      String cipSend = "AT+CIPSEND="; cipSend += connectionId; cipSend += ","; cipSend += webpage.length(); cipSend += "\r\n";
      sendData(cipSend,1000,DEBUG);
      sendData(webpage,1000,DEBUG);
      String closeCommand = "AT+CIPCLOSE="; closeCommand += connectionId; closeCommand += "\r\n";
      sendData(closeCommand,3000,DEBUG);
    }
  }
  else
  {
    Serial.println("wifi.available() == false");
    delay(2000);
  }
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    Serial.print("command: " + command);
    wifi.print(command); // send the read character to the esp8266
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(wifi.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = wifi.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.println("response: " + response);
    }
    
    return response;
}
/* */

