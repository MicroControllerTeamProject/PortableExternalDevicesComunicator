/* Know Your SRAM

This little sketch prints on the serial monitor
the boundaries of .data and .bss memory areas,
of the Heap and the Stack and the amount of
free memory.

Main code from Leonardo Miliani:
www.leonardomiliani.com

freeRam() agorithm from JeeLabs:
http://jeelabs.org/2011/05/22/atmega-memory-use/

This code is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Reviewed on 26/03/2013

*/

//for some MCUs (i.e. the ATmega2560) there's no definition for RAMSTART
#ifndef RAMSTART
extern int __data_start;
#endif

extern int __data_end;
//extern int __bss_start;
//extern int __bss_end;
extern int __heap_start;
extern int __brkval;
#include <OneWire.h>
#include <Wire.h>
#include <LiquidCrystal_SR3W.h>
#include <LiquidCrystal_SR2W.h>
#include <LiquidCrystal_SR.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>
#include <LCD.h>
#include <I2CIO.h>
#include <FastIO.h>
#include <SoftwareSerial.h>
int temp;

#if (defined(__AVR__))
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif
//#include <MemoryFree.h>
//#include <pgmStrToRAM.h>
//#include <ChipTemp.h>
#include "MyBlueTooth.h"
#include "BlueToothCommandsUtil.h"
#include "LSGEEpromRW.h" 
#include <EEPROM.h> 
#include <StringFunctions.h>
#include "MySim900.h"
#include "ActivityManager.h"
#include <TimeLib.h>

unsigned long minutesConverter(uint16_t minutes);

char version[15] = "X01 1.00-RTM";

ActivityManager* _delayForTemperature = new ActivityManager(2 * 60);

ActivityManager* _delayForVoltage = new ActivityManager(5 * 60);

//ActivityManager* _delayForFreeRam = new ActivityManager(60);

ActivityManager* _delayForSms = new ActivityManager(2 * 60);

//ActivityManager* _delayForSignalStrength = new ActivityManager(30);

String _oldPassword = "";

String _newPassword = "";

const byte _addressStartBufPhoneNumber = 1;

const byte _addressStartBufTemperatureMax = 16;

const byte _addressDBPhoneIsON = 52;

const byte _addressStartBufPhoneNumberAlternative = 54;

const byte _addressOffSetTemperature = 92;

bool _isBlueLedDisable = true;

bool _isDisableCall = false;

bool _isDisableCallWithTime = false;

unsigned long _disableCallDuration = minutesConverter(15);

unsigned long _disableCallTime = 0;

char _prefix[4] = "+39";

char _phoneNumber[11];

char _phoneNumberAlternative[11];

String _whatIsHappened = "";

uint8_t _phoneNumbers = 0;

uint8_t _tempMax = 0;

unsigned int _offSetTempValue = 324.13;

float _voltageValue = 0;

float _voltageMinValue = 0;

unsigned long _millsStart = 0;

const int BUFSIZEPHONENUMBER = 11;

const int BUFSIZEPHONENUMBERALTERANATIVE = 11;;

const int BUFSIZEDBPHONEON = 2;
char _bufDbPhoneON[BUFSIZEDBPHONEON];

const int BUFSIZETEMPERATUREMAX = 3;
char _bufTemperatureMax[BUFSIZETEMPERATUREMAX];

const int BUFSIZEOFFSETTEMPERATURE = 5;
char _bufOffSetTemperature[BUFSIZEOFFSETTEMPERATURE];


//---------------------------------------------       PINS USED  Start ----------------------------------------------------------

static const uint8_t softwareSerialExternalDevicesTxPort = A5;

static const uint8_t softwareSerialExternalDevicesRxPort = A4;

static const uint8_t softwareSerialExternalDevicesPinAlarm = A2;

static const uint8_t bluetoothKeyPin = 10;

static const uint8_t bluetoothTransistorPin = 6;

static const byte _pin_powerLed = 13;

static const byte _pin_rxSIM900 = 7;

static const byte _pin_txSIM900 = 8;

//---------------------------------------------       PINS USED END   ----------------------------------------------------------

bool _isBTEnable = true;

unsigned long _btTimeConfiguration = 0;

char problematicDevice[4];

