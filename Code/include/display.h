#include <Adafruit_NeoPixel.h>

class Display
{
public:
	Display(Adafruit_NeoPixel * Pixel);

	void setBrightness(uint8_t brightness);
	void setColor(uint32_t color);
	void fillColor(uint32_t color);
	void setNumber(uint8_t number, uint8_t charNumber);
	void displayClear();
	void setAnimatedColon(bool onOff);
	void setTime(uint8_t hh, uint8_t mm);

	void setAlarm(bool onOff);

	void animation();

	void setTempInColor(uint32_t color);
	void setTempOutColor(uint32_t color);

private:
	Adafruit_NeoPixel * pixel;

	uint32_t numberColor;
	bool colonState;
	bool colonAnimation;

	bool signs[16][7] = 	{{1,1,1,0,1,1,1},	//0
				 {0,0,1,0,0,0,1},	//1
			 	 {1,1,0,1,0,1,1},	//2
				 {0,1,1,1,0,1,1},	//3
				 {0,0,1,1,1,0,1},	//4
				 {0,1,1,1,1,1,0},	//5
				 {1,1,1,1,1,1,0},	//6
				 {0,0,1,0,0,1,1},	//7
				 {1,1,1,1,1,1,1},	//8
				 {0,1,1,1,1,1,1}	//9
			 };
};
