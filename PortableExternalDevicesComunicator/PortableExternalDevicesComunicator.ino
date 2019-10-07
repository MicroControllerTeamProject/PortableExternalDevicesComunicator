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

char version[15] = "-X01 1.00-alfa";

ActivityManager* _delayForTemperature = new ActivityManager(60);

ActivityManager* _delayForVoltage = new ActivityManager(60);

ActivityManager* _delayForFindPhone = new ActivityManager(30);

ActivityManager* _delayForSignalStrength = new ActivityManager(30);

MyBlueTooth* btSerial;

MySim900* mySim900;

String _oldPassword = "";

String _newPassword = "";

const byte _addressStartBufPhoneNumber = 1;

const byte _addressStartBufPrecisionNumber = 12;

const byte _addressStartBufTemperatureIsOn = 14;

const byte _addressStartBufTemperatureMax = 16;

//const byte _addressStartBufPirSensorIsON = 19;

const byte _addressStartDeviceAddress = 21;

const byte _addressStartDeviceName = 33;

const byte _addressStartFindOutPhonesON = 48;

const byte _addressStartBTSleepIsON = 50;

const byte _addressDBPhoneIsON = 52;

const byte _addressStartBufPhoneNumberAlternative = 54;

const byte _addressStartFindMode = 65;
//
//const byte _addressApn = 67;

const byte _addressOffSetTemperature = 92;

const byte _addressDelayFindMe = 94;

const byte _addressExternalInterruptIsOn = 96;

const byte _addressStartDeviceAddress2 = 98;

const byte _addressStartDeviceName2 = 110;

//uint8_t _isPIRSensorActivated = 0;

bool _isBlueLedDisable = true;

bool _isDisableCall = false;

//bool _isOnExternalDeviceAlarm = false;

char _prefix[4] = "+39";

bool _isAlarmOn = false;

//String _phoneNumber;

char _phoneNumber[11];

//String _phoneNumberAlternative;

char _phoneNumberAlternative[11];

String _whatIsHappened = "";

//uint8_t _isTemperatureCheckOn = 0;

uint8_t _isBTSleepON = 1;

uint8_t _isExternalInterruptOn = 0;

uint8_t _phoneNumbers = 0;

//uint8_t _findOutPhonesMode = 0;

uint8_t _tempMax = 0;

uint8_t _delayFindMe = 1;

unsigned int _offSetTempValue = 324.13;

String _signalStrength;

//String _deviceAddress = "";
//
//String _deviceAddress2 = "";
//
//String _deviceName = "";
//
//String _deviceName2 = "";

float _voltageValue = 0;

float _voltageMinValue = 0;

unsigned long _millsStart = 0;

//bool _isMasterMode = false;

//unsigned long _pirSensorTime = 0;

//unsigned long _timeToTurnOfBTAfterPowerOn = 300000;
//
//unsigned long _timeAfterPowerOnForBTFinder = 300000;

//String _apn = "";

bool _isDeviceDetected = false;

bool _isOnInterrupt = false;

const int BUFSIZEPHONENUMBER = 11;
//char _bufPhoneNumber[BUFSIZEPHONENUMBER];

const int BUFSIZEPHONENUMBERALTERANATIVE = 11;
//char _bufPhoneNumberAlternative[BUFSIZEPHONENUMBERALTERANATIVE];
//
//const int BUFSIZEPRECISION = 2;
//char _bufPrecisionNumber[BUFSIZEPRECISION];

const int BUFSIZETEMPERATUREISON = 2;
char _bufTemperatureIsOn[BUFSIZETEMPERATUREISON];

//const int BUFSIZEPIRSENSORISON = 2;
//char _bufPirSensorIsON[BUFSIZEPIRSENSORISON];

const int BUFSIZEFINDOUTPHONESON = 2;
char _bufFindOutPhonesON[BUFSIZEFINDOUTPHONESON];

const int BUFSIZEDBPHONEON = 2;
char _bufDbPhoneON[BUFSIZEDBPHONEON];

const int BUFSIZETEMPERATUREMAX = 3;
char _bufTemperatureMax[BUFSIZETEMPERATUREMAX];

const int BUFSIZEDEVICEADDRESS = 13;
char _bufDeviceAddress[BUFSIZEDEVICEADDRESS];
char _bufDeviceAddress2[BUFSIZEDEVICEADDRESS];


const int BUFSIZEDEVICENAME = 15;
char _bufDeviceName[BUFSIZEDEVICENAME];
char _bufDeviceName2[BUFSIZEDEVICENAME];

//const int BUFSIZEAPN = 25;
//char _bufApn[BUFSIZEAPN];

const int BUFSIZEOFFSETTEMPERATURE = 5;
char _bufOffSetTemperature[BUFSIZEOFFSETTEMPERATURE];

const int BUFSIZEDELAYFINDME = 2;
char _bufDelayFindMe[BUFSIZEDELAYFINDME];

const int BUFSIZEEXTERNALINTERRUPTISON = 2;
char _bufExternalInterruptIsON[BUFSIZEEXTERNALINTERRUPTISON];

//---------------------------------------------       PINS USED   ----------------------------------------------------------

//Caution: check on portableAlarm board for see what ports you can use,some of them are shorted out.

//static const uint8_t pirSensor1Pin = A4;
//
//static const uint8_t pirSensor2Pin = A5;

static const uint8_t softwareSerialExternalDevicesTxPort = A2;

static const uint8_t softwareSerialExternalDevicesPinAlarm = A4;

static const uint8_t softwareSerialExternalDevicesRxPort = 5;

static const uint8_t bluetoothKeyPin = 10;

static const uint8_t bluetoothTransistorPin = 6;

static const byte _pin_powerLed = 13;

static const byte _pin_rxSIM900 = 7;

static const byte _pin_txSIM900 = 8;

SoftwareSerial softwareSerial = SoftwareSerial(softwareSerialExternalDevicesRxPort, softwareSerialExternalDevicesTxPort);

