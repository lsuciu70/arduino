/* termometru */
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// the LCD pin's connection into Arduino
// LiquidCrystal(rs, enable, data4, data5, data6, data7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// running at 5 Volts
const float MAX_VOLT = 5.0f;

// measured voltage is between 0 and 1023
const int VOLT_SCALE = 1024;

// TMP36's resolution is 10 mV/C with a 500 mV
// offset to allow for negative temperatures
const int TEMP_RESOLUTION = 100;
const float TEMP_OFFSET = .5f;

// average divisor
const int AVG = 10;

// one second, 1000 milliseconds
const int SECOND = 1000;

// display precision, ± .5 degree
const float PRECISION = .5f;

// display refresh rate, seconds
const int REFRESH = 2;

// last read temperature
float currTemp = 0;

// name of degree centigrade sign for LCD
const byte GRAD = 0;

// degree centigrade sign on LCD
byte grad[8] = {
  B00100,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

// the setup routine runs once when starts
void setup() {
  lcd.createChar(GRAD, grad);
  lcd.begin(16,2);
}

// prints the temperature on LCD
void printTemp(float temp)
{
  lcd.clear();
  lcd.print("temp: ");
  lcd.print(temp);
  lcd.write(byte(GRAD));
  lcd.print("C");
}

// the loop routine runs over and over again forever
void loop() {
  float sum = 0;
//  for(int i = 0; i < AVG; ++i)
//  {
    int rd = analogRead(A1);
    // read voltage in mV, i.e: 153 -> 748 mV
    float volt = rd * MAX_VOLT / VOLT_SCALE;
    // read temperature
    float temp = (volt - TEMP_OFFSET) * TEMP_RESOLUTION;
//    sum += temp;
    // delay until next measurement
    delay(REFRESH * SECOND);
//  }
  // get the average
//  float temp_t = sum / AVG;
  // difference this measure vs previous one
//  float diff = currTemp - temp_t;
  // apply precision
//  if(diff > 0 && diff <= PRECISION)
//    return;
//  if(diff < 0 && diff >= -1 * PRECISION)
//    return;
  // save new current temperature
  currTemp = temp;
  // show it on LCD
  printTemp(currTemp);
}

/* */