SoftwareSerial softwareSerial(softwareSerialExternalDevicesRxPort, softwareSerialExternalDevicesTxPort);

MyBlueTooth btSerial(&Serial, bluetoothKeyPin, bluetoothTransistorPin, 38400, 9600);

#define _HARDWARE_CODE 

void setup()
{
  Serial.begin(9600);

	softwareSerial.begin(9600);

	inizializePins();

	smsInit();

	initilizeEEPromData();

	btSerial.Reset_To_Slave_Mode();

	_oldPassword = btSerial.GetPassword();

	btSerial.ReceveMode();

	btSerial.turnOnBlueTooth();

	_whatIsHappened = F("X");

	blinkLed();


}

void loop()
{
	if (_isBTEnable)
	{
    
		if (_btTimeConfiguration == 0) {
    
			_btTimeConfiguration = millis();

			callSim900();
		}

		blueToothConfigurationSystem();

		if ((millis() - _btTimeConfiguration) > minutesConverter(5))
		{
			_btTimeConfiguration = 0;
			_isBTEnable = false;
			callSim900();
		}
		return;
	}


	//if (_delayForFreeRam->IsDelayTimeFinished(true))
	//{
	//	Serial.print(F("mem :")); Serial.println(freeRam());
	//}

	while (digitalRead(softwareSerialExternalDevicesPinAlarm) == LOW) {
		//Serial.println(F("LOW"));
		callSim900();
		getExternalDevices();
		//readIncomingSMS();
	}

	readIncomingSMS();

	internalTemperatureActivity();

	voltageActivity();
}

void smsInit() {

	MySim900 mySim900(_pin_rxSIM900, _pin_txSIM900, false);

	delay(3000);

	mySim900.Begin(19200);

	delay(3000);

	mySim900.IsCallDisabled(false);

	mySim900.ATCommand("AT+CPMS=\"SM\"");

	delay(2000);

	mySim900.ATCommand("AT+CMGF=1");

	delay(2000);

	mySim900.ATCommand("AT+CMGD=1,4");

	delay(2000);
}

void initilizeEEPromData()
{
	LSG_EEpromRW eepromRW;

  #ifdef _HARDWARE_CODE  
  char hardware_code[4] = {};
	eepromRW.eeprom_read_string(500, hardware_code,4);
	Serial.print(F("\r\nhardware_code: "));Serial.println(hardware_code);
 #endif 

	eepromRW.eeprom_read_string(_addressStartBufPhoneNumber, _phoneNumber, BUFSIZEPHONENUMBER);

	eepromRW.eeprom_read_string(_addressStartBufPhoneNumberAlternative, _phoneNumberAlternative, BUFSIZEPHONENUMBERALTERANATIVE);

	eepromRW.eeprom_read_string(_addressDBPhoneIsON, _bufDbPhoneON, BUFSIZEDBPHONEON);
	_phoneNumbers = atoi(&_bufDbPhoneON[0]);

	eepromRW.eeprom_read_string(_addressStartBufTemperatureMax, _bufTemperatureMax, BUFSIZETEMPERATUREMAX);
	_tempMax = atoi(_bufTemperatureMax);

	eepromRW.eeprom_read_string(_addressOffSetTemperature, _bufOffSetTemperature, BUFSIZEOFFSETTEMPERATURE);
	_offSetTempValue = atoi(_bufOffSetTemperature);


}

void inizializePins()
{
	pinMode(_pin_powerLed, OUTPUT);
	pinMode(softwareSerialExternalDevicesPinAlarm, INPUT_PULLUP);
}

