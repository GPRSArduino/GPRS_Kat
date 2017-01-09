/***************************************************
  This is an example for our SIM800C Module

  These cellular modules use TTL Serial to communicate, 2 pins are
  required to interface
 
  BSD license, all text above must be included in any redistribution
 ****************************************************/

/*
Open up the serial console on the Arduino at 115200 baud to interact with FONA

Note that if you need to set a GPRS APN, username, and password scroll down to
the commented section below at the end of the setup() function.
Обратите внимание, что если вам нужно установить GPRS APN, имя пользователя и пароль прокрутки вниз
комментируемого раздел ниже в конце функции настройки ().


 Optionally configure a GPRS APN, username, and password.
 You might need to do this to access your network's GPRS/data
 network.  Contact your provider for the exact APN, username,
 and password values.  Username and password are optional and
 can be removed, but APN is required.
fona.setGPRSNetworkSettings(F("your APN"), F("your username"), F("your password"));

 Optionally configure HTTP gets to follow redirects over SSL.
 Default is not to follow SSL redirects, however if you uncomment
 the following line then redirects over SSL will be followed.
fona.setHTTPSRedirect(true);

 При необходимости настроить GPRS APN, имя пользователя и пароль.
 Возможно, придется сделать это, чтобы получить доступ к GPRS вашей сети / данные
 Сеть. Обратитесь к поставщику для точного APN, имя пользователя,
 И значения пароля. Имя пользователя и пароль не являются обязательными и
 Может быть удален, но имя точки доступа требуется.
fona.setGPRSNetworkSettings(F("your APN "), F (" Ваше имя пользователя "), F (" Ваш пароль "));

 При необходимости настройки HTTP получает следовать переадресовывает над SSL.
 По умолчанию не следовать SSL переадресовывает, однако, если вы раскомментировать
 Следующая строка затем перенаправляет через SSL будет сопровождаться.
 fona.setHTTPSRedirect(true);
*/
#include "SIM800C_FONA.h"
#include <SoftwareSerial.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <MemoryFree.h>

static const char* url1 = "http://vps3908.vps.host.ru/recieveReadings.php";


#define FONA_RX 7
#define FONA_TX 8
#define FONA_RST 6
#define PWR_On           5                          // Включение питания модуля SIM800
#define LED13           13                          // Индикация светодиодом

#define NETLIGHT         3                          // Индикация NETLIGHT
#define STATUS           9                          // Индикация STATUS

#define port1           11                          // Порт управления внешними устройствами (незадействован)
#define port2           12                          // Порт управления внешними устройствами (незадействован)

// Подключить  к выводу 8 сигнал TX модуля GPRS. Установить в библиотеке SIM800.h  
// Подключить  к выводу 7 сигнал RX модуля GPRS. Установить в библиотеке SIM800.h
//#define COMMON_ANODE
#define LED_RED      10                             // Индикация светодиодом RED
#define LED_BLUE     15                             // Индикация светодиодом BLUE
#define LED_GREEN    14                             // Индикация светодиодом GREEN

#define COLOR_NONE LOW, LOW, LOW
#define COLOR_RED HIGH, LOW, LOW
#define COLOR_GREEN LOW, HIGH, LOW
#define COLOR_BLUE LOW, LOW, HIGH