void setSim900()
{
	mySim900 = new MySim900(_pin_rxSIM900, _pin_txSIM900, false);
	mySim900->Begin(19200);
	mySim900->IsCallDisabled(false);
	mySim900->ATCommand("AT+CPMS=\"SM\"");
	//delay(5000);
	/*if (mySim900->IsAvailable() > 0)
	{
		String s = mySim900->ReadIncomingChars2();
		Serial.println(s);
	}*/
	mySim900->ATCommand("AT+CMGF=1");
	/*delay(5000);
	if (mySim900->IsAvailable() > 0)
	{
		String s = mySim900->ReadIncomingChars2();
		Serial.println(s);
	}*/
	mySim900->ATCommand("AT+CMGD=1,4");
	/*if (mySim900->IsAvailable() > 0)
	{
		String s = mySim900->ReadIncomingChars2();
		Serial.println(s);
	}*/
}

void setup()
{
	setSim900();

	inizializePins();

	inizializeInterrupts();

	btSerial = new MyBlueTooth(&Serial, bluetoothKeyPin, bluetoothTransistorPin, 38400, 9600);

	btSerial->Reset_To_Slave_Mode();

	_oldPassword = btSerial->GetPassword();

	btSerial->ReceveMode();

	initilizeEEPromData();

	/*if (_findOutPhonesMode != 0)
	{
		_isBTSleepON = 0;
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Find activated"), BlueToothCommandsUtil::Message));
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	}*/

	//_sensitivityAlarm = 2000 + ((_precision) * 500);

	btSerial->turnOnBlueTooth();

	_whatIsHappened = F("X");

	//Stare attenti perche' richiede un ritardo che ora � dato da creazione oggetti bluetooth.
	//mySim900->WaitSMSComing();


	blinkLed();
}

void initilizeEEPromData()
{
	//EEPROM.write(0, 1);
	LSG_EEpromRW* eepromRW = new LSG_EEpromRW();

	eepromRW->eeprom_read_string(_addressStartBufPhoneNumber, _phoneNumber, BUFSIZEPHONENUMBER);


	eepromRW->eeprom_read_string(_addressStartBufPhoneNumberAlternative, _phoneNumberAlternative, BUFSIZEPHONENUMBERALTERANATIVE);


	eepromRW->eeprom_read_string(_addressDBPhoneIsON, _bufDbPhoneON, BUFSIZEDBPHONEON);
	_phoneNumbers = atoi(&_bufDbPhoneON[0]);

	/*eepromRW->eeprom_read_string(_addressStartFindMode, _bufFindMode, BUFSIZEFINDMODE);

	_findMode = atoi(&_bufFindMode[0]);*/


	/*eepromRW->eeprom_read_string(_addressStartBufTemperatureIsOn, _bufTemperatureIsOn, BUFSIZETEMPERATUREISON);

	_isTemperatureCheckOn = atoi(&_bufTemperatureIsOn[0]);
*/
	/*eepromRW->eeprom_read_string(_addressStartFindOutPhonesON, _bufFindOutPhonesON, BUFSIZEFINDOUTPHONESON);
	_findOutPhonesMode = atoi(&_bufFindOutPhonesON[0]);*/

	/*eepromRW->eeprom_read_string(_addressStartBufPirSensorIsON, _bufPirSensorIsON, BUFSIZEPIRSENSORISON);
	_isPIRSensorActivated = atoi(&_bufPirSensorIsON[0]);*/

	/*eepromRW->eeprom_read_string(_addressStartBufPrecisionNumber, _bufPrecisionNumber, BUFSIZEPRECISION);
	_precision = atoi(&_bufPrecisionNumber[0]);*/

	eepromRW->eeprom_read_string(_addressStartBufTemperatureMax, _bufTemperatureMax, BUFSIZETEMPERATUREMAX);
	_tempMax = atoi(_bufTemperatureMax);


	/*eepromRW->eeprom_read_string(_addressStartDeviceAddress, _bufDeviceAddress, BUFSIZEDEVICEADDRESS);
	_deviceAddress = String(_bufDeviceAddress);

	eepromRW->eeprom_read_string(_addressStartDeviceName, _bufDeviceName, BUFSIZEDEVICENAME);
	_deviceName = String(_bufDeviceName);

	eepromRW->eeprom_read_string(_addressStartDeviceAddress2, _bufDeviceAddress2, BUFSIZEDEVICEADDRESS);
	_deviceAddress2 = String(_bufDeviceAddress2);

	eepromRW->eeprom_read_string(_addressStartDeviceName2, _bufDeviceName2, BUFSIZEDEVICENAME);
	_deviceName2 = String(_bufDeviceName2);*/


	//eepromRW->eeprom_read_string(_addressApn, _bufApn, BUFSIZEAPN);
	//_apn = String(_bufApn);
	//_apn.trim();

	eepromRW->eeprom_read_string(_addressOffSetTemperature, _bufOffSetTemperature, BUFSIZEOFFSETTEMPERATURE);
	_offSetTempValue = atoi(_bufOffSetTemperature);

	/*eepromRW->eeprom_read_string(_addressDelayFindMe, _bufDelayFindMe, BUFSIZEDELAYFINDME);
	_delayFindMe = atoi(_bufDelayFindMe);*/

	eepromRW->eeprom_read_string(_addressExternalInterruptIsOn, _bufExternalInterruptIsON, BUFSIZEEXTERNALINTERRUPTISON);
	_isExternalInterruptOn = atoi(&_bufExternalInterruptIsON[0]);

	delete(eepromRW);

}

void inizializePins()
{
	pinMode(_pin_powerLed, OUTPUT);
	pinMode(softwareSerialExternalDevicesPinAlarm, INPUT_PULLUP);
	pinMode(A2, OUTPUT);
}

void inizializeInterrupts()
{
	///*attachInterrupt(0, motionTiltInternalInterrupt, RISING);*/
	attachInterrupt(1, motionTiltExternalInterrupt, RISING);
}

