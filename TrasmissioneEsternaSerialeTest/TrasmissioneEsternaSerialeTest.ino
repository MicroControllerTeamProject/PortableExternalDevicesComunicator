#include "SoftwareSerial.h"
#include <TimeLib.h>


SoftwareSerial* softwareSerial = new SoftwareSerial(12, 5);

void setup(void) {
	Serial.begin(9600);
	pinMode(A4, OUTPUT);
	softwareSerial->begin(19200);
}

//Il simbolo 'N' � considerato simbolo di errore.
void loop(void) {

	digitalWrite(A4, LOW);

	if (softwareSerial->available() > 0)
	{
		String recevedMessage = softwareSerial->readString();
		recevedMessage.trim();
		Serial.println(recevedMessage);
		if (recevedMessage.startsWith("H"))
		{
			String hour = recevedMessage.substring(1, 3);
			String minute = (recevedMessage.substring(3, 5));
			setTime(hour.toInt(), minute.toInt(), 1, 1, 1, 2019);

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
}
