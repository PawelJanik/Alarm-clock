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

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include "setup.h"
#include "display.h"

#define WS2812_PIN 4
#define DHT_PIN 13
#define DS18B20_PIN 12
#define BUZZER_PIN 14
#define BUTTONS_PIN A0

#define DHTTYPE DHT22     // DHT 22 (AM2302)
#define pixelCount 33

unsigned long timer10ms = 0;
unsigned long timer1s = 0;
unsigned long timer10s = 0;
unsigned long timer1m = 0;

WiFiClient espClient;
PubSubClient client(espClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org");

Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixelCount, WS2812_PIN, NEO_GRB + NEO_KHZ800);
Display * display;

DHT dht(DHT_PIN, DHTTYPE);

OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

ButtonState buttonsState[3];

time_t requestSync()
{
	timeClient.update();
	time_t timestamp = timeClient.getEpochTime();
	setTime(timestamp);

	return 0;
}

void reconnect()
{
	digitalWrite(1, LOW);

	//if (client.connect(controllerName, mqttLogin, mqttPasswd))
	if (client.connect(controllerName))
	{
		client.subscribe("home/controllers/3/restart");
		client.subscribe("home/controllers/3/sleep");

		client.subscribe("home/myRoom/alarmClock/dateTime/timestamp");
		client.subscribe("home/myRoom/alarmClock/dateTime/timeZone");
		client.subscribe("home/myRoom/alarmClock/dateTime/DST");

		client.subscribe("home/myRoom/alarmClock/brightness");
		client.subscribe("home/myRoom/alarmClock/displayColor");
		client.subscribe("home/myRoom/alarmClock/displayState");

		client.subscribe("home/myRoom/alarmClock/tempInColor");
		client.subscribe("home/myRoom/alarmClock/tempOutColor");

		client.subscribe("home/myRoom/alarmClock/buzzerState");

		digitalWrite(1, HIGH);
	}
}

void callback(char * topic, byte* payload, unsigned int length)
{
	if(strcmp(topic,"home/controllers/3/restart")==0)
	{
		if((char)payload[0] == 'r')
		{
			client.publish("home/controllers/3/condition", "reset");
			delay(500);

			ESP.restart();
		}
	}

	if(strcmp(topic,"home/controllers/3/sleep")==0)
	{
		if((char)payload[0] == 's')
		{
			client.publish("home/controllers/3/condition", "sleep");
			delay(500);

			ESP.deepSleep(5 * 1000000, WAKE_RF_DEFAULT);
			//-----------------------------------------------------------------------------------------
			delay(10);
		}
	}

	if(strcmp(topic,"home/myRoom/alarmClock/dateTime/timestamp")==0)
	{
		time_t timestamp = bytesToInt(payload,length);

		setTime(timestamp);
	}

	if(strcmp(topic,"home/myRoom/alarmClock/dateTime/timeZone")==0)
	{
		int8_t timeZone = bytesToInt(payload,length);

		timeClient.setTimeOffset((timeZone * 3600) + (EEPROM.read(EEPROM_DST) * 3600));
		requestSync();

		EEPROM.write(EEPROM_TIME_ZONE, timeZone);
		EEPROM.commit();
	}

	if(strcmp(topic,"home/myRoom/alarmClock/dateTime/DST")==0)
	{
		int8_t DST = bytesToInt(payload,length);

		if((DST == 0) || (DST == 1))
		{
			timeClient.setTimeOffset((EEPROM.read(EEPROM_TIME_ZONE) * 3600) + (DST * 3600));
			requestSync();

			EEPROM.write(EEPROM_DST, DST);
			EEPROM.commit();
		}
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
	}

	if(strcmp(topic,"home/myRoom/alarmClock/displayState")==0)
	{
		char state = payload[0];

		if(state == '0')
		{
			display->displayOff(false,true,false,false);
		}
		else if(state == '1')
		{
			display->displayOn();
		}
	}

	if(strcmp(topic,"home/myRoom/alarmClock/tempInColor")==0)
	{
		uint32_t color = bytesToUint32(payload,length);
		display->setTempInColor(color);
	}

	if(strcmp(topic,"home/myRoom/alarmClock/tempOutColor")==0)
	{
		uint32_t color = bytesToUint32(payload,length);
		display->setTempOutColor(color);
	}

	if(strcmp(topic,"home/myRoom/alarmClock/buzzerState")==0)
	{
		char state = payload[0];

		if(state == '0')
		{
			digitalWrite(BUZZER_PIN, LOW);
		}
		else if(state == '1')
		{
			digitalWrite(BUZZER_PIN, HIGH);
		}
	}
}

void setup()
{
	Serial.begin(9600);

	EEPROM.begin(256);

	WIFIsetup();
	OTAsetup();
	client.setServer(mqttIP, mqttPort);
	client.setCallback(callback);
	reconnect();

	timeClient.setTimeOffset((EEPROM.read(EEPROM_TIME_ZONE) * 3600) + (EEPROM.read(EEPROM_DST) * 3600));
	timeClient.begin();
	setSyncProvider(requestSync);
	setSyncInterval(60);	//------------------------------------------------------to changed on 60*60*24

	display = new Display(&strip);

	display->setColor(234534534);
	display->setAnimatedColon(true);

	dht.begin();
	ds18b20.begin();

	pinMode(BUZZER_PIN, OUTPUT);
	pinMode(BUTTONS_PIN, INPUT);
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
		ArduinoOTA.handle();
	}

	if((millis() - timer10ms) > 5)
	{
		timer10ms = millis();

		readKey(buttonsState, &client);
	}

	if((millis() - timer1s) > 1000)
	{
		timer1s = millis();

		display->animation();

		display->setTime(hour(), minute());
		display->setAlarm(true);	//-----------------------------------------------------------------
	}

	if((millis() - timer10s) > 10000)
	{
		timer10s = millis();
		client.publish("home/controllers/3/condition", "ok");
	}

	if((millis() - timer1m) > 60000)
	{
		timer1m = millis();

		client.publish("home/sensors/temperature/7", String(dht.readTemperature()).c_str());
		client.publish("home/sensors/humidity/0", String(dht.readHumidity()).c_str());

		ds18b20.requestTemperatures();
		client.publish("home/sensors/temperature/8", String(ds18b20.getTempCByIndex(0)).c_str());
	}

	if (Serial.available() > 0)
	{
		String str = Serial.readStringUntil('\n');

		if(str.substring(1,4) == "BAT")
		{
			client.publish("home/controllers/3/batteryVoltage", str.substring(5,9).c_str());
		}
		else if(str.substring(1,4) == "EXT")
		{
			client.publish("home/controllers/3/externalPowerVoltage", str.substring(5,9).c_str());
		}
		else if(str.substring(1,4) == "LIG")
		{
			client.publish("home/sensors/light/1", str.substring(5,10).c_str());
		}
	}
}