void callSim900()
{
	if (_isDisableCall) { return; }

	char phoneNumber[14];

	strcpy(phoneNumber, _prefix);

	if (_phoneNumbers == 1)
	{
		strcat(phoneNumber, _phoneNumber);
	}

	if (_phoneNumbers == 2)
	{
		strcat(phoneNumber, _phoneNumberAlternative);
	}

	mySim900->DialVoiceCall(phoneNumber);

	delay(10000);

	mySim900->ReadIncomingChars2();

}

void motionTiltExternalInterrupt() {
	if (_isExternalInterruptOn /*&& !_isPIRSensorActivated*/) {
		//_isOnMotionDetect = true;
		_isOnInterrupt = true;
	}
}

void getSignalStrength()
{
	_signalStrength = mySim900->GetSignalStrength();
}

//void turnOffBluetoohIfTimeIsOver()
//{
//	if (_findOutPhonesMode == 0
//		&& (millis() > _timeToTurnOfBTAfterPowerOn)
//		&& btSerial->isBlueToothOn()
//		&& _isBTSleepON
//		)
//	{
//		btSerial->turnOffBlueTooth();
//		digitalWrite(13, HIGH);
//		delay(5000);
//		digitalWrite(13, LOW);
//	}
//}

//void turnOnBlueToothIfMotionIsDetected()
//{
//	if (_isOnExternalDeviceAlarm
//		&& !_isAlarmOn
//		&& btSerial->isBlueToothOff()
//		&& _isBTSleepON
//		)
//	{
//		_isOnExternalDeviceAlarm = false;
//		turnOnBlueToothAndSetTurnOffTimer(false);
//	}
//}

//bool isFindOutPhonesONAndSetBluetoothInMasterMode()
//{
//	if (_isDisableCall) { return; }
//
//	if ((_findOutPhonesMode == 1 || _findOutPhonesMode == 2) && (millis() > _timeAfterPowerOnForBTFinder))
//	{
//		if (_findOutPhonesMode == 1 && !_isAlarmOn)
//		{
//			_isAlarmOn = true;
//		}
//
//		if (_isMasterMode == false)
//		{
//			btSerial->Reset_To_Master_Mode();
//			_isMasterMode = true;
//		}
//
//		/*	PIRSensor* pirSensor = new PIRSensor(0, A5, 0, 0, "PirSensor01");
//		if (humanDetectedWithFindOutPhonesON = 0 && _isPIRSensorActivated && pirSensor->isHumanDetected())
//		{
//		humanDetectedWithFindOutPhonesON = millis();
//		}
//		else if (!pirSensor->isHumanDetected())
//		{
//		humanDetectedWithFindOutPhonesON = 0;
//		}*/
//		/*if (IsDeviceDetected(String(_bufDeviceAddress), String(_bufDeviceName)))*/
//
//	/*	uint8_t i = 0;
//		while (!isDeviceDetected || i < 5)
//		{
//			isDeviceDetected = btSerial->IsDeviceDetected(_deviceAddress, _deviceName);
//			i++;
//		}
//		i = 0;*/
//
//		for (uint8_t i = 0; i < _delayFindMe; i++)
//		{
//			_isDeviceDetected = btSerial->IsDeviceDetected(_deviceAddress, _deviceName);
//			if (_isDeviceDetected) { break; }
//			if (_findOutPhonesMode == 1)
//			{
//				_deviceAddress2.trim();
//				_deviceName2.trim();
//				if (_deviceAddress2.length() > 1 && _deviceName2.length() > 1) {
//					_isDeviceDetected = btSerial->IsDeviceDetected(_deviceAddress2, _deviceName2);
//					if (_isDeviceDetected) { break; };
//				}
//			}
//		}
//
//		//bool isHumanDetected = pirSensor->isHumanDetected();
//		if (_isDeviceDetected
//			//(isDeviceDetected
//			//	/*&& (millis() - humanDetectedWithFindOutPhonesON >= 7000)
//			//	&& humanDetectedWithFindOutPhonesON != 0*/
//			//	&&
//			//	isHumanDetected
//			//	&& _isPIRSensorActivated)
//			//||
//			//(
//			//	!_isPIRSensorActivated
//			//	&&
//
//			)
//			//)
//		{
//			blinkLed();
//			//reedRelaySensorActivity(A2);
//		}
//		else
//		{
//			if (_findOutPhonesMode == 2)
//			{
//				callSim900();
//				//delay(10000);
//				_isMasterMode = false;
//			}
//		}
//
//		/*	delete(pirSensor);*/
//		return _isDeviceDetected;
//	}
//}

void readMemoryAtRunTime()
{
	//this is necessary to wait for the Arduino Leonardo to get the serial interface up and running
#if defined(__ATmega32U4__)
	while (!Serial);
#else
	delay(2000);
#endif

#ifndef RAMSTART
	serialPrint("SRAM and .data space start: ", (int)&__data_start);
#else
	serialPrint("SRAM and .data space start: ", RAMSTART);
#endif
	serialPrint(".data space end/.bss start: ", (int)&__data_end); //same as "(int)&__bss_start)"
	serialPrint(".bss space end/HEAP start: ", (int)&__heap_start); //same as "(int)&__bss_end);"
	serialPrint("HEAP end: ", (int)__brkval);
	int tempRam = freeRam();
	serialPrint("STACK start: ", temp);
	serialPrint("STACK and SRAM end: ", RAMEND);
	serialPrint("Free memory at the moment: ", tempRam);
	Serial.println("----------------------------------------------------------------------------------");
}

char problematicDevice[4];

