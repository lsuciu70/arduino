#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

void setup() {
  Serial.begin(115200);

  // initialize the lcd
  lcd.begin(20, 4);
  lcd.setBacklight(HIGH);
  // go home
  lcd.home();
  lcd.print("Hello, ARDUINO ");
  // go to the next line
  lcd.setCursor ( 0, 1 );
  lcd.print(" WORLD!  ");

  Serial.println("Mere?");
}

void loop() {
}