void callSim900()
{
	MySim900 mySim900(_pin_rxSIM900, _pin_txSIM900, false);

	delay(3000);

	mySim900.Begin(19200);

	delay(3000);


	if (_isDisableCallWithTime && _disableCallTime == 0) {
		_disableCallTime = millis() + _disableCallDuration;
	}

	if (_isDisableCallWithTime && (_disableCallTime >= millis())) {
		return;
	}
	else if (_isDisableCallWithTime) {


		//Serial.println(F("restore.call"));


		_disableCallTime = 0;

		_isDisableCallWithTime = false;
	}

	char phoneNumber[14];

	strcpy(phoneNumber, _prefix);

	//Serial.println(F("make.call"));

	if (_isDisableCall) { return; }

	if (_phoneNumbers == 1)
	{
		strcat(phoneNumber, _phoneNumber);
		mySim900.DialVoiceCall(phoneNumber);
	}

	if (_phoneNumbers == 2)
	{
		strcat(phoneNumber, _phoneNumberAlternative);
		mySim900.DialVoiceCall(phoneNumber);
	}

	if (_phoneNumbers == 3)
	{
		strcat(phoneNumber, _phoneNumber);
		mySim900.DialVoiceCall(phoneNumber);
		//delay(10000);
		unsigned long time1 = millis();
		String messageReceived = "";
		while (millis() - time1 < 30000)
		{
			if (mySim900.IsAvailable())
			{
				messageReceived = mySim900.ReadIncomingChars2();
				messageReceived.trim();
				//Serial.println(messageReceived);
				if (messageReceived.equals("BUSY"))
				{
					//Serial.println("OCCUPATO");
					mySim900.ATCommand("ATH");
					memset(phoneNumber, 0, sizeof(phoneNumber));
					strcpy(phoneNumber, _prefix);
					strcat(phoneNumber, _phoneNumberAlternative);
					mySim900.DialVoiceCall(phoneNumber);
				}

			}
		}
	}

	delay(10000);

	//mySim900.ReadIncomingChars2();

}

bool chechDevicesValue(char buffExternalDevices[100])
{
	bool isOnAlarm = false;
	//Serial.println(h);
	for (int i = 0; i < 100; i += 9)
	{
		int index = 0;
		//Serial.print("i :"); Serial.println(i);
		for (int ii = i; ii < i + 4; ii++)
		{
			/*Serial.print("ii :"); Serial.println(ii);
			Serial.print(h[ii]);*/
			if (buffExternalDevices[ii] == 'N')
			{
				//Serial.print(F("ALARM DEVICE :")); Serial.println(problematicDevice);
				isOnAlarm = true;
			}
			problematicDevice[index] = buffExternalDevices[ii];
			index++;
		}
		memset(problematicDevice, 0, sizeof(problematicDevice));
	}
	return isOnAlarm;
}

void getExternalDevices()
{
	bool isOnAlarm = false;

	delay(1500);

	softwareSerial.print("H");

	if (hour() < 10) { softwareSerial.print('0'); }
	softwareSerial.print(hour());
	if (minute() < 10) { softwareSerial.print('0'); }
	softwareSerial.print(minute());

	if (softwareSerial.available() > 0)
	{
		//Serial.println(F("got mess:"));
		char* buffExtenalDevices = new char[100];
		softwareSerial.readStringUntil('*').toCharArray(buffExtenalDevices, 100);
		isOnAlarm = chechDevicesValue(buffExtenalDevices);
		delete[] buffExtenalDevices;
	}

	if (isOnAlarm)
	{
		//Serial.println("Fare una chiamata");
		callSim900();
	}
}

void blinkLed()
{
	if (_isBlueLedDisable) { return; }
	pinMode(_pin_powerLed, OUTPUT);
	for (uint8_t i = 0; i < 3; i++)
	{
		digitalWrite(_pin_powerLed, HIGH);
		delay(50);
		digitalWrite(_pin_powerLed, LOW);
		delay(50);
	}
}

