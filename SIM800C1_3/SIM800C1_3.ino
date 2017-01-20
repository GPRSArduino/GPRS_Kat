/*
��������� �������� ������ �� ������ GPRS
06.01.2017�.

���������:
1) ���� ��������, ��� ��� ������ ���������� ��� ���������, ����������� �� ���� ,��� ���.
������ ��������, ��� ������� ��������� ����� ����� ���800 ������� ������� ������ (���� ����), � ����� ������ ������.
����� �������� ��������� ���������� �����, �� ��� ���� ����� ��������� ���800 �� �����, ��� ��������, ��� �������� ����� ���800 �� ����.
��������� � ���800 ���� �������, ����� �������� ������, ��������� �� �� ������ � ����, ������� �� ����� � ��������.
����� ����, ����� ������ ����� �������� ����� ��������� ������ (��� ������ �������� �������� ������, ���� ��� ����������� � ����). ��� ��������?

2) � ���� ��������� ����� ������ �� �����? ��� ��� ��������? ������������ �������� ����� ����� ����� "������� ������"

3) ���������� �������� ������ �� ������ ���������, ��������� "����� ���������", � ����� � ������� ���� �����������������.
�.�., ���������� ��� ��� 3-4 ���� ��������� ��� ���� �������. ������ ��� ����� 20 ������ � �������� �� ����� - 10 �����. 20*10 = 200 �����. �.�., 3,3 ����. 
����������, ��� �������� �����, � ����� ���������� ������������ �� �������� ������.
������ ��� ���������� - ���������� ������ ���������������� (�� �������) � ������ ���������� � ��������� ������

4) �� ���������, �������� � ��� ���������� ��������� ���������, ����� ���������� ���������� ��� ������������� ����������: 

1.  ����� �� ������ ������ ��� ������ ��������, ���� ���������� �������� � 1 ������ ����� ������ � ������ �������� ������ �� ������;
���� ��� ����� 1� ������� ������ �� ������ ������ ���������� �������� (������ ������ �� 2� �������� ������ �������� ,
�������������� ���� �������� ����� �������� ����� � ����� �� ������� ��� �� ����������). 

2.  ������� ��������� ��������� �������� ������ ��� ������ ��������� � �������. ��������� � �������,
������ 1� �������� ����� � ������, � 2� � ����� � � ����. �����, ���� ���� ������, ������� ���, 
����� ���������� �� �������� �� ������, � ������ ������� ������ 3-5 ���, ����� ����� � ��������� ������, � ������� �����,
� ����� ���� ��� ������ �������� �������. ����� �������, ���� ���� �� ���� ��� ���������� �� ������������� � �� ������� ����� �� �������, � LOOP �� ������. 

3.   ������� �� ����� ������

4.  ������� ���������� �������� ����� ��������� ���������� , ���������� � ������, � �� ������� �� �������������.
��.. ����� ������ ��������� ����� ������ ���� 0150@05@06+79852517615@07TempRemote � ��� �����






*/

#include "SIM800.h"
#include <SoftwareSerial.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

#define con Serial
#define speed_Serial 19200
static const char* url1 = "http://vps3908.vps.host.ru/recieveReadings.php";
//static const char* url2 = "AT+CIPPING=\"www.yandex.ru\"";
//static const char* url3 = "www.yandex.ru";


#define PIN_TX           7                             // ����������  � ������ 7 ������ RX ������ GPRS
#define PIN_RX           8                              // ����������  � ������ 8 ������ TX ������ GPRS

SoftwareSerial SIM800CSS = SoftwareSerial(PIN_RX, PIN_TX);
SoftwareSerial *GPRSSerial = &SIM800CSS;



#define PWR_On           5                          // ��������� ������� ������ SIM800
#define SIM800_RESET_PIN 6                          // ����� ������ SIM800
#define LED13           13                          // ��������� �����������
#define NETLIGHT         3                          // ��������� NETLIGHT
#define STATUS           9                          // ��������� STATUS

#define port1           11                          // ���� ���������� �������� ������������ (��������������)
#define port2           12                          // ���� ���������� �������� ������������ (��������������)

													// ����������  � ������ 8 ������ TX ������ GPRS. ���������� � ���������� SIM800.h  
													// ����������  � ������ 7 ������ RX ������ GPRS. ���������� � ���������� SIM800.h
