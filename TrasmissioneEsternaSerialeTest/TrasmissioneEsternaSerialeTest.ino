#include "SoftwareSerial.h"

SoftwareSerial* softwareSerial = new SoftwareSerial(12, 5);

void setup(void) {
	Serial.begin(9600);
	pinMode(A4, OUTPUT);
	softwareSerial->begin(19200);
}

//Il simbolo 'N' è considerato simbolo di errore.
void loop(void) {

	digitalWrite(A4, HIGH);

	if (softwareSerial->available() > 0)
	{
		//Serial.println(softwareSerial->readString());
		String h = softwareSerial->readString();
		h.trim();
		if (h.equals("ok"))
		{
			Serial.println("Entrato");
			softwareSerial->print("t01Y08.50");
			softwareSerial->print("t02Y07.50");
			softwareSerial->print("t03Y47.50");
			softwareSerial->print("t04Y48.50");
			softwareSerial->print("t05Y47.50");
			softwareSerial->print("t06Y47.50");
			softwareSerial->print("t07Y48.50");
			softwareSerial->print("t08Y47.50");
			softwareSerial->print("t09Y47.50");
			softwareSerial->print("t10Y48.50");
			softwareSerial->print("t11N47.50*");
			//delay(400);
		}
	}
	
	
}