char problematicDeviceValue[5];

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
				Serial.print("ALARM DEVICE :"); Serial.println(problematicDevice);
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
	delete mySim900;
	softwareSerial.begin(19200);

	softwareSerial.print("H"); 
	if (hour() < 10) { softwareSerial.print('0'); }
	softwareSerial.print(hour()); 
	if (minute() < 10) { softwareSerial.print('0'); }
	softwareSerial.print(minute());

	//////Pulisco buffer se ci fosse roba
	/*softwareSerial.readStringUntil('*');
	softwareSerial.flush();*/
	delay(2000);
	
	if (softwareSerial.available() > 0)
	{
		char *buffExtenalDevices = new char[100];
		softwareSerial.readStringUntil('*').toCharArray(buffExtenalDevices, 100);
		//Serial.println(buffExtenalDevices);
		isOnAlarm = chechDevicesValue(buffExtenalDevices);
		delete[] buffExtenalDevices;
	}
	setSim900();

	if (isOnAlarm)
	{
		callSim900();
	}
}

unsigned long isWaitingForSMS = 0;

void loop()
{
	/*if (digitalRead(softwareSerialExternalDevicesPinAlarm) == LOW)
	{
		getExternalDevices();
	}*/
	while (digitalRead(softwareSerialExternalDevicesPinAlarm) == LOW && !_isDisableCall)
	{
		if ((millis() - isWaitingForSMS) > 60000)
		{
			getExternalDevices();	
			isWaitingForSMS = millis();
		}
		readIncomingSMS();
	}

	readIncomingSMS();

	if (_delayForSignalStrength->IsDelayTimeFinished(true))
	{
		getSignalStrength();
	}
	
	internalTemperatureActivity();

	voltageActivity();

	blueToothConfigurationSystem();

	//readMemoryAtRunTime();
}

void isOnInterrupt()
{
	if (_isDisableCall) {
		_isOnInterrupt = false;
		return;
	}

	if (_isAlarmOn && _isExternalInterruptOn && _isOnInterrupt)
	{
		//Serial.println("lampeggio");
		blinkLed();

		detachInterrupt(0);
		detachInterrupt(1);

		//Do something

		EIFR |= 1 << INTF1; //clear external interrupt 1
		//EIFR |= 1 << INTF0; //clear external interrupt 0
		//EIFR = 0x01;
		sei();

		/*attachInterrupt(0, motionTiltInternalInterrupt, RISING);*/
		attachInterrupt(1, motionTiltExternalInterrupt, RISING);

		_isOnInterrupt = false;
	}
}

void turnOnBlueToothAndSetTurnOffTimer(bool isFromSMS)
{
	//TODO: Fare metodo su mybluetooth per riattivarlo.
	//Essenziale tutta la trafila di istruzioni altrimenti non si riattiva bluetooth
	Serial.flush();
	btSerial->Reset_To_Slave_Mode();
	//btSerial->ReceveMode();
	//btSerial->turnOnBlueTooth();
	/*if (_findOutPhonesMode == 0 || isFromSMS)
	{*/
		btSerial->ReceveMode();
		btSerial->turnOnBlueTooth();
		/*_timeToTurnOfBTAfterPowerOn = millis() + 300000;
		_timeAfterPowerOnForBTFinder = millis() + 120000;*/
	//}
	/*_isMasterMode = false;*/
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
	char* alarmStatus = new char[15];

	if (_isAlarmOn)
	{
		String(F("Alarm ON")).toCharArray(alarmStatus, 15);
	}
	else
	{
		String(F("Alarm OFF")).toCharArray(alarmStatus, 15);
	}

	char result[30];   // array to hold the result.

	strcpy(result, alarmStatus); // copy string one into the result.
	strcat(result, version); // append string two to the result.

	//ChipTemp* chipTemp = new ChipTemp();
	int internalTemperature = getTemp();//chipTemp->celsius();
	/*delete (chipTemp);*/
	delete(alarmStatus);



	String battery = calculateBatteryLevel(_voltageValue);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(result, BlueToothCommandsUtil::Title));

	//char* commandString = new char[15];

	//String(F("Configuration")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Configuration"), BlueToothCommandsUtil::Menu, F("001")));


	//String(F("Security")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Security"), BlueToothCommandsUtil::Menu, F("004")));

	if (!_isAlarmOn)
	{
		//String(F("Alarm On")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Alarm On"), BlueToothCommandsUtil::Command, F("002")));
	}
	else
	{
		//String(F("Alarm OFF")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Alarm OFF"), BlueToothCommandsUtil::Command, F("003")));
	}

	//String(F("Temp.:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Temp.:" + String(internalTemperature), BlueToothCommandsUtil::Info));

	/*btSerial->println(BlueToothCommandsUtil::CommandConstructor("Batt.value:" + String(_voltageValue), BlueToothCommandsUtil::Info));*/

	//String(F("Batt.level:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Batt.level:" + battery, BlueToothCommandsUtil::Info));

	
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("WhatzUp:" + _whatIsHappened, BlueToothCommandsUtil::Info));

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
	
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Time:" + hours + ":" + String(minutes), BlueToothCommandsUtil::Info));

	//String(F("Signal:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Signal:" + _signalStrength, BlueToothCommandsUtil::Info));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	btSerial->Flush();
	//delete(commandString);
}