//#define COMMON_ANODE
#define LED_RED      10                             // ��������� ����������� RED
#define LED_BLUE     15                             // ��������� ����������� BLUE
#define LED_GREEN    14                             // ��������� ����������� GREEN

#define COLOR_NONE LOW, LOW, LOW
#define COLOR_RED HIGH, LOW, LOW
#define COLOR_GREEN LOW, HIGH, LOW
#define COLOR_BLUE LOW, LOW, HIGH

volatile int state = LOW;
volatile int state_device = 0;                     // ��������� ������ ��� ������� 
												   // 1 - �� ��������������� � ����, �����
												   // 2 - ���������������� � ����
												   // 3 - GPRS ����� �����������
volatile int metering_NETLIGHT = 0;
volatile unsigned long metering_temp = 0;

bool start_error = false;                         // ���� ����������� ������ ������ ��� ������.

CGPRS_SIM800 gprs;
uint32_t count  = 0;
uint32_t errors = 0;
//String imei = "";
String CSQ = "";                                    // ������� ������� ������
String SMS_center = "";
String zero_tel   = "";
String imei = "861445030362268";                  // ���� IMEI
//#define DELIM "&"
#define DELIM "@"

unsigned long time;                                 // ���������� ��� ��������� ������
unsigned long time_day = 86400;                     // ���������� ������ � ������
unsigned long previousMillis = 0;
unsigned long interval = 30;                        // �������� �������� ������ 30 ������
//unsigned long interval = 300;                     // �������� �������� ������ 5 �����
bool time_set = false;                              // ����������� �������� �������� ���


													// ����� ���������
#define MESSAGE_LENGTH 20

													// ����� ��������� � ������ ���-�����
int messageIndex = 0;

// ����� ���������
char message[MESSAGE_LENGTH];
// �����, � �������� ������ ���������
//char phone[16];
// ���� �������� ���������
char datetime[24];
char data_tel[16];                                  // ����� ��� ������ �������








int Address_tel1       = 100;                         // ����� � EEPROM �������� 1
int Address_tel2       = 120;                         // ����� � EEPROM �������� 2
int Address_tel3       = 140;                         // ����� � EEPROM �������� 3
int Address_errorAll   = 160;                         // ����� � EEPROM �������� ����� ������
int Address_port1      = 180;                         // ����� � EEPROM ���� ������ (��������������)
int Address_port2      = 190;                         // ����� � EEPROM ���� ������ (��������������)
int Address_interval   = 200;                         // ����� � EEPROM �������� ���������
int Address_SMS_center = 220;                         // ����� � EEPROM SMS ������

//char data_tel[16];                                  // ����� ��� ������ �������

//int dataport1 = 0;                                  // ���� ������ (��������������)
//int dataport2 = 0;                                  // ���� ������ (��������������)


uint8_t oneWirePins[]={16, 17, 4};                     //������ �������� ����������� DS18x20. ����������� ������ ����� ������������� ����������� �������� � ������.
													   // ������ ������ ���� ���������� ������.
uint8_t oneWirePinsCount=sizeof(oneWirePins)/sizeof(int);

OneWire ds18x20_1(oneWirePins[0]);
OneWire ds18x20_2(oneWirePins[1]);
OneWire ds18x20_3(oneWirePins[2]);
DallasTemperature sensor1(&ds18x20_1);
DallasTemperature sensor2(&ds18x20_2);
DallasTemperature sensor3(&ds18x20_3);


void(* resetFunc) (void) = 0;                         // ��������� ������� reset

 void setColor(bool red, bool green, bool blue)       // ��������� ����� �������� ������������ ����������.
 {
	  #ifdef COMMON_ANODE
		red = !red;
		green = !green;
		blue = !blue;
	  #endif
	  digitalWrite(LED_RED, red);
	  digitalWrite(LED_GREEN, green);
	  digitalWrite(LED_BLUE, blue);    
 }