String splitStringIndex(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = { 0, -1 };
	int maxIndex = data.length() - 1;

	for (int i = 0; i <= maxIndex && found <= index; i++) {
		if (data.charAt(i) == separator || i == maxIndex) {
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}
	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String calculateBatteryLevel(float batteryLevel)

{
	if (batteryLevel <= 3.25)
		return F("[    ]+");
	if (batteryLevel <= 3.30)
		return F("[|   ]+");
	if (batteryLevel <= 3.40)
		return F("[||  ]+");
	if (batteryLevel <= 3.60)
		return F("[||| ]+");
	if (batteryLevel <= 5.50)
		return F("[||||]+");

}

void loadMainMenu()
{
	char alarmStatus[15]={};

	/*if (_isAlarmOn)
	{
		String(F("Alarm ON")).toCharArray(alarmStatus, 15);
	}
	else
	{
		String(F("Alarm OFF")).toCharArray(alarmStatus, 15);
	}*/

	char result[30];   // array to hold the result.

	strcpy(result, alarmStatus); // copy string one into the result.
	strcat(result, version); // append string two to the result.

	//ChipTemp* chipTemp = new ChipTemp();
	int internalTemperature = getTemp();//chipTemp->celsius();
	/*delete (chipTemp);*/
	delete(alarmStatus);



	String battery = calculateBatteryLevel(_voltageValue);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor(result, BlueToothCommandsUtil::Title));

	//char* commandString = new char[15];

	//String(F("Configuration")).toCharArray(commandString, 15);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Configuration"), BlueToothCommandsUtil::Menu, F("001")));


	//String(F("Security")).toCharArray(commandString, 15);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Security"), BlueToothCommandsUtil::Menu, F("004")));

	//if (!_isAlarmOn)
	//{
	//	//String(F("Alarm On")).toCharArray(commandString, 15);
	//	btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Alarm On"), BlueToothCommandsUtil::Command, F("002")));
	//}
	//else
	//{
	//	//String(F("Alarm OFF")).toCharArray(commandString, 15);
	//	btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Alarm OFF"), BlueToothCommandsUtil::Command, F("003")));
	//}

	//String(F("Temp.:")).toCharArray(commandString, 15);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("Temp.:" + String(internalTemperature), BlueToothCommandsUtil::Info));

	/*btSerial.println(BlueToothCommandsUtil::CommandConstructor("Batt.value:" + String(_voltageValue), BlueToothCommandsUtil::Info));*/

	//String(F("Batt.level:")).toCharArray(commandString, 15);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("Batt.level:" + battery, BlueToothCommandsUtil::Info));


	btSerial.println(BlueToothCommandsUtil::CommandConstructor("WhatzUp:" + _whatIsHappened, BlueToothCommandsUtil::Info));

	String hours = String(hour());

	String minutes = String(minute());

	if (hour() < 10)
	{
		hours = "0" + String(hour());
	}

	if (minute() < 10)
	{
		minutes = "0" + String(minute());
	}

	btSerial.println(BlueToothCommandsUtil::CommandConstructor("Time:" + hours + ":" + String(minutes), BlueToothCommandsUtil::Info));

	////String(F("Signal:")).toCharArray(commandString, 15);
	//btSerial.println(BlueToothCommandsUtil::CommandConstructor("Signal:" + _signalStrength, BlueToothCommandsUtil::Info));

	btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	btSerial.Flush();
	//delete(commandString);
}

void loadConfigurationMenu()
{
	btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Configuration"), BlueToothCommandsUtil::Title));

	btSerial.println(BlueToothCommandsUtil::CommandConstructor("Phone:" + String(_phoneNumber), BlueToothCommandsUtil::Data, F("001")));

	
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("Ph.Altern.:" + String(_phoneNumberAlternative), BlueToothCommandsUtil::Data, F("099")));

	
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("N.Phone:" + String(_phoneNumbers), BlueToothCommandsUtil::Data, F("098")));

	
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("TempMax:" + String(_tempMax), BlueToothCommandsUtil::Data, F("004")));

	
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("OffSetTemp:" + String(_offSetTempValue), BlueToothCommandsUtil::Data, F("095")));

	btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
}

void blueToothConfigurationSystem()
{
	LSG_EEpromRW eepromRW;
	String _bluetoothData = "";
	if (btSerial.available())
	{
		_bluetoothData = btSerial.readString();
	
#pragma region Main Menu-#0
		if (_bluetoothData.indexOf(F("#0")) > -1)
		{
			loadMainMenu();
		}

#pragma region Commands
#pragma endregion

#pragma region Data
#pragma endregion

#pragma endregion Main Menu

		//ROOT Main/Configuration
#pragma region Configuration Menu-#M001
		if (_bluetoothData.indexOf(F("M001")) > -1)
		{
			loadConfigurationMenu();
		}
#pragma region Commands

#pragma endregion


#pragma region Data
		if (_bluetoothData.indexOf(F("D001")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_phoneNumber, BUFSIZEPHONENUMBER);
				eepromRW.eeprom_write_string(1, _phoneNumber);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D095")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufOffSetTemperature, BUFSIZEOFFSETTEMPERATURE);
				eepromRW.eeprom_write_string(_addressOffSetTemperature, _bufOffSetTemperature);
				_offSetTempValue = atoi(&_bufOffSetTemperature[0]);
			}

			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D098")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufDbPhoneON, BUFSIZEDBPHONEON);
				eepromRW.eeprom_write_string(_addressDBPhoneIsON, _bufDbPhoneON);
				_phoneNumbers = atoi(&_bufDbPhoneON[0]);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D099")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_phoneNumberAlternative, BUFSIZEPHONENUMBERALTERANATIVE);
				eepromRW.eeprom_write_string(_addressStartBufPhoneNumberAlternative, _phoneNumberAlternative);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D004")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufTemperatureMax, BUFSIZETEMPERATUREMAX);
				eepromRW.eeprom_write_string(16, _bufTemperatureMax);
				_tempMax = atoi(_bufTemperatureMax);
			}
			loadConfigurationMenu();
		}

