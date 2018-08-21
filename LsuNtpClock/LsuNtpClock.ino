#include <Arduino.h>
#include <WiFiUdp.h>

#include <LiquidCrystal_I2C.h> // https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/NewliquidCrystal_1.3.4.zip

#include <LsuNtpTime.h>   // https://github.com/lsuciu70/arduino/tree/master/libraries/LsuNtpTime

const byte LCD_LINES = 4;
const byte LCD_CHARS_PER_LINE = 20;

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

WiFiUDP udp;

void showTimeOnLcd();

void setup()
{
	Serial.begin(115200);

	// initialize the lcd
	lcd.begin(LCD_CHARS_PER_LINE, LCD_LINES);
	lcd.setBacklight(HIGH);
	lcd.clear();

	// initialize and start the NTP library
	LsuNtpTime::init(udp);
	LsuNtpTime::start(6 * 3600);
}

void loop()
{
	showTimeOnLcd();
	delay(LsuNtpTime::SECOND);
}

void showTimeOnLcd()
{
	lcd.setCursor(0, 0);
    // put date and time on LCD if fits
    if (LCD_CHARS_PER_LINE >= LsuNtpTime::datetimeStringLength)
    {
        String datetimeStr = LsuNtpTime::datetimeString();
        lcd.print(datetimeStr);
        Serial.println(datetimeStr);
    }
    else if (LCD_CHARS_PER_LINE >= LsuNtpTime::dateStringLength)
    {
        if (LCD_LINES >= 2)
        {
            String dateStr = LsuNtpTime::dateString();
            lcd.print(dateStr);
            // move to second row
            lcd.setCursor(0, 1);
            String timeStr = LsuNtpTime::timeString();
            lcd.print(timeStr);
            Serial.println(
                    String(dateStr) + " " + timeStr + " (two rows on LCD!");
        }
        else
        {
            String timeStr = LsuNtpTime::timeString();
            lcd.print(timeStr);
            Serial.println(
                    String(LsuNtpTime::datetimeString())
                            + " (date does not fit on LCD!)");

        }
    }
    else if (LCD_CHARS_PER_LINE >= LsuNtpTime::timeStringLength)
    {
        String timeStr = LsuNtpTime::timeString();
        lcd.print(timeStr);
        Serial.println(
                String(LsuNtpTime::datetimeString())
                        + " (date does not fit on LCD!)");
    }
    else
    {
        Serial.println(
                String(LsuNtpTime::datetimeString())
                        + " (does not fit on LCD!)");
    }
}

