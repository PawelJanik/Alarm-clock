#include <Arduino.h>
#include <avr/sleep.h>
#include "prescaler.h"

#define SENSE_EXT_V_PIN A2
#define SENSE_BAT_V_PIN A3
#define SENSE_LIGHT_PIN A1

#define ANALOG_RATIO 3.22	//----for 3.3V
//#define ANALOG_RATIO 4.88	//----for 5V

unsigned long timer200ms = 0;
unsigned long timer5s = 0;
unsigned long timer1m = 0;

int16_t lastLightValue = 0;
unsigned long lightTimer = 0;

void setup()
{
	setClockPrescaler(CLOCK_PRESCALER_8);	//-----------------------------------------------------------
	pinMode(3, OUTPUT);

	pinMode(SENSE_EXT_V_PIN, INPUT);
	pinMode(SENSE_BAT_V_PIN, INPUT);

	Serial.begin(9600);
}

void loop()
{
	if((millis() - timer200ms) > 200)
	{
		timer200ms = millis();

		int16_t lightValue = analogRead(SENSE_LIGHT_PIN);
		if(((lightValue - 10) > lastLightValue) || ((lightValue + 10) < lastLightValue))
		{
			Serial.print("-LIG:");
			Serial.println(lightValue);
			lastLightValue = lightValue;
		}
		else if((millis() - lightTimer) > 1000)
		{
			lightTimer = millis();

			Serial.print("-LIG:");
			Serial.println(lightValue);
			lastLightValue = lightValue;
		}
	}

	if((millis() - timer5s) > 1000*5)	//----------------------------to changed
	{
		timer5s = millis();

		Serial.print("-EXT:");
		Serial.println(analogRead(SENSE_EXT_V_PIN)*(double)ANALOG_RATIO/500);
	}

	if((millis() - timer1m) > 10000*60)
	{
		timer1m = millis();

		Serial.print("-BAT:");
		Serial.println(analogRead(SENSE_BAT_V_PIN)*(double)ANALOG_RATIO/500);
	}
}