void sendTemps()
{
	//Serial.println("\nTemp");
	sensor1.requestTemperatures();
	sensor2.requestTemperatures();
	sensor3.requestTemperatures();
	float t1 = sensor1.getTempCByIndex(0);
	float t2 = sensor2.getTempCByIndex(0);
	float t3 = sensor3.getTempCByIndex(0);
	float tsumma = t1 + t2 + t3 + 88.88;
	int signal = gprs.getSignalQuality();
	int error_All = 0;
	EEPROM.get(Address_errorAll, error_All);
	//String toSend = formHeader()+DELIM+"temp1="+String(t1)+DELIM+"temp2="+String(t2)+DELIM+"tempint="+String(t3)+ DELIM+"slevel="+String(signal)+DELIM+"ecs="+String(errors)+DELIM+"ec="+String(error_All)+formEnd();
	String toSend = formHeader() + DELIM + String(t1) + DELIM + String(t2) + DELIM + String(t3) + DELIM + String(signal) + DELIM + String(errors) + DELIM + String(error_All) + formEnd() + DELIM + String(tsumma);


	//Serial.println(toSend);

	Serial.println(toSend.length());
	gprs_send(toSend);
}

//gprs.ping("www.yandex.ru");




String formHeader() 
{
  String uptime = "17/01/01,10:10:10 00";
  GSM_LOCATION loc;                               // �������� ����� �� ���������
  if (gprs.getLocation(&loc)) 
  {
   uptime  = String(loc.year)+'/'+ String(loc.month)+'/'+ String(loc.day)+','+String(loc.hour)+':'+ String(loc.minute)+':'+String(loc.second)+" 00";
  //  uptime  = "date="+ String(loc.year)+'_'+ String(loc.month)+'_'+ String(loc.day)+','+String(loc.hour)+':'+ String(loc.minute)+':'+String(loc.second)+"00";
  }
 // return "imei=" + imei + DELIM + uptime;
   return "t1=" + imei + DELIM + uptime;
}
String formEnd() 
{
	char buf[13] ;

	EEPROM.get(Address_tel1, buf);
	String master_tel1(buf);
	//con.println(master_tel1);

	EEPROM.get(Address_tel2, buf);
	String master_tel2(buf);
	//con.println(master_tel2);
	
	EEPROM.get(Address_tel3, buf);
	String master_tel3(buf);
	//con.println(master_tel3);

	 EEPROM.get(Address_SMS_center, SMS_center);   //�������� �� EEPROM ��� �����


	if(EEPROM.read(Address_port1))
	 {
 
	 }
	 else
	 {
		//dataport1 = digitalRead(port1);
		//Serial.println(dataport1);
	 }

	 if(EEPROM.read(Address_port2))
	 {
 
	 }
	 else
	 {
		/*dataport2 = digitalRead(port2);
		Serial.println(dataport2);*/
	 }
	String mytel = "mytel=" + master_tel1;
	String tel1 = "tel1=" + master_tel2;
	String tel2 = "tel2=" + master_tel3;

	//return DELIM + mytel + DELIM +tel1 + DELIM + tel2;
	return DELIM + master_tel1 + DELIM + master_tel2 + DELIM + master_tel3 + DELIM + SMS_center;

}



