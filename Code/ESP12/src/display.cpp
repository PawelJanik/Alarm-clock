#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
  #include "WConstants.h"
  #include <Adafruit_NeoPixel.h>
#endif

#include "display.h"

Display::Display(Adafruit_NeoPixel * Pixel)
{
	pixel = Pixel;

	pixel->begin();
	pixel->show();

	colonState = false;
	colonAnimation = false;

	displayState.numbers = true;
	displayState.colon = true;
	displayState.alarm = true;
	displayState.temperatureMarkers = true;
}

void Display::displayClear()
{
	pixel->clear();
}

void Display::setBrightness(uint8_t brightness)
{
	pixel->setBrightness(brightness);
	pixel->show();
}

void Display::fillColor(uint32_t color)
{
	pixel->fill(color, 0, 33);
	pixel->show();
}

void Display::setColor(uint32_t color)
{
	numberColor = color;
	pixel->show();
}

void Display::setTempInColor(uint32_t color)
{
	if(displayState.temperatureMarkers == false)
	{
		return;
	}

	pixel->setPixelColor(32, color);
	pixel->show();
}

void Display::setTempOutColor(uint32_t color)
{
	if(displayState.temperatureMarkers == false)
	{
		return;
	}

	pixel->setPixelColor(31, color);
	pixel->show();
}

void Display::setNumber(int8_t number, uint8_t charNumber)
{
	uint8_t shiftValue = 0;

	switch(charNumber)
	{
		case 1: shiftValue = 7;break;
		case 2:	shiftValue = 16;break;
		case 3:	shiftValue = 23;break;
	}
	if(number < 0)
	{
		for(uint8_t i = 0; i < 7; i++)
		{
			pixel->setPixelColor(i + shiftValue, 0);
		}
	}
	else
	{
		for(uint8_t i = 0; i < 7; i++)
		{
			if(signs[number][i] == true)
			{
				pixel->setPixelColor(i + shiftValue, numberColor);
			}
			else
			{
				pixel->setPixelColor(i + shiftValue, 0);
			}
		}
	}

	pixel->show();
}

void Display::setAnimatedColon(bool onOff)
{
	colonAnimation = true;
}

void Display::setTime(uint8_t hh, uint8_t mm)
{
	if(displayState.numbers == false)
	{
		return;
	}

	if(hh < 10)
	{
		cleanSegment(0);
		setNumber(hh, 1);
	}
	else
	{
		setNumber(hh/10, 0);
		setNumber(hh%10, 1);
	}

	if(mm < 10)
	{
		setNumber(0, 2);
		setNumber(mm, 3);
	}
	else
	{
		setNumber(mm/10, 2);
		setNumber(mm%10, 3);
	}
}

void Display::animation()
{
	if(colonAnimation == true)
	{
		if(colonState == false)
		{
			pixel->fill(numberColor, 14, 2);

			colonState = true;
		}
		else
		{
			pixel->fill(0, 14, 2);

			colonState = false;
		}
	}

	pixel->show();
}

void Display::setAlarm(bool onOff)
{
	if(displayState.alarm == false)
	{
		return;
	}

	if(onOff == true)
	{
		pixel->setPixelColor(30, numberColor);
	}
	else
	{
		pixel->setPixelColor(30, 0);
	}
	pixel->show();
}

void Display::cleanSegment(uint8_t segmentNumber)
{
	setNumber(-1, segmentNumber);
}

void Display::cleanAllSegments()
{
	for(uint8_t i = 0; i < 4; i++)
	{
		cleanSegment(i);
	}
}

void Display::displayOff(bool numbers, bool colon, bool alarm, bool temperatureMarkers)
{
	if(numbers == false)
	{
		cleanAllSegments();
		displayState.numbers = false;
	}

	if(colon == false)
	{
		colonAnimation = false;
		displayState.colon = false;
	}

	if(alarm == false)
	{
		setAlarm(false);
		displayState.alarm = false;
	}

	if(temperatureMarkers == false)
	{
		setTempInColor(0);
		setTempOutColor(0);
		displayState.temperatureMarkers = false;
	}
}

void Display::displayOn()
{
	displayState.numbers = true;
	displayState.colon = true;
	displayState.alarm = true;
	displayState.temperatureMarkers = true;
}

bool Display::getDisplayState()
{

}
