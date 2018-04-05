#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Set the LCD I2C address

// 0
byte c000[8] =
{ 0b00111, 0b00111, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000 };
byte c001[8] =
{ 0b11100, 0b11100, 0b00011, 0b00011, 0b00011, 0b00011, 0b00011, 0b00011 };
byte c010[8] =
{ 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11100, 0b00111, 0b00111 };
byte c011[8] =
{ 0b00011, 0b00011, 0b00011, 0b00011, 0b00011, 0b00011, 0b11100, 0b11100 };

// 1
byte c100[8] =
{ 0b00001, 0b00001, 0b00011, 0b00111, 0b00101, 0b00001, 0b00001, 0b00001 };
byte c101[8] =
{ 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000 };
byte c110[8] =
{ 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00111, 0b00111 };
byte c111[8] =
{ 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11100, 0b11100 };

//2
byte c200[8] =
{ 0b00111, 0b00111, 0b11000, 0b11000, 0b00000, 0b00000, 0b00000, 0b00000 };
byte c201[8] =
{ 0b11100, 0b11100, 0b00011, 0b00011, 0b00011, 0b00011, 0b01100, 0b01100 };
byte c210[8] =
{ 0b00001, 0b00001, 0b00110, 0b00110, 0b11000, 0b11000, 0b11111, 0b11111 };
byte c211[8] =
{ 0b10000, 0b10000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111 };

void write(uint8_t x, uint8_t n)
{
  switch (n)
  {
  case 0:
    lcd.createChar(0, c000);
    lcd.createChar(1, c001);
    lcd.createChar(2, c010);
    lcd.createChar(3, c011);
    // go to the 1st line (0), xth position
    lcd.setCursor(2*x, 0);
    lcd.write((uint8_t) 0);
    lcd.write((uint8_t) 1);
    // go to the 2nd line (1), xth position
    lcd.setCursor(2*x, 1);
    lcd.write((uint8_t) 2);
    lcd.write((uint8_t) 3);
    break;
  case 1:
    lcd.createChar(4, c100);
    lcd.createChar(5, c101);
    lcd.createChar(6, c110);
    lcd.createChar(7, c111);
  // go to the 1st line (0), xth position
    lcd.setCursor(2*x, 0);
    lcd.write((uint8_t) 4);
    lcd.write((uint8_t) 5);
    // go to the 2nd line (1), xth position
    lcd.setCursor(2*x, 1);
    lcd.write((uint8_t) 6);
    lcd.write((uint8_t) 7);
    break;
  default:
    return;
  }
}

void setup()
{

  // initialize the lcd
  lcd.begin(20, 4);
 // lcd.begin(16, 2);
  lcd.setBacklight(HIGH);
  // go home
  lcd.clear();
  // go home
  lcd.home();
  write(0, 0);
  write(1, 1);
}

void loop()
{
}
