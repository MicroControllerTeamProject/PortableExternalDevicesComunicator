#include "SoftwareSerial.h"
#include <TimeLib.h>

bool _isTimeInitialize = false;

SoftwareSerial* softwareSerial = new SoftwareSerial(12, 5);

void setup(void) {
	Serial.begin(9600);
	pinMode(A4, OUTPUT);
	softwareSerial->begin(19200);
}

String getSerialMessage()
{
	String recevedMessage = "";
	if (softwareSerial->available() > 0)
	{
		recevedMessage = softwareSerial->readString();
		recevedMessage.trim();
	}
	return recevedMessage;
}


//Il simbolo 'N' è considerato simbolo di errore.
void loop(void) {
	String receivedMessage = "";
	while (!_isTimeInitialize)
	{
		digitalWrite(A4, LOW);
		receivedMessage = getSerialMessage();
		if (receivedMessage.startsWith("H"))
		{
			String hour = receivedMessage.substring(1, 3);
			String minute = (receivedMessage.substring(3, 5));
			setTime(hour.toInt(), minute.toInt(), 1, 1, 1, 2019);
			Serial.print(hour); Serial.print(":"); Serial.print(minute);
			_isTimeInitialize = true;
		}
	}

	digitalWrite(A4, LOW);

	receivedMessage = getSerialMessage();
	if (receivedMessage.startsWith("H"))
	{
			softwareSerial->print("t01N08.50");
			softwareSerial->print("t02Y07.50");
			softwareSerial->print("t03Y47.50");
			softwareSerial->print("t04Y48.50");
			softwareSerial->print("t05Y47.50");
			softwareSerial->print("t06Y47.50");
			softwareSerial->print("t07Y48.50");
			softwareSerial->print("t08Y47.50");
			softwareSerial->print("t09Y47.50");
			softwareSerial->print("t10Y48.50");
			softwareSerial->print("t11Y47.50*");
		}
}