char replybuffer[255];   // this is a large buffer for replies
char imei[15] = { 0 }; // MUST use a 15 character buffer for IMEI!
char ccid[20] = { 0 }; // MUST use a 20 character buffer for CCID!
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
SIM800C_FONA fona = SIM800C_FONA(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;

uint32_t count = 0;
uint32_t errors = 0;
String string_imei = "";
String CSQ = "";                                    // Уровень сигнала приема
String SMS_center = "";
String zero_tel = "";
//String imei = "861445030362268";                  // Тест IMEI
//#define DELIM "&"
#define DELIM "@"
//char mydata[] = "t1=861445030362268@04/01/02,15:22:52 00@24.50@25.60";
// тел Мегафон +79258110171

unsigned long time;                                 // Переменная для суточного сброса
unsigned long time_day = 86400;                     // Переменная секунд в сутках
unsigned long previousMillis = 0;
unsigned long interval = 10;                        // Интервал передачи данных 10 секунд
													//unsigned long interval = 300;                     // Интервал передачи данных 5 минут
bool time_set = false;                              // Фиксировать интервал заданный СМС


int Address_tel1 = 100;                         // Адрес в EEPROM телефона 1
int Address_tel2 = 120;                         // Адрес в EEPROM телефона 2
int Address_tel3 = 140;                         // Адрес в EEPROM телефона 3
int Address_errorAll = 160;                         // Адрес в EEPROM счетчика общих ошибок
int Address_port1 = 180;                         // Адрес в EEPROM порт данных (незадействован)
int Address_port2 = 190;                         // Адрес в EEPROM порт данных (незадействован)
int Address_interval = 200;                         // Адрес в EEPROM величины интервала
int Address_SMS_center = 220;                         // Адрес в EEPROM SMS центра

char data_tel[13];                                  // Буфер для номера телефоа

int dataport1 = 0;                                  // порт данных (незадействован)
int dataport2 = 0;                                  // порт данных (незадействован)


uint8_t oneWirePins[] = { 16, 17, 4 };                     //номера датчиков температуры DS18x20. Переставляя номера можно устанавливать очередность передачи в строке.
														   // Сейчас первым идет внутренний датчик.
uint8_t oneWirePinsCount = sizeof(oneWirePins) / sizeof(int);

OneWire ds18x20_1(oneWirePins[0]);
OneWire ds18x20_2(oneWirePins[1]);
OneWire ds18x20_3(oneWirePins[2]);
DallasTemperature sensor1(&ds18x20_1);
DallasTemperature sensor2(&ds18x20_2);
DallasTemperature sensor3(&ds18x20_3);

void(*resetFunc) (void) = 0;                         // объявляем функцию reset

void setColor(bool red, bool green, bool blue)       // Включение цвета свечения трехцветного светодиода.
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
	Serial.println(F("\nTemp"));
	sensor1.requestTemperatures();
	sensor2.requestTemperatures();
	sensor3.requestTemperatures();
	float t1 = sensor1.getTempCByIndex(0);
	float t2 = sensor2.getTempCByIndex(0);
	float t3 = sensor3.getTempCByIndex(0);
	float tsumma = t1 + t2 + t3 + 88.88;
	int signal = get_rssi();
	int error_All = 0;
	EEPROM.get(Address_errorAll, error_All);
	//String toSend = formHeader()+DELIM+"temp1="+String(t1)+DELIM+"temp2="+String(t2)+DELIM+"tempint="+String(t3)+ DELIM+"slevel="+String(signal)+DELIM+"ecs="+String(errors)+DELIM+"ec="+String(error_All)+formEnd();
	String toSend = formHeader() + DELIM + String(t1) + DELIM + String(t2) + DELIM + String(t3) + DELIM + String(signal) + DELIM + String(errors) + DELIM + String(error_All) + formEnd() + DELIM + String(tsumma);
	Serial.println(toSend);
	Serial.println(F("String length: "));
	Serial.println(toSend.length());
	//gprs_send(toSend);
}