void loadConfigurationMenu()
{
	//char* commandString = new char[15];
	//String(F("Configuration")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Configuration"), BlueToothCommandsUtil::Title));

	//String(F("Phone:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Phone:" + String(_phoneNumber), BlueToothCommandsUtil::Data, F("001")));

	//String(F("Ph.Altern.:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Ph.Altern.:" + String(_phoneNumberAlternative), BlueToothCommandsUtil::Data, F("099")));

	//String(F("N.Phone:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("N.Phone:" + String(_phoneNumbers), BlueToothCommandsUtil::Data, F("098")));

	//String(F("TempMax:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("TempMax:" + String(_tempMax), BlueToothCommandsUtil::Data, F("004")));

	//String(F("OffSetTemp:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("OffSetTemp:" + String(_offSetTempValue), BlueToothCommandsUtil::Data, F("095")));

	////String(F("Apn:")).toCharArray(commandString, 15);
	//btSerial->println(BlueToothCommandsUtil::CommandConstructor("Apn:" + _apn, BlueToothCommandsUtil::Data, F("096")));

	//if (!_isPIRSensorActivated && _findOutPhonesMode == 0)
	//{
	//	//String(F("Prec.:")).toCharArray(commandString, 15);
	//	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Prec.:" + String(_precision), BlueToothCommandsUtil::Data, F("002")));
	//}

	/*String(F("TempON:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + String(_isTemperatureCheckOn), BlueToothCommandsUtil::Data, F("003")));*/

	//if (_findOutPhonesMode != 2)
	//{
	//	//String(F("PIR status:")).toCharArray(commandString, 15);
	//	btSerial->println(BlueToothCommandsUtil::CommandConstructor("PIR status:" + String(_isPIRSensorActivated), BlueToothCommandsUtil::Data, F("005")));
	//}

	//if (_findOutPhonesMode != 0)
	//{
	//	//String(F("Addr:")).toCharArray(commandString, 15);
	//	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Addr:" + _deviceAddress, BlueToothCommandsUtil::Data, F("010")));

	//	//String(F("Name:")).toCharArray(commandString, 15);
	//	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Name:" + _deviceName, BlueToothCommandsUtil::Data, F("011")));

	//	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Addr2:" + _deviceAddress2, BlueToothCommandsUtil::Data, F("015")));

	//	//String(F("Name:")).toCharArray(commandString, 15);
	//	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Name2:" + _deviceName2, BlueToothCommandsUtil::Data, F("016")));

	//	//String(F("FindTime.:")).toCharArray(commandString, 15);
	//	btSerial->println(BlueToothCommandsUtil::CommandConstructor("FindLoop:" + String(_delayFindMe), BlueToothCommandsUtil::Data, F("094")));
	//}

	////String(F("Find phone:")).toCharArray(commandString, 15);
	//btSerial->println(BlueToothCommandsUtil::CommandConstructor("FindMode:" + String(_findOutPhonesMode), BlueToothCommandsUtil::Data, F("012")));

	////String(F("Ext.Int:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Ext.Int:" + String(_isExternalInterruptOn), BlueToothCommandsUtil::Data, F("013")));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	//delete(commandString);
}

