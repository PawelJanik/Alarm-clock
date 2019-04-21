#include <ESP8266WiFi.h>

#define EEPROM_TIME_ZONE 0
#define EEPROM_DST 1

int32_t bytesToInt(unsigned char* b, unsigned length)
{
	int32_t val = 0;
	int j = 0;
	uint8_t sign = 0;

	if(b[0] == '-')
	{
		sign = 1;
	}

	for (int i = length-1; i >= sign; --i)
	{
		val += (b[i] - '0') * pow(10,j);
		j++;
	}

	if(b[0] == '-')
	{
		val = val * (-1);
	}
	return val;
}

uint32_t bytesToUint32(unsigned char* b, unsigned length)
{
	uint32_t val = 0;
	int j = 0;

	for (int i = length-1; i >= 0; --i)
	{
		val += (b[i] - '0') * pow(10,j);
		j++;
	}
	return val;
}