String formHeader()
{
	String uptime = "17/01/01,10:10:10 00";
	string_imei = String(imei);
	char buffer[23];
	
	fona.getTime(buffer, 23);  // make sure replybuffer is at least 23 bytes!
	Serial.print(F("Time = ")); Serial.println(buffer);


	//GSM_LOCATION loc;                               // Получить время из интернета
	//if (gprs.getLocation(&loc))
	//{
	//	uptime = String(loc.year) + '/' + String(loc.month) + '/' + String(loc.day) + ',' + String(loc.hour) + ':' + String(loc.minute) + ':' + String(loc.second) + " 00";
	//	//  uptime  = "date="+ String(loc.year)+'_'+ String(loc.month)+'_'+ String(loc.day)+','+String(loc.hour)+':'+ String(loc.minute)+':'+String(loc.second)+"00";
	//}
	// return "imei=" + imei + DELIM + uptime;
	return "t1=" + string_imei + DELIM + uptime;
}
String formEnd()
{
	char buf[13];

	EEPROM.get(Address_tel1, buf);
	String master_tel1(buf);
	Serial.println(master_tel1);

	EEPROM.get(Address_tel2, buf);
	String master_tel2(buf);
	Serial.println(master_tel2);

	EEPROM.get(Address_tel3, buf);
	String master_tel3(buf);
	Serial.println(master_tel3);

	//EEPROM.get(Address_tel1, master_tel1); 
	//EEPROM.get(Address_tel2, master_tel3); 
	//EEPROM.get(Address_tel2, master_tel3); 


	EEPROM.get(Address_SMS_center, SMS_center);   //Получить из EEPROM СМС центр


	if (EEPROM.read(Address_port1))
	{

	}
	else
	{
		//dataport1 = digitalRead(port1);
		//Serial.println(dataport1);
	}

	if (EEPROM.read(Address_port2))
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


void printMenu(void) 
{
  //Serial.println(F("-------------------------------------"));
  //Serial.println(F("[?] Print this menu"));
  //Serial.println(F("[a] read the ADC 2.8V max (FONA800 & 808)"));
  //Serial.println(F("[b] read the Battery V and % charged"));
  //Serial.println(F("[C] read the SIM CCID"));
  //Serial.println(F("[U] Unlock SIM with PIN code"));
  //Serial.println(F("[i] read RSSI"));
  //Serial.println(F("[n] get Network status"));
  //Serial.println(F("[v] set audio Volume"));
  //Serial.println(F("[V] get Volume"));
  //Serial.println(F("[H] set Headphone audio (FONA800 & 808)"));
  //Serial.println(F("[e] set External audio (FONA800 & 808)"));
  //Serial.println(F("[T] play audio Tone"));
  //Serial.println(F("[P] PWM/Buzzer out (FONA800 & 808)"));

  //// FM (SIM800 only!)
  //Serial.println(F("[f] tune FM radio (FONA800)"));
  //Serial.println(F("[F] turn off FM (FONA800)"));
  //Serial.println(F("[m] set FM volume (FONA800)"));
  //Serial.println(F("[M] get FM volume (FONA800)"));
  //Serial.println(F("[q] get FM station signal level (FONA800)"));

  //// Phone
  //Serial.println(F("[c] make phone Call"));
  //Serial.println(F("[A] get call status"));
  //Serial.println(F("[h] Hang up phone"));
  //Serial.println(F("[p] Pick up phone"));

  //// SMS
  //Serial.println(F("[N] Number of SMSs"));
  //Serial.println(F("[r] Read SMS #"));
  //Serial.println(F("[R] Read All SMS"));
  //Serial.println(F("[d] Delete SMS #"));
  //Serial.println(F("[s] Send SMS"));
  //Serial.println(F("[u] Send USSD"));
  //
  //// Time
  //Serial.println(F("[y] Enable network time sync (FONA 800 & 808)"));
  //Serial.println(F("[Y] Enable NTP time sync (GPRS FONA 800 & 808)"));
  //Serial.println(F("[t] Get network time"));

  //// GPRS
  //Serial.println(F("[G] Enable GPRS"));
  //Serial.println(F("[g] Disable GPRS"));
  //Serial.println(F("[l] Query GSMLOC (GPRS)"));
  //Serial.println(F("[w] Read webpage (GPRS)"));
  //Serial.println(F("[W] Post to website (GPRS)"));
  // 
  //Serial.println(F("[S] create Serial passthru tunnel"));
  //Serial.println(F("-------------------------------------"));
  //Serial.println(F(""));

}

void flushSerial() 
{
  while (Serial.available())
    Serial.read();
}

char readBlocking() 
{
  while (!Serial.available());
  return Serial.read();
}
uint16_t readnumber() 
{
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) 
  {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout) 
{
  uint16_t buffidx = 0;
  boolean timeoutvalid = true;
  if (timeout == 0) timeoutvalid = false;

  while (true) 
  {
    if (buffidx > maxbuff) 
    {
      //Serial.println(F("SPACE"));
      break;
    }

    while (Serial.available()) 
    {
      char c =  Serial.read();

      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0)   // the first 0x0A is ignored
          continue;

        timeout = 0;         // the second 0x0A is the end of the line
        timeoutvalid = true;
        break;
      }
      buff[buffidx] = c;
      buffidx++;
    }

    if (timeoutvalid && timeout == 0) {
      //Serial.println(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}

void init_SIM800C()
{
	Serial.println(F("Initializing....(May take 5 seconds)"));
	digitalWrite(FONA_RST, LOW);               // Сигнал сброс в исходное состояние
	digitalWrite(LED13, LOW);
	digitalWrite(PWR_On, HIGH);                // Кратковременно отключаем питание модуля GPRS
	delay(2000);
	digitalWrite(LED13, HIGH);
	digitalWrite(PWR_On, LOW);
	delay(1500);
	//digitalWrite(FONA_RST,   HIGH);            // Производим сброс модема после включения питания
	//delay(1200);
	//digitalWrite(FONA_RST,   LOW);               
	delay(3000);

	fonaSerial->begin(19200);

	if (!fona.begin(*fonaSerial))
	{
		Serial.println(F("Couldn't find SIM80C"));
		while (1);
	}
	Serial.println(F("SIM800C is OK"));

	uint8_t imeiLen = fona.getIMEI(imei);
	if (imeiLen > 0)
	{
		Serial.print("Module IMEI: "); Serial.println(imei); // Print module IMEI number.
	}
	uint8_t ccidLen = fona.getSIMCCID(ccid);                 // read the CCID make sure replybuffer is at least 21 bytes!
	if (ccidLen > 0)
	{
		Serial.print(F("SIM CCID = ")); Serial.println(ccid);
	}
	//delay(20000);

	uint8_t operatorLen = fona.getOperator();
	if (operatorLen > 0)
	{
		//char get_operator[] = "";
		//char *p = strstr(replybuffer, ",\"");
		//if (p)
		//{
		//	p += 2;
		//	char *s1 = strchr(p, '\"');
		//	//if (s) *s = 0;
		//	//strcpy(replybuffer, p);
		//	//return sendCheckReply(get_operator, ok_reply);
		//	//Serial.print(F("Operator: ")); Serial.println(replybuffer); // 
		//}

		Serial.print(F("Operator: ")); Serial.println(replybuffer); // 
	}
}

int get_rssi()
{
	// read the RSSI
	uint8_t n = fona.getRSSI();
	int8_t r;
	 
	Serial.print(F("RSSI = ")); Serial.print(n); Serial.print(": ");
	if (n == 0) r = -115;
	if (n == 1) r = -111;
	if (n == 31) r = -52;
	if ((n >= 2) && (n <= 30)) 
	{
		r = map(n, 2, 30, -110, -54);
	}
	Serial.print(r); Serial.println(F(" dBm"));
	return n;
}




  
void setup() {
  while (!Serial);

  Serial.begin(115200);
  Serial.println(F("SIM800C basic test"));
  pinMode(FONA_RST, OUTPUT);
  pinMode(LED13,    OUTPUT);
  pinMode(PWR_On,   OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(NETLIGHT, INPUT);                      // Индикация NETLIGHT
  pinMode(STATUS, INPUT);                        // Индикация STATUS

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

  init_SIM800C();

 

 // Serial.println(F("OK"));           // con.println("OK");
 // for (;;)
 // {  
	//  if (fona.HTTP_init()) break;                        // Все нормально, модуль ответил , Прервать попытки и выйти из цикла
	//  Serial.print(">");
	//  while (fona.available())
	//  {
	//	  Serial.write(fona.read());
	//  }
	////  Serial.println(gprs.buffer);                          // Не получилось, ("ERROR") 
	//  //String stringError = fona.read();
	//  //if (stringError.indexOf(F("ERROR")) > -1)
	//  //{
	//	 // Serial.print(F("\nNo internet connection"));
	//	 // delay(1000);
	//	 // resetFunc();                                //вызываем reset при отсутствии доступа к серверу
	//  //}
	//  fona.HTTP_init();                                  // Не получилось, попробовать снова 
	//  delay(1000);
 // }

  if (EEPROM.read(0) != 31)
  {
	  Serial.println(F("Start clear EEPROM"));               //  
	  for (int i = 0; i<1023; i++)
	  {
		  EEPROM.write(i, 0);
	  }
	  EEPROM.write(0, 31);
	  EEPROM.put(Address_interval, interval);                     // строка начальной установки интервалов
	  EEPROM.put(Address_tel1, "+79990000000");
	  EEPROM.put(Address_tel2, "+79990000000");
	  EEPROM.put(Address_tel3, "+79990000000");
	  EEPROM.put(Address_SMS_center, "+79990000000");
	  Serial.println(F("Clear EEPROM End"));
  }

  SMS_center = "SMS.RU";                                   //  SMS_center = "SMS.RU";
														   // EEPROM.put(Address_interval, interval);                    // Закоментировать строку после установки интервалов
  EEPROM.put(Address_SMS_center, SMS_center);                  // Закоментировать строку после установки СМС центра
  EEPROM.get(Address_interval, interval);                      //Получить из EEPROM интервал
  EEPROM.get(Address_SMS_center, SMS_center);                  //Получить из EEPROM СМС центр

  Serial.print(F("Interval sec:"));
  Serial.println(interval);
  Serial.println(SMS_center);

  setColor(COLOR_BLUE);
 // sendTemps();
  setColor(COLOR_GREEN);
  Serial.println(F("\nSIM800 setup end"));
  time = millis();                                              // Старт отсчета суток

}
void loop() {
	Serial.print(F("SM800C > "));
	//while (!Serial.available()) 
	//{
	//	if (fona.available()) 
	//	{
	//		Serial.write(fona.read());
	//	}
	//}
	//flushSerial();
 //	 while (fona.available())
	// {
	////	Serial.write(fona.read());
	//	//sendTemps();
	// }
	/* sendTemps();
	 delay(10000);*/

	 unsigned long currentMillis = millis();
	 if (!time_set)                                                               // 
	 {
		 EEPROM.get(Address_interval, interval);                               // Получить интервал из EEPROM Address_interval
	 }
	 if ((unsigned long)(currentMillis - previousMillis) >= interval * 1000)
	 {
		 Serial.print(F("Interval sec:"));
		 Serial.println((currentMillis - previousMillis) / 1000);
		 setColor(COLOR_BLUE);
		 previousMillis = currentMillis;
		 sendTemps();
		 setColor(COLOR_GREEN);
		 Serial.print(F("\nfree memory: "));
		 Serial.println(freeMemory());
	 }
	 flushSerial();
	 if (millis() - time > time_day * 1000) resetFunc();                       //вызываем reset интервалом в сутки
	 delay(1000);

}

  //char command = Serial.read();
  //Serial.println(command);

//
//  switch (command) 
//  {
//    case '?': {
//        printMenu();
//        break;
//      }
//
//    case 'a': {
//        // read the ADC
//        uint16_t adc;
//        if (! fona.getADCVoltage(&adc)) {
//          Serial.println(F("Failed to read ADC"));
//        } else {
//          Serial.print(F("ADC = ")); Serial.print(adc); Serial.println(F(" mV"));
//        }
//        break;
//      }
//
//    case 'b': {
//        // read the battery voltage and percentage
//        uint16_t vbat;
//        if (! fona.getBattVoltage(&vbat)) {
//          Serial.println(F("Failed to read Batt"));
//        } else {
//          Serial.print(F("VBat = ")); Serial.print(vbat); Serial.println(F(" mV"));
//        }
//
//
//        if (! fona.getBattPercent(&vbat)) {
//          Serial.println(F("Failed to read Batt"));
//        } else {
//          Serial.print(F("VPct = ")); Serial.print(vbat); Serial.println(F("%"));
//        }
//
//        break;
//      }
//
//    case 'U': {
//        // Unlock the SIM with a PIN code
//        char PIN[5];
//        flushSerial();
//        Serial.println(F("Enter 4-digit PIN"));
//        readline(PIN, 3);
//        Serial.println(PIN);
//        Serial.print(F("Unlocking SIM card: "));
//        if (! fona.unlockSIM(PIN)) {
//          Serial.println(F("Failed"));
//        } else {
//          Serial.println(F("OK!"));
//        }
//        break;
//      }
//
//    case 'C': {
//        // read the CCID
//        fona.getSIMCCID(replybuffer);  // make sure replybuffer is at least 21 bytes!
//        Serial.print(F("SIM CCID = ")); Serial.println(replybuffer);
//        break;
//      }
//
//    case 'i': {
//        // read the RSSI
//        uint8_t n = fona.getRSSI();
//        int8_t r;
//
//        Serial.print(F("RSSI = ")); Serial.print(n); Serial.print(": ");
//        if (n == 0) r = -115;
//        if (n == 1) r = -111;
//        if (n == 31) r = -52;
//        if ((n >= 2) && (n <= 30)) {
//          r = map(n, 2, 30, -110, -54);
//        }
//        Serial.print(r); Serial.println(F(" dBm"));
//
//        break;
//      }
//
//    case 'n': {
//        // read the network/cellular status
//        uint8_t n = fona.getNetworkStatus();
//        Serial.print(F("Network status "));
//        Serial.print(n);
//        Serial.print(F(": "));
//        if (n == 0) Serial.println(F("Not registered"));
//        if (n == 1) Serial.println(F("Registered (home)"));
//        if (n == 2) Serial.println(F("Not registered (searching)"));
//        if (n == 3) Serial.println(F("Denied"));
//        if (n == 4) Serial.println(F("Unknown"));
//        if (n == 5) Serial.println(F("Registered roaming"));
//        break;
//      }
//
//    /*** Audio ***/
//    case 'v': {
//        // set volume
//        flushSerial();
//      
//          Serial.print(F("Set Vol % [0-100] "));
//     
//        uint8_t vol = readnumber();
//        Serial.println();
//        if (! fona.setVolume(vol)) {
//          Serial.println(F("Failed"));
//        } else {
//          Serial.println(F("OK!"));
//        }
//        break;
//      }
//
//    case 'V': {
//        uint8_t v = fona.getVolume();
//        Serial.print(v);
//    
//          Serial.println("%");
//      
//        break;
//      }
//
//    case 'H': {
//        // Set Headphone output
//        if (! fona.setAudio(FONA_HEADSETAUDIO)) {
//          Serial.println(F("Failed"));
//        } else {
//          Serial.println(F("OK!"));
//        }
//        fona.setMicVolume(FONA_HEADSETAUDIO, 15);
//        break;
//      }
//    case 'e': {
//        // Set External output
//        if (! fona.setAudio(FONA_EXTAUDIO)) {
//          Serial.println(F("Failed"));
//        } else {
//          Serial.println(F("OK!"));
//        }
//
//        fona.setMicVolume(FONA_EXTAUDIO, 10);
//        break;
//      }
//
//    case 'T': {
//        // play tone
//        flushSerial();
//        Serial.print(F("Play tone #"));
//        uint8_t kittone = readnumber();
//        Serial.println();
//        // play for 1 second (1000 ms)
//        if (! fona.playToolkitTone(kittone, 1000)) {
//          Serial.println(F("Failed"));
//        } else {
//          Serial.println(F("OK!"));
//        }
//        break;
//      }
//
//  
//    /*** PWM ***/
//
//    case 'P': {
//        // PWM Buzzer output @ 2KHz max
//        flushSerial();
//        Serial.print(F("PWM Freq, 0 = Off, (1-2000): "));
//        uint16_t freq = readnumber();
//        Serial.println();
//        if (! fona.setPWM(freq)) {
//          Serial.println(F("Failed"));
//        } else {
//          Serial.println(F("OK!"));
//        }
//        break;
//      }
//
//    /*** Call ***/
//    case 'c': {
//        // call a phone!
//        char number[30];
//        flushSerial();
//        Serial.print(F("Call #"));
//        readline(number, 30);
//        Serial.println();
//        Serial.print(F("Calling ")); Serial.println(number);
//        if (!fona.callPhone(number)) {
//          Serial.println(F("Failed"));
//        } else {
//          Serial.println(F("Sent!"));
//        }
//
//        break;
//      }
//    case 'A': {
//        // get call status
//        int8_t callstat = fona.getCallStatus();
//        switch (callstat) {
//          case 0: Serial.println(F("Ready")); break;
//          case 1: Serial.println(F("Could not get status")); break;
//          case 3: Serial.println(F("Ringing (incoming)")); break;
//          case 4: Serial.println(F("Ringing/in progress (outgoing)")); break;
//          default: Serial.println(F("Unknown")); break;
//        }
//        break;
//      }
//      
//    case 'h': {
//        // hang up!
//        if (! fona.hangUp()) {
//          Serial.println(F("Failed"));
//        } else {
//          Serial.println(F("OK!"));
//        }
//        break;
//      }
//
//    case 'p': {
//        // pick up!
//        if (! fona.pickUp()) {
//          Serial.println(F("Failed"));
//        } else {
//          Serial.println(F("OK!"));
//        }
//        break;
//      }
//
//    /*** SMS ***/
//
//    case 'N': {
//        // read the number of SMS's!
//        int8_t smsnum = fona.getNumSMS();
//        if (smsnum < 0) {
//          Serial.println(F("Could not read # SMS"));
//        } else {
//          Serial.print(smsnum);
//          Serial.println(F(" SMS's on SIM card!"));
//        }
//        break;
//      }
//    case 'r': {
//        // read an SMS
//        flushSerial();
//        Serial.print(F("Read #"));
//        uint8_t smsn = readnumber();
//        Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);
//
//        // Retrieve SMS sender address/phone number.
//        if (! fona.getSMSSender(smsn, replybuffer, 250)) {
//          Serial.println("Failed!");
//          break;
//        }
//        Serial.print(F("FROM: ")); Serial.println(replybuffer);
//
//        // Retrieve SMS value.
//        uint16_t smslen;
//        if (! fona.readSMS(smsn, replybuffer, 250, &smslen)) { // pass in buffer and max len!
//          Serial.println("Failed!");
//          break;
//        }
//        Serial.print(F("***** SMS #")); Serial.print(smsn);
//        Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
//        Serial.println(replybuffer);
//        Serial.println(F("*****"));
//
//        break;
//      }
//    case 'R': {
//        // read all SMS
//        int8_t smsnum = fona.getNumSMS();
//        uint16_t smslen;
//        int8_t smsn;
//
//   
//          smsn = 1;  // 1 indexed
//   
//
//        for ( ; smsn <= smsnum; smsn++) {
//          Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);
//          if (!fona.readSMS(smsn, replybuffer, 250, &smslen)) {  // pass in buffer and max len!
//            Serial.println(F("Failed!"));
//            break;
//          }
//          // if the length is zero, its a special case where the index number is higher
//          // so increase the max we'll look at!
//          if (smslen == 0) {
//            Serial.println(F("[empty slot]"));
//            smsnum++;
//            continue;
//          }
//
//          Serial.print(F("***** SMS #")); Serial.print(smsn);
//          Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
//          Serial.println(replybuffer);
//          Serial.println(F("*****"));
//        }
//        break;
//      }
//
//    case 'd': {
//        // delete an SMS
//        flushSerial();
//        Serial.print(F("Delete #"));
//        uint8_t smsn = readnumber();
//
//        Serial.print(F("\n\rDeleting SMS #")); Serial.println(smsn);
//        if (fona.deleteSMS(smsn)) {
//          Serial.println(F("OK!"));
//        } else {
//          Serial.println(F("Couldn't delete"));
//        }
//        break;
//      }
//
//    case 's': {
//        // send an SMS!
//        char sendto[21], message[141];
//        flushSerial();
//        Serial.print(F("Send to #"));
//        readline(sendto, 20);
//        Serial.println(sendto);
//        Serial.print(F("Type out one-line message (140 char): "));
//        readline(message, 140);
//        Serial.println(message);
//        if (!fona.sendSMS(sendto, message)) {
//          Serial.println(F("Failed"));
//        } else {
//          Serial.println(F("Sent!"));
//        }
//
//        break;
//      }
//
//    case 'u': {
//      // send a USSD!
//      char message[141];
//      flushSerial();
//      Serial.print(F("Type out one-line message (140 char): "));
//      readline(message, 140);
//      Serial.println(message);
//
//      uint16_t ussdlen;
//      if (!fona.sendUSSD(message, replybuffer, 250, &ussdlen)) { // pass in buffer and max len!
//        Serial.println(F("Failed"));
//      } else {
//        Serial.println(F("Sent!"));
//        Serial.print(F("***** USSD Reply"));
//        Serial.print(" ("); Serial.print(ussdlen); Serial.println(F(") bytes *****"));
//        Serial.println(replybuffer);
//        Serial.println(F("*****"));
//      }
//    }
//
//    /*** Time ***/
//
//    case 'y': {
//        // enable network time sync
//        if (!fona.enableNetworkTimeSync(true))
//          Serial.println(F("Failed to enable"));
//        break;
//      }
//
//    case 'Y': {
//        // enable NTP time sync
//        if (!fona.enableNTPTimeSync(true, F("pool.ntp.org")))
//          Serial.println(F("Failed to enable"));
//        break;
//      }
//
//    case 't': {
//        // read the time
//        char buffer[23];
//
//        fona.getTime(buffer, 23);  // make sure replybuffer is at least 23 bytes!
//        Serial.print(F("Time = ")); Serial.println(buffer);
//        break;
//      }
//
//     /*********************************** GPRS */
//
//    case 'g': {
//        // turn GPRS off
//        if (!fona.enableGPRS(false))
//          Serial.println(F("Failed to turn off"));
//        break;
//      }
//    case 'G': {
//        // turn GPRS on
//        if (!fona.enableGPRS(true))
//          Serial.println(F("Failed to turn on"));
//        break;
//      }
//    case 'l': {
//        // check for GSMLOC (requires GPRS)
//        uint16_t returncode;
//
//        if (!fona.getGSMLoc(&returncode, replybuffer, 250))
//          Serial.println(F("Failed!"));
//        if (returncode == 0) {
//          Serial.println(replybuffer);
//        } else {
//          Serial.print(F("Fail code #")); Serial.println(returncode);
//        }
//
//        break;
//      }
//    case 'w': {
//        // read website URL
//        uint16_t statuscode;
//        int16_t length;
//        char url[80];
//
//        flushSerial();
//        Serial.println(F("NOTE: in beta! Use small webpages to read!"));
//        Serial.println(F("URL to read (e.g. www.adafruit.com/testwifi/index.html):"));
//        Serial.print(F("http://")); readline(url, 79);
//        Serial.println(url);
//
//        Serial.println(F("****"));
//        if (!fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length)) {
//          Serial.println("Failed!");
//          break;
//        }
//        while (length > 0) {
//          while (fona.available()) {
//            char c = fona.read();
//
//            // Serial.write is too slow, we'll write directly to Serial register!
//#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
//            loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
//            UDR0 = c;
//#else
//            Serial.write(c);
//#endif
//            length--;
//            if (! length) break;
//          }
//        }
//        Serial.println(F("\n****"));
//        fona.HTTP_GET_end();
//        break;
//      }
//
//    case 'W': {
//        // Post data to website
//        uint16_t statuscode;
//        int16_t length;
//        char url[80];
//        char data[80];
//
//        flushSerial();
//        Serial.println(F("NOTE: in beta! Use simple websites to post!"));
//        Serial.println(F("URL to post (e.g. httpbin.org/post):"));
//        Serial.print(F("http://")); readline(url, 79);
//        Serial.println(url);
//        Serial.println(F("Data to post (e.g. \"foo\" or \"{\"simple\":\"json\"}\"):"));
//        readline(data, 79);
//        Serial.println(data);
//
//        Serial.println(F("****"));
//        if (!fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length)) {
//          Serial.println("Failed!");
//          break;
//        }
//        while (length > 0) {
//          while (fona.available()) {
//            char c = fona.read();
//
//#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
//            loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
//            UDR0 = c;
//#else
//            Serial.write(c);
//#endif
//
//            length--;
//            if (! length) break;
//          }
//        }
//        Serial.println(F("\n****"));
//        fona.HTTP_POST_end();
//        break;
//      }
//    /*****************************************/
//
//    case 'S': {
//        Serial.println(F("Creating SERIAL TUBE"));
//        while (1) {
//          while (Serial.available()) {
//            delay(1);
//            fona.write(Serial.read());
//          }
//          if (fona.available()) {
//            Serial.write(fona.read());
//          }
//        }
//        break;
//      }
//
//    default: {
//        Serial.println(F("Unknown command"));
//        printMenu();
//        break;
//      }
//  }
  // flush input