void blueToothConfigurationSystem()
{
	LSG_EEpromRW* eepromRW = new LSG_EEpromRW();
	String _bluetoothData = "";
	if (btSerial->available())
	{
		_bluetoothData = btSerial->readString();
		//BluetoothData.trim();

		//ROOT: Main
#pragma region Main Menu-#0
		if (_bluetoothData.indexOf(F("#0")) > -1)
		{
			loadMainMenu();
		}


#pragma region Commands

		if (_bluetoothData.indexOf(F("C002")) > -1)
		{
			//do something
			//digitalWrite(_pin_powerLed, HIGH);
			_isAlarmOn = true;
			/*_isOnExternalDeviceAlarm = false;*/
			/*_timeAfterPowerOnForBTFinder = 0;*/
			//btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Portable Alarm ON"), BlueToothCommandsUtil::Title));
		/*	btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Alarm ON"), BlueToothCommandsUtil::Message));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));*/
			loadMainMenu();
		}

		if (_bluetoothData.indexOf(F("C003")) > -1)
		{
			//do something
			_isAlarmOn = false;

			//digitalWrite(_pin_powerLed, LOW);
			//btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Portable Alarm OFF"), BlueToothCommandsUtil::Title));
		/*	btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Alarm OFF"), BlueToothCommandsUtil::Message));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));*/
			loadMainMenu();
		}
#pragma endregion

#pragma region Data


#pragma endregion


#pragma endregion

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
				/*const int BUFSIZEPHONENUMBER = 11;
				char _bufPhoneNumber[BUFSIZEPHONENUMBER];*/

				splitString.toCharArray(_phoneNumber, BUFSIZEPHONENUMBER);
				eepromRW->eeprom_write_string(1, _phoneNumber);
				//_phoneNumber = splitString;
			}
			loadConfigurationMenu();
		}

		//if (_bluetoothData.indexOf(F("D097")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	if (isValidNumber(splitString))
		//	{
		//		/*const int BUFSIZEFINDMODE = 2;
		//		char _bufFindMode[BUFSIZEFINDMODE];*/

		//		splitString.toCharArray(_bufFindMode, BUFSIZEFINDMODE);
		//		eepromRW->eeprom_write_string(_addressStartFindMode, _bufFindMode);
		//		_findMode = atoi(&_bufFindMode[0]);
		//	}
		//	updateCommand();
		//	/*btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("FindMode updated"), BlueToothCommandsUtil::Message));
		//	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));*/
		//}

		if (_bluetoothData.indexOf(F("D094")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*const int BUFSIZEDBPHONEON = 2;
				char _bufDbPhoneON[BUFSIZEDBPHONEON];*/

				splitString.toCharArray(_bufDelayFindMe, BUFSIZEDEVICEADDRESS);
				eepromRW->eeprom_write_string(_addressDelayFindMe, _bufDelayFindMe);
				_delayFindMe = atoi(&_bufDelayFindMe[0]);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D095")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufOffSetTemperature, BUFSIZEOFFSETTEMPERATURE);
				eepromRW->eeprom_write_string(_addressOffSetTemperature, _bufOffSetTemperature);
				_offSetTempValue = atoi(&_bufOffSetTemperature[0]);
			}

			loadConfigurationMenu();
		}

		//if (_bluetoothData.indexOf(F("D096")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	/*if (isValidNumber(splitString))
		//	{*/
		//	/*const int BUFSIZEPHONENUMBERALTERNATIVE = 11;
		//	char _bufPhoneNumberAlternative[BUFSIZEPHONENUMBERALTERNATIVE];*/

		//	splitString.toCharArray(_bufApn, BUFSIZEAPN);
		//	eepromRW->eeprom_write_string(_addressApn, _bufApn);
		//	_apn = splitString;
		//	//}
		//	loadConfigurationMenu();
		//}

		if (_bluetoothData.indexOf(F("D098")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*const int BUFSIZEDBPHONEON = 2;
				char _bufDbPhoneON[BUFSIZEDBPHONEON];*/

				splitString.toCharArray(_bufDbPhoneON, BUFSIZEDBPHONEON);
				eepromRW->eeprom_write_string(_addressDBPhoneIsON, _bufDbPhoneON);
				_phoneNumbers = atoi(&_bufDbPhoneON[0]);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D099")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*const int BUFSIZEPHONENUMBERALTERNATIVE = 11;
				char _bufPhoneNumberAlternative[BUFSIZEPHONENUMBERALTERNATIVE];*/

				splitString.toCharArray(_phoneNumberAlternative, BUFSIZEPHONENUMBERALTERANATIVE);
				eepromRW->eeprom_write_string(_addressStartBufPhoneNumberAlternative, _phoneNumberAlternative);
				//_phoneNumberAlternative = splitString;
			}
			loadConfigurationMenu();
		}

		//if (_bluetoothData.indexOf(F("D002")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	if (isValidNumber(splitString))
		//	{
		//		/*char _bufPrecisionNumber[2];*/
		//		splitString.toCharArray(_bufPrecisionNumber, 2);
		//		eepromRW->eeprom_write_string(12, _bufPrecisionNumber);
		//		_precision = atoi(&_bufPrecisionNumber[0]);
		//		_sensitivityAlarm = 2000 + ((_precision) * 500);
		//	}
		//	loadConfigurationMenu();
		//}

		//if (_bluetoothData.indexOf(F("D003")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	if (isValidNumber(splitString))
		//	{
		//		/*const int BUFSIZETEMPERATUREISON = 2;
		//		char _bufTemperatureIsOn[BUFSIZETEMPERATUREISON];*/

		//		splitString.toCharArray(_bufTemperatureIsOn, BUFSIZETEMPERATUREISON);
		//		eepromRW->eeprom_write_string(14, _bufTemperatureIsOn);
		//		_isTemperatureCheckOn = atoi(&_bufTemperatureIsOn[0]);
		//	}
		//	updateCommand();
		//}

		//if (_bluetoothData.indexOf(F("D033")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	if (isValidNumber(splitString))
		//	{
		//		/*const int BUFSIZEBTSLEEPISON = 2;
		//		char _bufBTSleepIsON[BUFSIZEBTSLEEPISON];*/

		//		splitString.toCharArray(_bufBTSleepIsON, BUFSIZEBTSLEEPISON);
		//		eepromRW->eeprom_write_string(_addressStartBTSleepIsON, _bufBTSleepIsON);
		//		_isBTSleepON = atoi(&_bufBTSleepIsON[0]);
		//	}
		//	updateCommand();
		//	/*btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("BTSleep updated"), BlueToothCommandsUtil::Message));
		//	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));*/
		//}

		if (_bluetoothData.indexOf(F("D004")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*const int BUFSIZETEMPERATUREVALUE = 3;
				char _bufTemperatureValue[BUFSIZETEMPERATUREVALUE];*/

				splitString.toCharArray(_bufTemperatureMax, BUFSIZETEMPERATUREMAX);
				eepromRW->eeprom_write_string(16, _bufTemperatureMax);
				_tempMax = atoi(_bufTemperatureMax);
			}
			loadConfigurationMenu();
		}

		//if (_bluetoothData.indexOf(F("D005")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	if (isValidNumber(splitString))
		//	{
		//		/*	const int BUFSIZEPIRSENSORISON = 2;
		//			char _bufPirSensorIsON[BUFSIZEPIRSENSORISON];*/
		//		splitString.toCharArray(_bufPirSensorIsON, BUFSIZEPIRSENSORISON);
		//		eepromRW->eeprom_write_string(19, _bufPirSensorIsON);
		//		_isPIRSensorActivated = atoi(&_bufPirSensorIsON[0]);
		//	}
		//	loadConfigurationMenu();
		//}

		//if (_bluetoothData.indexOf(F("D010")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	if (isValidNumber(splitString))
		//	{
		//		/*const int BUFSIZEDEVICEADDRESS = 13;
		//		char _bufDeviceAddress[BUFSIZEDEVICEADDRESS];*/

		//		splitString.toCharArray(_bufDeviceAddress, BUFSIZEDEVICEADDRESS);
		//		eepromRW->eeprom_write_string(_addressStartDeviceAddress, _bufDeviceAddress);
		//		_deviceAddress = splitString;
		//	}
		//	loadConfigurationMenu();
		//}

		//if (_bluetoothData.indexOf(F("D011")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	if (isValidNumber(splitString))
		//	{
		//		/*const int BUFSIZEDEVICENAME = 15;
		//		char _bufDeviceName[BUFSIZEDEVICENAME];*/

		//		splitString.toCharArray(_bufDeviceName, BUFSIZEDEVICENAME);
		//		eepromRW->eeprom_write_string(_addressStartDeviceName, _bufDeviceName);
		//		_deviceName = splitString;
		//	}
		//	loadConfigurationMenu();
		//}

		/*if (_bluetoothData.indexOf(F("D015")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufDeviceAddress2, BUFSIZEDEVICEADDRESS);
				eepromRW->eeprom_write_string(_addressStartDeviceAddress2, _bufDeviceAddress2);
				_deviceAddress2 = splitString;
			}
			loadConfigurationMenu();
		}*/

		/*if (_bluetoothData.indexOf(F("D016")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufDeviceName2, BUFSIZEDEVICENAME);
				eepromRW->eeprom_write_string(_addressStartDeviceName2, _bufDeviceName2);
				_deviceName2 = splitString;
			}
			loadConfigurationMenu();
		}*/

		//if (_bluetoothData.indexOf(F("D012")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	if (isValidNumber(splitString))
		//	{
		//		/*const int BUFSIZEFINDOUTPHONESON = 2;
		//		char _bufFindOutPhonesON[BUFSIZEFINDOUTPHONESON];*/

		//		splitString.toCharArray(_bufFindOutPhonesON, BUFSIZEFINDOUTPHONESON);
		//		eepromRW->eeprom_write_string(_addressStartFindOutPhonesON, _bufFindOutPhonesON);
		//		_findOutPhonesMode = atoi(&_bufFindOutPhonesON[0]);
		//		if (_findOutPhonesMode != 0)
		//		{
		//			_isBTSleepON = 0;
		//			if (_findOutPhonesMode == 2)
		//			{
		//				/*_isPIRSensorActivated = 0;*/
		//			}
		//		}
		//		else
		//		{
		//			/*const int BUFSIZEBTSLEEPISON = 2;
		//			char _bufBTSleepIsON[BUFSIZEBTSLEEPISON];*/
		//			/*eepromRW->eeprom_read_string(_addressStartBTSleepIsON, _bufBTSleepIsON, BUFSIZEBTSLEEPISON);
		//			_isBTSleepON = atoi(&_bufBTSleepIsON[0]);*/

		//			_isBTSleepON = 1;
		//		}

		//	}

		//	loadConfigurationMenu();
		//}

		if (_bluetoothData.indexOf(F("D013")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufExternalInterruptIsON, BUFSIZEEXTERNALINTERRUPTISON);
				eepromRW->eeprom_write_string(_addressExternalInterruptIsOn, _bufExternalInterruptIsON);
				_isExternalInterruptOn = atoi(&_bufExternalInterruptIsON[0]);
			}
			loadConfigurationMenu();
		}