void gprs_send(String data) 
{
  con.print(F("Requesting "));               //con.print("Requesting ");
  con.print(url1);
  con.print('?');
  con.println(data);
  gprs.httpConnectStr(url1, data);
  count++;
  if(count >1)
  {
	  start_error = true;
  }
  while (gprs.httpIsConnected() == 0) 
  {
	con.write('.');
	for (byte n = 0; n < 25 && !gprs.available(); n++) 
	{
	  delay(10);
	}
  }
  if (gprs.httpState == HTTP_ERROR) 
  {
	con.println(F("Connect error"));
	//if(start_error)                        // ������������ ������ ��� ������ �������
	//{
		errors++;
		errorAllmem();
	//}
	if (errors > 20)
	  {
			//con.println("Number of transmission errors exceeded");
			resetFunc();          // �������� reset ����� 30 ������
	  }
	delay(3000);
	return; 
  }
  
  con.println();
  gprs.httpRead();
  int ret;
  while ((ret = gprs.httpIsRead()) == 0) 
  {
	// ����� ������� ���-�� �����, �� ����� ��������
  }
  if (gprs.httpState == HTTP_ERROR) 
  {
	errors++;
	errorAllmem();
	if (errors > 20)
	  {
			con.println(F("The number of server errors exceeded 20"));
			resetFunc();         // �������� reset ����� 20 ������
	  }
	delay(3000);
	return; 
  }

  // ������ �� �������� ��������� �� �����.
   con.print(F("[Payload] "));                            //con.print("[Payload] ");
   con.println(gprs.buffer);
   String command = gprs.buffer;                          // �������� ������ ������ � �������
   String commEXE = command.substring(0, 2);              // �������� ������ � ��������
   int var = commEXE.toInt();                             // �������� ����� �������. ������������� ������ ������� � ����� 

   if(var == 1)                                           // ��������� ������� 1
	{
		String commData = command.substring(2, 10);       // �������� ������ � �������
		unsigned long interval1 = commData.toInt();       // ������������� ������ ������ � ����� 
		con.println(interval1);
		if(interval1 > 10 && interval1 < 86401)           // ���������� ��������� �� 10  ������ �� 24 �����.
		{
		  if(interval1!=interval)                         // ���� ���������� �� ���������� - �� ������ � EEPROM
		  {
			 if(!time_set)                                // ���� ��� ������� �������� ��������� �� ��� 
			 {
				interval = interval1;                     // ����������� �������� �������� �� ������
				EEPROM.put(Address_interval, interval);   // �������� �������� EEPROM , ���������� �� �������
			 }
		  }
		}
		con.println(interval);
	}
	
	else if(var == 2)                                  // ��������� ������� 2
	{
		command.remove(0, 2);                          // �������� ������ ������ �������� �� �������
		EEPROM.get(Address_tel1, data_tel);            // �������� ����� �������� �� EEPROM
		String num_tel(data_tel);
		if (command != num_tel)                        // ���� ���������� �� ���������� - �� ������ � EEPROM
		{
			 con.println(F("no compare"));               //Serial.println("no compare");
			for(int i=0;i<13;i++)
			{
				EEPROM.write(i+Address_tel1,command[i]);
			}
		}
	}
	
	else if(var == 3)                                  // ��������� ������� 3
	{
		command.remove(0, 2);                          // �������� ������ ������ �������� �� �������
		EEPROM.get(Address_tel2, data_tel);            // �������� ����� �������� �� EEPROM
		String num_tel(data_tel);
		if (command != num_tel)                        // ���� ���������� �� ���������� - �� ������ � EEPROM
		{
			con.println(F("no compare"));
			for(int i=0;i<13;i++)
			{
				EEPROM.write(i+Address_tel2,command[i]);
			}
		}
	}
	else if(var == 4)                                  // ��������� ������� 4
	{
		command.remove(0, 2);                          // �������� ������ ������ �������� �� �������
		EEPROM.get(Address_tel3, data_tel);            // �������� ����� �������� �� EEPROM
		String num_tel(data_tel);
		if (command != num_tel)                        // ���� ���������� �� ���������� - �� ������ � EEPROM
		{
			Serial.println(F("no compare"));
			for(int i=0;i<13;i++)
			{
				EEPROM.write(i+Address_tel3,command[i]);
			}
		}
	}
	else if(var == 5)                                  // ��������� ������� 5
	{
		  EEPROM.put(Address_errorAll, 0);             // �������� ������� ������
	}
	else if(var == 6)                                  // ��������� ������� 6
	{
		command.remove(0, 2);                          // �������� ������ ������ �������� �� �������
		EEPROM.get(Address_SMS_center, data_tel);      // �������� �� EEPROM ��� �����
		String num_tel(data_tel);
		if (command != num_tel)                        // ���� ���������� �� ���������� - �� ������ � EEPROM
		{
			Serial.println(F("no compare"));
			for(int i=0;i<13;i++)
			{
				EEPROM.write(i+Address_SMS_center,command[i]);
			}
		}
	}
	else if(var == 7)                                  // ��������� ������� 7
	{
		time_set = false;                              // ����� �������� ��������� ��������� ���
	}
	else if(var == 8)                                  // ��������� ������� 8
	{
		//  ����� � ����� ����� �������� �� 90 ������  
	}
	else
	{
		// ����� ����� ��� �� ��������� ���� ������� �� ������
	}
	
  // �������� ����������
  con.print(F("Total:"));                  //con.print("Total:");
  con.print(count);
  if (errors)                               // ���� ���� ������ - ��������
  {
	con.print(F(" Errors:")); //con.print(" Errors:");
	con.print(errors);
  }
  con.println();
}

