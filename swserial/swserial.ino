/*
  Software serial multple serial test

  Receives from the hardware serial, sends to software serial.
  Receives from software serial, sends to hardware serial.

  The circuit:
   RX is digital pin 10 (connect to TX of other device)
   TX is digital pin 11 (connect to RX of other device)

  Note:
  Not all pins on the Mega and Mega 2560 support change interrupts,
  so only the following can be used for RX:
  10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

  Not all pins on the Leonardo support change interrupts,
  so only the following can be used for RX:
  8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

  created back in the mists of time
  modified 25 May 2012
  by Tom Igoe
  based on Mikal Hart's example

  This example code is in the public domain.

*/
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

// Supported baud rates are 300, 600, 1200, 2400, 4800,
// 9600, 14400, 19200, 28800, 31250, 38400, 57600, and 115200.
#define BAUD_RATE 115200

/* 
 Messages:
 - AT
 - AT+RST = reset
 - AT+GMR = output the firmware revision number
 */
 
 void setup()
 {
  // Open serial communications and wait for port to open:
  Serial.begin(BAUD_RATE);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("USB serial ready");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(BAUD_RATE);
}

void write()
{
  // HW Serial -> SW Serial
  String message = "";
  if (Serial.available())
  {
    int c;
    while ((c = Serial.read()) >= 0)
    {
      message += (char) c;
      mySerial.write((char) c);
    }
    if (message.length() > 0)
      Serial.println("message: " + message);
  }
}

void read()
{
  String response = "";
  if (mySerial.available()) {
    int c;
    while ((c = mySerial.read()) >= 0)
    {
      response += (char) c;
    }
    if (response.length() > 0)
      Serial.println(response);
  }
}

void loop() { // run over and over
  String message = "", response = "";
  read();
  write();
}