#pragma endregion

#pragma Configuration Menu endregion


#pragma region Security-M004
		if (_bluetoothData.indexOf(F("M004")) > -1)
		{
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Security"), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw.:"), BlueToothCommandsUtil::Menu, F("005")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change name:"), BlueToothCommandsUtil::Menu, F("006")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}
#pragma region Menu
		if (_bluetoothData.indexOf(F("M005")) > -1)
		{
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Insert old passw.:"), BlueToothCommandsUtil::Data, F("006")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}

		if (_bluetoothData.indexOf(F("M006")) > -1)
		{
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Insert name:"), BlueToothCommandsUtil::Data, F("007")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
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
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Insert new passw:"), BlueToothCommandsUtil::Data, F("008")));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			}
			else
			{
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Wrong passw:"), BlueToothCommandsUtil::Message));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			}

		}

		if (_bluetoothData.indexOf(F("D008")) > -1)
		{
			_newPassword = splitStringIndex(_bluetoothData, ';', 1);
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Confirm pass:"), BlueToothCommandsUtil::Data, F("009")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}

		if (_bluetoothData.indexOf(F("D009")) > -1)
		{
			if (_newPassword == splitStringIndex(_bluetoothData, ';', 1))
			{
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("changed:"), BlueToothCommandsUtil::Message));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
				delay(2000);
				btSerial->SetPassword(_newPassword);
				_oldPassword = _newPassword;
			}

			else
			{
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("passw. doesn't match"), BlueToothCommandsUtil::Message));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
				btSerial->println("D006");
			}
		}


		if (_bluetoothData.indexOf(F("D007")) > -1)
		{
			String btName = splitStringIndex(_bluetoothData, ';', 1);

			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("changed:"), BlueToothCommandsUtil::Message));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			delay(2000);
			btSerial->SetBlueToothName(btName);
		}


#pragma endregion

#pragma endregion

		delay(100);
	}
	delete(eepromRW);
}

boolean isValidNumber(String str)
{
	for (byte i = 0; i < str.length(); i++)
	{
		if (isDigit(str.charAt(i))) return true;
	}
	return false;
}

//void pirSensorActivity()
//{
//	//Serial.print("analogRead(A4) = "); Serial.println(analogRead(A4));
//	//Serial.print("digitalRead(A4) = "); Serial.println(digitalRead(A4));
//	//Serial.println("---------------------------------------------------------");
//	//Serial.print("analogRead(A5) = "); Serial.println(analogRead(A5));
//	//Serial.print("digitalRead(A5) = "); Serial.println(digitalRead(A5));
//
//	//delay(1000);
//	if (_isDisableCall) { return; }
//	if (_isPIRSensorActivated && _isAlarmOn)
//	{
//		if (digitalRead(pirSensor2Pin) && digitalRead(pirSensor1Pin))
//		{
//			blinkLed();
//			_whatIsHappened = F("P");
//
//			if (isAM() && hour() < 6)
//			{
//				if ((millis() - _pirSensorTime) > 40000)
//				{
//					_pirSensorTime = 0;
//				}
//				if (_pirSensorTime != 0)
//				{
//					//Serial.println("Alarme scattato");
//					callSim900();
//					_isMasterMode = false;
//				}
//				else
//				{
//					_pirSensorTime = millis();
//				}
//
//				delay(15000);
//			}
//			else if (_findOutPhonesMode == 1 && _isDeviceDetected && digitalRead(3))
//			{
//				//Aggiungere codice che gestisce interrupt pin aperto.
//				//reedRelaySensorActivity(A5);
//				delay(15000);
//			}
//		}
//		/*unsigned int count0 = 0;
//		for (unsigned int i = 0; i < 110; i++)
//		{
//		if (digitalRead(5))
//		{
//		count0++;
//		}
//		}
//
//		if (count0 > 100)
//		{
//		blinkLed();
//		_whatIsHappened = F("P");
//		reedRelaySensorActivity(A4);
//		callSim900('1');
//		}*/
//	}
//}

//void reedRelaySensorActivity(uint8_t pin)
//{
//	pinMode(pin, OUTPUT);
//	digitalWrite(pin, HIGH);
//	delay(1000);
//	digitalWrite(pin, LOW);
//}