#pragma endregion

#pragma Configuration Menu endregion


#pragma region Security-M004
		if (_bluetoothData.indexOf(F("M004")) > -1)
		{
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Security"), BlueToothCommandsUtil::Title));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw.:"), BlueToothCommandsUtil::Menu, F("005")));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change name:"), BlueToothCommandsUtil::Menu, F("006")));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}
#pragma region Menu
		if (_bluetoothData.indexOf(F("M005")) > -1)
		{
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Insert old passw.:"), BlueToothCommandsUtil::Data, F("006")));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}

		if (_bluetoothData.indexOf(F("M006")) > -1)
		{
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Insert name:"), BlueToothCommandsUtil::Data, F("007")));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}
#pragma endregion


#pragma region Commands

#pragma endregion

#pragma region Data
		if (_bluetoothData.indexOf(F("D006")) > -1)
		{
			String confirmedOldPassword = splitStringIndex(_bluetoothData, ';', 1);

			if (_oldPassword == confirmedOldPassword)
			{
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Insert new passw:"), BlueToothCommandsUtil::Data, F("008")));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			}
			else
			{
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Wrong passw:"), BlueToothCommandsUtil::Message));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			}

		}

		if (_bluetoothData.indexOf(F("D008")) > -1)
		{
			_newPassword = splitStringIndex(_bluetoothData, ';', 1);
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Confirm pass:"), BlueToothCommandsUtil::Data, F("009")));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}

		if (_bluetoothData.indexOf(F("D009")) > -1)
		{
			if (_newPassword == splitStringIndex(_bluetoothData, ';', 1))
			{
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("changed:"), BlueToothCommandsUtil::Message));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
				delay(2000);
				btSerial.SetPassword(_newPassword);
				_oldPassword = _newPassword;
			}

			else
			{
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("passw. doesn't match"), BlueToothCommandsUtil::Message));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
				btSerial.println("D006");
			}
		}


		if (_bluetoothData.indexOf(F("D007")) > -1)
		{
			String btName = splitStringIndex(_bluetoothData, ';', 1);

			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("changed:"), BlueToothCommandsUtil::Message));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			delay(2000);
			btSerial.SetBlueToothName(btName);
		}


#pragma endregion

#pragma endregion

		delay(100);
	}
	
}

boolean isValidNumber(String str)
{
	for (byte i = 0; i < str.length(); i++)
	{
		if (isDigit(str.charAt(i))) return true;
	}
	return false;
}

void internalTemperatureActivity()
{
	if (_delayForTemperature->IsDelayTimeFinished(true))
	{
		/*if (_isTemperatureCheckOn == 0) return;*/

		/*ChipTemp* chipTemp = new ChipTemp();*/

		if ((uint8_t)getTemp() > _tempMax)
		{

			_whatIsHappened = F("T");
			Serial.println(F("temp.act"));
			callSim900();
		}
		/*delete chipTemp;*/
	}
}

void voltageActivity()
{
	if (_delayForVoltage->IsDelayTimeFinished(true))
	{
		_voltageValue = (5.10 / 1023.00) * analogRead(A1);
		_voltageMinValue = 3.25;

		if (_voltageValue < _voltageMinValue)
		{
			_whatIsHappened = F("V");
			//Serial.println(F("volt.act"));
			callSim900();
		}
	}
}