void errorAllmem()
{
  int error_All;
  EEPROM.get(Address_errorAll, error_All);
  error_All++;
  EEPROM.put(Address_errorAll, error_All);            
}

int freeRam ()
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setTime(String val, String f_phone)
{
//   strcpy_P(bufmessage, (char*)pgm_read_word(&(table_message2[9])));

  if (val.indexOf(F("Timeset")) > -1)         // (val.indexOf("Timeset") > -1) 
  {
	 interval = 20;                                     // ���������� �������� 20 ������
	 time_set = true;                                   // ���������� �������� ��������� ��������� ���
	 Serial.println(interval);
  } 
  else if (val.indexOf(F("Restart")) > -1) 
  {
	  Serial.print(f_phone);
	  Serial.print("..");
	  Serial.println(F("Restart"));
	  delay(1000);
	  resetFunc();                                        //�������� reset
  } 
  else if (val.indexOf(F("Timeoff")) > -1) 
  {
	 time_set = false;                              // ����� �������� ��������� ��������� ���
  } 
  else
  {
	   Serial.println(F("Unknown command"));         // Serial.println("Unknown command");
  }
}


void blink()
{
	unsigned long current_M = millis();

	metering_NETLIGHT = current_M - metering_temp;
	metering_temp = current_M;
	//Serial.println(metering_NETLIGHT);
	if (metering_NETLIGHT > 3055 && metering_NETLIGHT < 3070)
	{
		state_device = 2;                 // 2 - ���������������� � ����
	}
	else if (metering_NETLIGHT > 855 && metering_NETLIGHT < 870)
	{
		state_device = 1;                // 1 �� ��������������� � ����, �����
	}
	else if (metering_NETLIGHT > 350 && metering_NETLIGHT < 370)
	{
		state_device = 3;                // 3 - GPRS ����� �����������
	}
	if (state_device == 1)
	{
		state = !state;
		if (!state)
		{
			setColor(COLOR_RED);
		}
		else
		{
			setColor(COLOR_GREEN);
		}
	}

	if (state_device == 2)
	{
		state = !state;
		if (!state)
		{
			setColor(COLOR_RED);
		}
		else
		{
			setColor(COLOR_BLUE);
		}
	}
}

void start_init()
{
	bool setup_ok = false;
	uint8_t count_init = 0;
	do
	{
		con.println(F("Initializing....(May take 5-10 seconds)"));

		digitalWrite(SIM800_RESET_PIN, LOW);                      // ������ ����� � �������� ���������
		digitalWrite(LED13, LOW);
		digitalWrite(PWR_On, HIGH);                               // �������������� ��������� ������� ������ GPRS
		delay(2000);
		digitalWrite(LED13, HIGH);
		digitalWrite(PWR_On, LOW);
		delay(1500);
		digitalWrite(SIM800_RESET_PIN, HIGH);                     // ���������� ����� ������ ����� ��������� �������
		delay(1000);
		digitalWrite(SIM800_RESET_PIN, LOW);

		while (digitalRead(STATUS) == LOW)
		{
			// �������� ��������� �����������  ���� ������ �� ���������
		}
		delay(2000);
		con.println(F("Power SIM800 On"));


		GPRSSerial->begin(19200);                               // �������� ������ � ������� SIM800C

		while (!gprs.begin(*GPRSSerial))
		{
			Serial.println(F("Couldn't find module GPRS"));
			while (1);
		}
		con.println(F("OK"));                  // 


		if (gprs.getIMEI())                       // �������� IMEI
		{
			con.print(F("\nIMEI:"));
			//imei = gprs.buffer;                 // ��������� �� ����� �������
			//gprs.cleanStr(imei);                // ��������� �� ����� �������
			con.println(imei);
		}
		else
		{
			// IMEI �� �����������

		}

		while (state_device != 2)  // �������� ����������� � ����
		{
			delay(1000);
			// �������� ��������� �����������  ���� ������ �� ����������������� ����� 60 ������
		}
		delay(1000);
		Serial.print(F("Setting up network..."));

		char n = gprs.getNetworkStatus();
		
		Serial.print(F("\nNetwork status "));
		Serial.print(n);
		Serial.print(F(": "));
		if (n == '0') Serial.println(F("\nNot registered"));                      // 0 � �� ���������������, ������ ���� ���
		if (n == '1') Serial.println(F("\nRegistered (home)"));                   // 1 � ���������������, �������� ����
		if (n == '2') Serial.println(F("\nNot registered (searching)"));          // 2 � �� ���������������, ��� ����� ����� ����
		if (n == '3') Serial.println(F("\nDenied"));                              // 3 � ����������� ���������
		if (n == '4') Serial.println(F("\nUnknown"));                             // 4 � ����������
		if (n == '5') Serial.println(F("\nRegistered roaming"));                  // 5 � �������

		if (n == '1' || n == '5')                                                 // ���� �������� ���� ��� �������
		{
			if (state_device == 2)                 // ��������� ��������� ����������� ������ � ���������
			{
				delay(2000);
				do
				{
					int signal = gprs.getSignalQuality();
					Serial.print(F("rssi ..")); Serial.println(signal);
					delay(1000);
					Serial.println(F("GPRS connect .."));
					byte ret = gprs.setup();                                              // ����������� � GPRS
					//Serial.print("ret - "); Serial.print(ret);
					if (ret == 0)
					{
						while (state_device != 3)  // �������� ����������� � ����
						{
							delay(50);
							// �������� ��������� �����������  ���� ������ �� ����������������� �� ����������������� ����� 60 ������
						}
						Serial.println(F("\nGPRS connect OK!+"));
						setColor(COLOR_GREEN);                 // �������� ������� ���������
						setup_ok = true;
					}
					else
					{
						count_init++;
						Serial.println(F("Failed init GPRS"));
						delay(5000);
						if (state_device == 3)
						{
							Serial.println(F("GPRS connect OK!-"));
							setColor(COLOR_GREEN);                 // �������� ������� ���������
							setup_ok = true;
						}
					}
				} while (!setup_ok);
			}
		}
	} while (count_init > 20 || setup_ok == false);    // 20 ������� ������������������ � ����
}