void internalTemperatureActivity()
{
	if (_delayForTemperature->IsDelayTimeFinished(true))
	{
		/*if (_isTemperatureCheckOn == 0) return;*/

		/*ChipTemp* chipTemp = new ChipTemp();*/

		if ((uint8_t)getTemp() > _tempMax)
		{

			_whatIsHappened = F("T");
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
			callSim900();
		}
	}
}

void readIncomingSMS()
{
	mySim900->ATCommand("AT+CMGL");//=\"REC UNREAD\"");
	//mySim900->ATCommand("AT+CMGR=1");
	delay(100);
	/**/

	if (mySim900->IsAvailable() > 0)
	{
		String response = mySim900->ReadIncomingChars2();
		delay(500);
		//Serial.print("####"); Serial.print(response); Serial.println("####");
		response.trim();
		//if (response.substring(0, 5) == F("+CMT:"))
		//if (response.indexOf("+CMT:") != -1)
		if (response.indexOf("+CMGL:") != -1)
		{
			blinkLed();

			/*Serial.println(response.substring(51, 61));
			Serial.println(response.substring(36, 46));*/

			if (response.substring(36, 46) != _phoneNumber &&
				response.substring(51, 61) != _phoneNumber &&
				response.substring(36, 46) != _phoneNumberAlternative &&
				response.substring(51, 61) != _phoneNumberAlternative)
			{
				//Serial.println("Numero errato");
				return;
			}
			int index = response.lastIndexOf('"');
			String smsCommand = response.substring(index + 1, index + 8);
			smsCommand.trim();
			//Serial.println(smsCommand);
			delay(1000);
			listOfSmsCommands(smsCommand);
		}
	}
}

void listOfSmsCommands(String command)
{
	command.trim();
	//Imposta ora di sistema
	if (command.startsWith("H"))
	{
		String hour = command.substring(1, 3);
		//Serial.println(hour);
		String minute = (command.substring(3, 5));
		//Serial.println(minute);
		setTime(hour.toInt(), minute.toInt(), 1, 1, 1, 2019);
		callSim900();
	}

	//Attiva chiamate
	if (command == F("Ac"))
	{
		_isDisableCall = false;
		callSim900();
	}

	//Disattiva chiamate
	if (command == F("Dc"))
	{
		callSim900();
		_isDisableCall = true;
	}
	////Allarme ON
	//if (command == F("Ao"))
	//{
	//	_isDisableCall = false;
	//	callSim900('0');
	//	if (_findOutPhonesMode == 1 || _findOutPhonesMode == 2)
	//	{
	//		_timeAfterPowerOnForBTFinder = 0;
	//	}
	//	_isAlarmOn = true;
	//}

	//Accende bluetooth
	if (command == F("Ab"))
	{
		turnOnBlueToothAndSetTurnOffTimer(true);
	}
	//Accende led
	if (command == F("Al"))
	{
		_isBlueLedDisable = false;
		callSim900();
	}
	//Check system
	if (command == F("Ck"))
	{
		callSim900();
	}
	////Position enable.
	//if (command == F("Pe"))
	//{
	//	_isPositionEnable = true;
	//}
	////Position disable.
	//if (command == F("Pd"))
	//{
	//	_isPositionEnable = false;
	//}
	//Attiva funzione non vedermi
	//if (command == F("Nv"))
	//{
	//	_findOutPhonesMode = 1;
	//	_isBTSleepON = false;
	//	callSim900();
	//}

	////Attiva External interrupt
	//if (command == F("Ex"))
	//{
	//	_isExternalInterruptOn = 1;
	//	callSim900();
	//}

	////Disattiva External interrupt
	//if (command == F("Ey"))
	//{
	//	_isExternalInterruptOn = 0;
	//	callSim900();
	//}

	////Attiva motion detect senza bluetooth
	//if (command == F("Md"))
	//{
	//	_isBTSleepON = true;
	//	_isPIRSensorActivated = 0;
	//	_findOutPhonesMode = 0;
	//	activateFunctionAlarm();
	//}

	//Attiva pir sensor senza bluetooth
	//if (command == F("Wc"))
	//{
	//	_isBTSleepON = true;
	//	_isPIRSensorActivated = 1;
	//	_findOutPhonesMode = 0;
	//	activateFunctionAlarm();
	//}

	////Find me
	//if (command == F("Fm"))
	//{
	//	_isBTSleepON = false;
	//	_findOutPhonesMode = 2;
	//	activateFunctionAlarm();
	//}
}

void activateFunctionAlarm()
{
	/*_timeToTurnOfBTAfterPowerOn = 0;
	_timeAfterPowerOnForBTFinder = 0;*/
	_isDisableCall = false;
	_isAlarmOn = true;
	callSim900();
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

//unsigned int offSetTempValue(double externalTemperature)
//{
//	unsigned int wADC;
//	double t;
//	// The internal temperature has to be used
//	// with the internal reference of 1.1V.
//	// Channel 8 can not be selected with
//	// the analogRead function yet.
//
//	// Set the internal reference and mux.
//	ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
//	ADCSRA |= _BV(ADEN);  // enable the ADC
//
//	delay(20);            // wait for voltages to become stable.
//
//	ADCSRA |= _BV(ADSC);  // Start the ADC
//
//						  // Detect end-of-conversion
//	while (bit_is_set(ADCSRA, ADSC));
//
//	// Reading register "ADCW" takes care of how to read ADCL and ADCH.
//	wADC = ADCW;
//
//	// The offset of 324.31 could be wrong. It is just an indication.
//	//t = (wADC - _offSetTempValue) / 1.22;
//
//	// The returned temperature is in degrees Celsius.
//	return (-(externalTemperature * 1.22) + wADC);
//}

//print function
void serialPrint(String tempString, int tempData) {
	Serial.print(tempString);
	Serial.print(tempData, DEC);
	Serial.print(" $");
	Serial.println(tempData, HEX);
}

//computes the free memory (from JeeLabs)
int freeRam() {
	int v;
	temp = (int)&v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}