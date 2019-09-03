#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define EEPROM_TIME_ZONE 0
#define EEPROM_DST 1

struct ButtonState
{
	bool isClicked;
	unsigned long clickTime;
};

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

void readKey(ButtonState * buttonsState, PubSubClient * client)
{
	int buttonsValue = analogRead(A0);



	if(buttonsValue > 1000)
	{
		if(buttonsState[0].isClicked == false)
		{
			buttonsState[0].clickTime = millis();
			buttonsState[0].isClicked = true;
			client->publish("home/sensors/switch/2/clicked", "1");
		}
		else
		{
			client->publish("home/sensors/switch/2", String(millis() - buttonsState[0].clickTime).c_str());
		}
	}
	else if(buttonsValue > 350)
	{
		if(buttonsState[1].isClicked == false)
		{
			buttonsState[1].clickTime = millis();
			buttonsState[1].isClicked = true;
			client->publish("home/sensors/switch/3/clicked", "1");
		}
		else
		{
			client->publish("home/sensors/switch/3", String(millis() - buttonsState[1].clickTime).c_str());
		}
	}
	else if(buttonsValue > 200)
	{
		if(buttonsState[2].isClicked == false)
		{
			buttonsState[2].clickTime = millis();
			buttonsState[2].isClicked = true;
			client->publish("home/sensors/switch/4/clicked", "1");
		}
		else
		{
			client->publish("home/sensors/switch/4", String(millis() - buttonsState[2].clickTime).c_str());
		}
	}
	else
	{
		for(uint8_t i = 0; i < 3; i++)
		{
			char topic[31] = "home/sensors/switch/i/released";

			if(buttonsState[i].isClicked == true)
			{
				topic[20] = '2' + i;
				buttonsState[i].isClicked = false;
				client->publish(topic, "1");
			}
		}
	}
}
