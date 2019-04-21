#include "config.h"
#include "main.h"

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

#include <Adafruit_NeoPixel.h>

#include <TimeLib.h>
#include <NTPClient.h>

#include "setup.h"
#include "display.h"

WiFiClient espClient;
PubSubClient client(espClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org");

unsigned long timer1s = 0;

#define pixelCount 33
Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixelCount, 4, NEO_GRB + NEO_KHZ800);
Display * display;

time_t requestSync()
{
	Serial.print("Time sync: ");

	timeClient.update();
	time_t timestamp = timeClient.getEpochTime();
	setTime(timestamp);

	Serial.println(timestamp);
	return 0;
}

void reconnect()
{
	digitalWrite(1, LOW);

	//if (client.connect(controllerName, mqttLogin, mqttPasswd))
	if (client.connect(controllerName))
	{
		client.subscribe("home/controllers/2/restart");
		client.subscribe("home/controllers/2/sleep");

		client.subscribe("home/myRoom/dateTime/timestamp");
		client.subscribe("home/myRoom/dateTime/timeZone");
		client.subscribe("home/myRoom/dateTime/DST");

		client.subscribe("home/myRoom/alarmClock/brightness");
		client.subscribe("home/myRoom/alarmClock/displayColor");

		client.subscribe("home/myRoom/alarmClock/TempInColor");
		client.subscribe("home/myRoom/alarmClock/TempOutColor");

		digitalWrite(1, HIGH);
	}
}

void callback(char * topic, byte* payload, unsigned int length)
{
	if(strcmp(topic,"home/controllers/2/restart")==0)
	{
		if((char)payload[0] == 'r')
		{
			client.publish("home/controllers/2/condition", "reset");
			delay(500);

			digitalWrite(1, LOW);
			digitalWrite(3, LOW);
			ESP.restart();
		}
	}

	if(strcmp(topic,"home/controllers/2/sleep")==0)
	{
		if((char)payload[0] == 's')
		{
			client.publish("home/controllers/2/condition", "sleep");
			delay(500);

			//ESP.deepSleep(30e6, RF_DEFAULT);
		}
	}

	if(strcmp(topic,"home/myRoom/dateTime/timestamp")==0)
	{
		time_t timestamp = bytesToInt(payload,length);

		setTime(timestamp);
	}

	if(strcmp(topic,"home/myRoom/dateTime/timeZone")==0)
	{
		int8_t timeZone = bytesToInt(payload,length);

		timeClient.setTimeOffset(timeZone * 3600);
		requestSync();

		EEPROM.write(EEPROM_TIME_ZONE, timeZone);
		EEPROM.commit();

		Serial.print("Time zone: ");
		Serial.println(timeZone);
	}

	if(strcmp(topic,"home/myRoom/dateTime/DST")==0)
	{
		int8_t DST = bytesToInt(payload,length);

		if((DST == 0) || (DST == 1))
		{
			timeClient.setTimeOffset((EEPROM.read(EEPROM_TIME_ZONE) * 3600) + (DST * 3600));
			requestSync();

			EEPROM.write(EEPROM_DST, DST);
			EEPROM.commit();
		}
		Serial.print("DST: ");
		Serial.println(DST);
	}

	if(strcmp(topic,"home/myRoom/alarmClock/brightness")==0)
	{
		int value = bytesToInt(payload,length);

		display->setBrightness(value);
	}

	if(strcmp(topic,"home/myRoom/alarmClock/displayColor")==0)
	{
		uint32_t color = bytesToUint32(payload,length);
		display->setColor(color);

		Serial.print("Display color: ");
		Serial.println(color);
	}

	if(strcmp(topic,"home/myRoom/alarmClock/TempInColor")==0)
	{
		uint32_t color = bytesToUint32(payload,length);
		display->setTempInColor(color);

		Serial.print("Temperature inside color: ");
		Serial.println(color);
	}

	if(strcmp(topic,"home/myRoom/alarmClock/TempOutColor")==0)
	{
		uint32_t color = bytesToUint32(payload,length);
		display->setTempOutColor(color);

		Serial.print("Temperature outside color: ");
		Serial.println(color);
	}
}

void setup()
{
	Serial.begin(9600);	//----------------------------------------------------------------------

	EEPROM.begin(256);

	WIFIsetup();
	OTAsetup();
	client.setServer(mqttIP, mqttPort);
	client.setCallback(callback);
	reconnect();

	timeClient.setTimeOffset((EEPROM.read(EEPROM_TIME_ZONE) * 3600) + (EEPROM.read(EEPROM_DST) * 3600));
	timeClient.begin();
	setSyncProvider(requestSync);
	setSyncInterval(60);	//-------------to changed on 60*60*24

	display = new Display(&strip);

	display->setColor(234534534);
	display->setAnimatedColon(true);
	display->setBrightness(10);	//------------------------------------------------------
}

void loop()
{
	if (!client.connected())
	{
		reconnect();
	}
	else
	{
		client.loop();
	}

	ArduinoOTA.handle();


	if((millis() - timer1s) > 1000)
	{
		timer1s = millis();

		display->animation();

		display->setTime(hour(), minute());
		display->setAlarm(true);	//------------------------------------------------------
	}
}