void readIncomingSMS()
{
	if (!_delayForSms->IsDelayTimeFinished(true)) return;

	MySim900 mySim900(_pin_rxSIM900, _pin_txSIM900, false);

	delay(3000);

	mySim900.Begin(19200);

	delay(3000);

	String smsResponse = "";

	String response = "";

	//Serial.println(F("sms func"));

	mySim900.ReadIncomingChars2();

	delay(2000);

	for (uint8_t index = 1; index < 4; index++)
	{
		char number[2] = { index + 48 ,'\0' };

		char commandFindSms[10] = "AT+CMGR=";

		strcat(commandFindSms, number);

		mySim900.ATCommand(commandFindSms);

		delay(4000);

		if (mySim900.IsAvailable())
		{
			response = mySim900.ReadIncomingChars2();
			//Serial.print(F("#")); Serial.print(response); Serial.println(F("#"));
			response.trim();

			if (response.indexOf("+CMGR:") != -1)
			{
				blinkLed();

				int index = response.lastIndexOf('"');
				smsResponse = response.substring(index + 3);
				index = smsResponse.lastIndexOf('\r\n');
				smsResponse = smsResponse.substring(0, index);

				//Serial.print(F("smsResponse :")); Serial.println(smsResponse);

				delay(1000);

				char commandSmsDelete[10] = "AT+CMGD=";

				strcat(commandSmsDelete, number);

				mySim900.ATCommand(commandSmsDelete);


				//Serial.print(F("del.sms ")); Serial.println(number);


				delay(3000);

				if (response[0] != '#')
				{
					if (response.substring(36, 46) != _phoneNumber &&
						response.substring(51, 61) != _phoneNumber &&
						response.substring(36, 46) != _phoneNumberAlternative &&
						response.substring(51, 61) != _phoneNumberAlternative) {

						//Serial.println(F("Num.err"));

					}
					else {
						listOfSmsCommands(smsResponse);
					}
				}
				else
				{
					listOfSmsCommands(smsResponse);
				}

			}
		}


	}
}

void listOfSmsCommands(String command)
{
	command.trim();

	if (command.startsWith("H"))
	{
		String hour = command.substring(1, 3);
		//Serial.println(hour);
		String minute = (command.substring(3, 5));
		//Serial.println(minute);
		setTime(hour.toInt(), minute.toInt(), 1, 1, 1, 2019);
		callSim900();
	}

	if (command.startsWith("#"))
	{
		command.substring(1).toCharArray(_phoneNumber, 11);
		//Serial.print(F("_phne.Num: ")); Serial.println(_phoneNumber);
	}

	if (command == F("Ac"))
	{
		_isDisableCall = false;
		callSim900();
	}

	if (command == F("Dc"))
	{
		callSim900();
		_isDisableCall = true;
	}

	if (command == F("Dt"))
	{
		//Serial.println(F("Dis.call.time"));
		_isDisableCallWithTime = true;
	}

	if (command == F("Al"))
	{
		_isBlueLedDisable = false;
		callSim900();
	}

	if (command == F("Ck"))
	{

		//Serial.println(F("test"));

		callSim900();
	}

	if (command == F("Be"))
	{

		//Serial.println(F("bten"));

		_isBTEnable = true;
	}
}

double getTemp(void)
{
	unsigned int wADC;
	double t;

	// The internal temperature has to be used
	// with the internal reference of 1.1V.
	// Channel 8 can not be selected with
	// the analogRead function yet.

	// Set the internal reference and mux.
	ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
	ADCSRA |= _BV(ADEN);  // enable the ADC

	delay(20);            // wait for voltages to become stable.

	ADCSRA |= _BV(ADSC);  // Start the ADC

	// Detect end-of-conversion
	while (bit_is_set(ADCSRA, ADSC));

	// Reading register "ADCW" takes care of how to read ADCL and ADCH.
	wADC = ADCW;

	// The offset of 324.31 could be wrong. It is just an indication.
	t = (wADC - _offSetTempValue) / 1.22;

	// The returned temperature is in degrees Celsius.
	return (t);
}

//computes the free memory (from JeeLabs)
int freeRam() {
	int v;
	temp = (int)&v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

unsigned long minutesConverter(uint16_t minutes)
{
	return (unsigned long)minutes * 60UL * 1000UL;
}