void setup()
{
	con.begin(speed_Serial);
	con.println(F("\n SIM800 setup start"));     

	pinMode(SIM800_RESET_PIN, OUTPUT);
	pinMode(LED13, OUTPUT);
	pinMode(PWR_On, OUTPUT);

	pinMode(LED_RED,  OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
	pinMode(LED_GREEN,OUTPUT);
	pinMode(NETLIGHT ,INPUT);                      // ��������� NETLIGHT
	pinMode(STATUS ,INPUT);                        // ��������� STATUS

	setColor(COLOR_RED);
	delay(300);
	setColor(COLOR_GREEN); 
	delay(300);
	setColor(COLOR_BLUE);
	delay(300);
	setColor(COLOR_RED);
	
	DeviceAddress deviceAddress;
	sensor1.setOneWire(&ds18x20_1);
	sensor2.setOneWire(&ds18x20_2);
	sensor3.setOneWire(&ds18x20_3);
	sensor1.begin();
	sensor2.begin();
	sensor3.begin();
	if (sensor1.getAddress(deviceAddress, 0)) sensor1.setResolution(deviceAddress, 12);
	if (sensor2.getAddress(deviceAddress, 0)) sensor2.setResolution(deviceAddress, 12);
	if (sensor3.getAddress(deviceAddress, 0)) sensor2.setResolution(deviceAddress, 12);

	attachInterrupt(1, blink, RISING);            // �������� ����������. ��������� ��������� ������

	start_init();
	

	con.println(F("OK"));           // con.println("OK");
	for (;;) 
	{
		if (gprs.httpInit()) break;                        // ��� ���������, ������ ������� , �������� ������� � ����� �� �����
		con.print(">");
		con.println(gprs.buffer);                          // �� ����������, ("ERROR") 
		String stringError = gprs.buffer;             
		if (stringError.indexOf(F("ERROR")) > -1)          
			{
				con.print(F("\nNo internet connection"));     
				delay(1000);
				resetFunc();                                //�������� reset ��� ���������� ������� � �������
			}
		gprs.httpUninit();                                  // �� ����������, ����������� ����� 
		delay(1000);
	}

	if(EEPROM.read(0)!=32)
	{
		con.println (F("Start clear EEPROM"));               //  
		for(int i = 0; i<1023;i++)
		{
			EEPROM.write(i,0);
		}
		EEPROM.write(0,32);
		EEPROM.put(Address_interval, interval);                     // ������ ��������� ��������� ����������
		EEPROM.put(Address_tel1, "+79858258846");
		// EEPROM.put(Address_tel1, "+79990000000");
		EEPROM.put(Address_tel2, "+79162632701");
		EEPROM.put(Address_tel3, "+79990000000");
		EEPROM.put(Address_SMS_center, "+79990000000");
		con.println (F("Clear EEPROM End"));                              
	}

	SMS_center = "SMS.RU";                                   //  SMS_center = "SMS.RU";
	//EEPROM.put(Address_interval, interval);                    // ��������������� ������ ����� ��������� ����������
	EEPROM.put(Address_SMS_center, SMS_center);                  // ��������������� ������ ����� ��������� ��� ������
	EEPROM.get(Address_interval, interval);                      //�������� �� EEPROM ��������
	EEPROM.get(Address_SMS_center, SMS_center);                  //�������� �� EEPROM ��� �����

	con.print(F("Interval sec:"));
	con.println(interval);
	con.println(SMS_center);
//	setColor(COLOR_GREEN);
	con.println(F("\nSIM800 setup end"));                        
	time = millis();                                              // ����� ������� �����
	
//	gprs.sendCommand("AT+CIPPING=\"www.yandex.ru\"", 1000);
	//delay(3000);
}

void loop()
{
	
	if (gprs.readSMS(message, data_tel, datetime))
	{
		// ������� �����, � �������� ������ ���
		Serial.print(F("From number: "));
		Serial.println(data_tel);

		// ������� ����, ����� ������ ���
		Serial.print(F("Datetime: "));
		Serial.println(datetime);

		// ������� ����� ���������
		Serial.print(F("Recieved Message: "));
		Serial.println(message);
	}



/*

 if (gprs.checkSMS()) 
  {
	con.print(F("SMS:"));                    //  con.print("SMS:");
	con.println(gprs.val);
	con.println(gprs.buffer);

	if (gprs.val.indexOf(F("+CMT")) > -1)  //���� ��������� ��� (��� ����������� ������ ������ "+CMT" ������� "RING", ������ �� �� �����, �� ����������� �� ���� ������ �����)
	{    
	//------------- ����� �������� ����� � ��� 
	char buf[13] ;

	EEPROM.get(Address_tel2, buf);                                         // ������������ ������� ������� 1
	String master_tel2(buf);
	EEPROM.get(Address_tel3, buf);                                         // ������������ ������� ������� 2
	String master_tel3(buf);
	EEPROM.get(Address_SMS_center, buf);                                   // ������������ ������� ��� ������
	String master_SMS_center(buf);

	  if (gprs.val.indexOf(master_tel2) > -1)                              //���� ��� �� ������� 1
	  {   
		con.println(F("Commanda tel1"));
		setTime(gprs.val, master_tel2);
	  }
	  else if(gprs.val.indexOf(master_tel3) > -1)                          //���� ��� �� ������� 2
	  {
		con.println(F("Commanda tel2"));
		setTime(gprs.val, master_tel3);
	  }
	  else if(gprs.val.indexOf(master_SMS_center) > -1)                    //���� ��� �� ������� 2
	  {
		con.println(F("SMS centr"));
		setTime(gprs.val, master_SMS_center);
	  }
	  else
	  {
		  con.println(F("phone ignored"));            
	  }
	 }
		
		gprs.val = "";
  }
 */
 
	unsigned long currentMillis = millis();
	if(!time_set)                                                               // 
	{
		 EEPROM.get( Address_interval, interval);                               // �������� �������� �� EEPROM Address_interval
	}
	if ((unsigned long)(currentMillis - previousMillis) >= interval*1000) 
	{
		con.print(F("Interval sec:"));                                       
		con.println((currentMillis-previousMillis)/1000);
		setColor(COLOR_BLUE);
		previousMillis = currentMillis;
		//gprs.sendCommand("AT+CIPPING=\"www.yandex.ru\"", 1000);
		sendTemps();
		setColor(COLOR_GREEN);
		con.print(F("\nfree memory: "));                                 
		con.println(freeRam());
	}

	if(millis() - time > time_day*1000) resetFunc();                       //�������� reset ���������� � �����
	delay(500);
}
