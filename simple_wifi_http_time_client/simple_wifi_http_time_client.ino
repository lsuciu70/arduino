/*
    Simple HTTP get webclient test
*/

#include <ESP8266WiFi.h>
#include <time.h>

const char* ssid     = "lsu-phone";
const char* password = "r4cD7TPG";

const char* host = "www.google.ro";

void setup() {
  Serial.begin(115200);
  delay(100);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  Serial.print("Getting time from: ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  unsigned long time = 0;
  const char* request = "HEAD / HTTP/1.0\r\n\r\n";
  // Make an HTTP 1.1 head request
  client.print(request);
  char buf[5];      // temporary buffer for characters
  client.setTimeout(5000);
  // look for Date: header
  if (client.find((char *)"\r\nDate: "))
  {
    client.readBytes(buf, 5); // day name
    buf[5] = '\0';
    Serial.print(buf);

    unsigned day = client.parseInt();     // day
    client.readBytes(buf, 1);    // discard one
    client.readBytes(buf, 3);    // month
    buf[3] = '\0';
    int year = client.parseInt();    // year

    // print date
    Serial.print(day); Serial.print(" ");
    Serial.print(buf); Serial.print(" ");
    Serial.println(year);

    byte hour = client.parseInt();   // hour
    byte minute = client.parseInt(); // minute
    byte second = client.parseInt(); // second

    // print time
    if (hour < 10) Serial.print("0");
    Serial.print(hour); Serial.print(":");
    if (minute < 10) Serial.print("0");
    Serial.print(minute); Serial.print(":");
    if (second < 10) Serial.print("0");
    Serial.print(second); Serial.print(" ");

    client.readBytes(buf, 1);    // discard one
    client.readBytes(buf, 3);    // GMT
    buf[3] = '\0';
    Serial.println(buf);

    int daysInPrevMonths;
    switch (buf[0])
    {
      case 'F': daysInPrevMonths =  31; break; // Feb
      case 'S': daysInPrevMonths = 243; break; // Sep
      case 'O': daysInPrevMonths = 273; break; // Oct
      case 'N': daysInPrevMonths = 304; break; // Nov
      case 'D': daysInPrevMonths = 334; break; // Dec
      default:
        if (buf[0] == 'J' && buf[1] == 'a')
          daysInPrevMonths = 0;   // Jan
        else if (buf[0] == 'A' && buf[1] == 'p')
          daysInPrevMonths = 90;    // Apr
        else switch (buf[2])
          {
            case 'r': daysInPrevMonths =  59; break; // Mar
            case 'y': daysInPrevMonths = 120; break; // May
            case 'n': daysInPrevMonths = 151; break; // Jun
            case 'l': daysInPrevMonths = 181; break; // Jul
            default: // add a default label here to avoid compiler warning
            case 'g': daysInPrevMonths = 212; break; // Aug
          }
    }

    // This code will not work after February 2100
    // because it does not account for 2100 not being a leap year and because
    // we use the day variable as accumulator, which would overflow in 2149
    day += (year - 1970) * 365; // days from 1970 to the whole past year
    day += (year - 1969) >> 2;  // plus one day per leap year
    day += daysInPrevMonths;  // plus days for previous months this year
    if (daysInPrevMonths >= 59  // if we are past February
        && ((year & 3) == 0)) // and this is a leap year
      day += 1;     // add one day
    // Remove today, add hours, minutes and seconds this month
    time = (((day - 1ul) * 24 + hour) * 60 + minute) * 60 + second;
  }
  Serial.print("Unix time: ");
  Serial.println(time);
  struct tm * timeinfo;
  time_t rawtime(time);
  timeinfo = localtime (&rawtime);
//  Serial.println(asctime(timeinfo));
  delay(10);
  client.flush();
  client.stop();
  delay(10000); // 10 seconds
}
