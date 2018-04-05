#include <Arduino.h>

#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <LsuWiFi.h>
#include <LsuNtpTime.h>

const int SECOND = 1000;

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
const byte LCD_LINES = 2;
const byte LCD_CHARS_PER_LINE = 16;

char datetimebuff[LCD_CHARS_PER_LINE + 1];
char temphumidbuff[LCD_CHARS_PER_LINE + 1];

const uint8_t DHTTYPE = DHT22;     // DHT 22 (AM2302)
const uint8_t DHTPIN = 2;

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
uint32_t lastRead;

float temperature, humidity;

byte percent [8] = {
  0b11001,
  0b11010,
  0b00010,
  0b00100,
  0b00100,
  0b01000,
  0b01011,
  0b10011
};

int floatToRound05Int(float temp)
{
  int t = (int) (temp * 100);
  uint8_t mod = t % 10;
  switch (mod)
  {
  case 1:
  case 2:
    t += (0 - mod);
    break;
  case 3:
  case 4:
  case 6:
  case 7:
    t += (5 - mod);
    break;
  case 8:
  case 9:
    t += (10 - mod);
    break;
  default:
    break;
  }
  return t;
}

char* temphumidString(char* buff)
{
  if(temperature < 1 || humidity < 1)
  {
    sprintf(buff, "%s", "Masuri eronate");
    return buff;
  }
  int t = floatToRound05Int(temperature);
  int t_d = t % 100, t_i = (t - t_d) / 100;
  int h = floatToRound05Int(humidity);
  int h_d = h % 100, h_i = (h - h_d) / 100;
  sprintf(buff, "%02d.%02d %cC %02d.%02d %c", t_i, t_d, (char) 0xDF, h_i, h_d, (char) 0x25);
  return buff;
}

void display()
{
  char* datetime = 0;
  if(LCD_CHARS_PER_LINE >= LsuNtpTime::DMY_HMS_STR_LEN)
  	datetime = LsuNtpTime::datetimeString(datetimebuff);
  else if(LCD_CHARS_PER_LINE >= LsuNtpTime::HMS_STR_LEN)
  	datetime = LsuNtpTime::timeString(datetimebuff);
  else
  {
  	Serial.println("Unsupported chars per line");
  	return;
  }
  lcd.setCursor(0, 0);
  lcd.print(datetime);
  char* temphumid = temphumidString(temphumidbuff);
  lcd.setCursor(0, 1);
  lcd.print(temphumid);
  Serial.println(datetime);
  Serial.println(temphumid);
}

void readTempHumid()
{
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature))
  {
    Serial.println("Wrong temperature");
    return;
  }
  temperature = event.temperature;
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity))
  {
    Serial.println("Wrong humidity");
    return;
  }
  humidity = event.relative_humidity;
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  LsuWiFi::connect();
  LsuNtpTime::begin();

  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
  lastRead = 0;
  temperature = 0;
  humidity = 0;

  lcd.begin(LCD_CHARS_PER_LINE, LCD_LINES);
  lcd.setBacklight(HIGH);
  lcd.clear();
  delay(SECOND);
}

void loop() {
  LsuWiFi::connect();
  if(lastRead == 0 || (millis() - lastRead) > delayMS) {
  	lastRead = millis();
  	readTempHumid();
  }
  display();
  delay(50);
}
