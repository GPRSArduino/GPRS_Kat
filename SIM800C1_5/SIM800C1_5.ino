/*
Программа передачи данных по каналу GPRS
24.01.2017г.



*/

#include "SIM800.h"
#include <SoftwareSerial.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <avr/wdt.h>

#define con Serial
#define speed_Serial 115200

static const char* url1   = "http://vps3908.vps.host.ru/recieveReadings.php";
//static const char* urlssl = "https://vps3908.vps.host.ru/recieveReadings.php";
static const char* url_ping = "www.yandex.ru";


#define PIN_TX           7                             // Подключить  к выводу 7 сигнал RX модуля GPRS
#define PIN_RX           8                              // Подключить  к выводу 8 сигнал TX модуля GPRS

SoftwareSerial SIM800CSS = SoftwareSerial(PIN_RX, PIN_TX);
SoftwareSerial *GPRSSerial = &SIM800CSS;



#define PWR_On           5                          // Включение питания модуля SIM800
#define SIM800_RESET_PIN 6                          // Сброс модуля SIM800
#define LED13           13                          // Индикация светодиодом
#define NETLIGHT         3                          // Индикация NETLIGHT
#define STATUS           9                          // Индикация STATUS

#define port1           11                          // Порт управления внешними устройствами (незадействован)
#define port2           12                          // Порт управления внешними устройствами (незадействован)

												
//#define COMMON_ANODE
#define LED_RED      10                             // Индикация светодиодом RED
#define LED_BLUE     15                             // Индикация светодиодом BLUE
#define LED_GREEN    14                             // Индикация светодиодом GREEN

#define COLOR_NONE LOW, LOW, LOW
#define COLOR_GREEN LOW, HIGH, LOW
#define COLOR_BLUE LOW, LOW, HIGH

volatile int state = LOW;
volatile int state_device = 0;                     // Состояние модуля при запуске 
#define COLOR_RED HIGH, LOW, LOW
												   // 1 - Не зарегистрирован в сети, поиск
												   // 2 - Зарегистрировано в сети
												   // 3 - GPRS связь установлена
volatile int metering_NETLIGHT       = 0;
volatile unsigned long metering_temp = 0;
volatile int count_blink1            = 0;          // Счетчик попыток подключиться к базовой станции
volatile int count_blink2            = 0;          // Счетчик попыток подключиться к базовой станции
bool send_ok                         = false;      // Признак успешной передачи данных

CGPRS_SIM800 gprs;
uint32_t count    = 0;
uint32_t errors   = 0;
//String imei       = "";
String imei = "861445030362268";                   // Тест IMEI
String CSQ        = "";                               // Уровень сигнала приема
String SMS_center = "";
String zero_tel   = "";
String SIMCCID    = "";
#define DELIM "@"

unsigned long time;                                   // Переменная для суточного сброса
unsigned long time_day         = 86400;                 // Переменная секунд в сутках
unsigned long previousMillis   = 0;
//unsigned long interval       = 60;                    // Интервал передачи данных 200 секунд
unsigned long interval         = 300;                   // Интервал передачи данных 5 минут
bool time_set                  = false;                 // Фиксировать интервал заданный СМС
bool ssl_set                   = false;                 // Признак шифрования
unsigned long time_ping        = 360;                   // Интервал проверки ping 6 минут.
unsigned long previousPing     = 0;                     // Временный Интервал проверки ping


int Address_tel1       = 100;                         // Адрес в EEPROM телефона 1
int Address_ssl        = 120;                         // Адрес в EEPROM признака шифрования
int Address_errorAll   = 160;                         // Адрес в EEPROM счетчика общих ошибок
int Address_interval   = 200;                         // Адрес в EEPROM величины интервала
int Address_SMS_center = 220;                         // Адрес в EEPROM SMS центра

char data_tel[16];                                    // Буфер для номера телефоа

uint8_t oneWirePins[]={16, 17, 4};                     //номера датчиков температуры DS18x20. Переставляя номера можно устанавливать очередность передачи в строке.
													   // Сейчас первым идет внутренний датчик.
uint8_t oneWirePinsCount=sizeof(oneWirePins)/sizeof(int);

OneWire ds18x20_1(oneWirePins[0]);
OneWire ds18x20_2(oneWirePins[1]);
OneWire ds18x20_3(oneWirePins[2]);
DallasTemperature sensor1(&ds18x20_1);
DallasTemperature sensor2(&ds18x20_2);
DallasTemperature sensor3(&ds18x20_3);


void(* resetFunc) (void) = 0;                         // объявляем функцию reset

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

	String toSend = formHeader() + DELIM + String(t1) + DELIM + String(t2) + DELIM + String(t3) + DELIM + String(signal) + DELIM + String(errors) + DELIM + String(error_All) + formEnd() + DELIM + String(tsumma);

	//Serial.println(toSend);

	Serial.println(toSend.length());

	int count_send = 0;
	while (1)
	{
		if (gprs_send(toSend))
		{
			return;
		}
		else
		{
			count_send++;
			Serial.print("Attempt to transfer data .."); Serial.println(count_send);
			if (count_send>5) resetFunc(); // 5 попыток. Что то пошло не так с интернетом
			ping();
		}
		delay(6000);
	}

}

String formHeader() 
{
  String uptime = "17/01/01,10:10:10 00";
  GSM_LOCATION loc;                               // Получить время из интернета
  if (gprs.getLocation(&loc)) 
  {
   uptime  = String(loc.year)+'/'+ String(loc.month)+'/'+ String(loc.day)+','+String(loc.hour)+':'+ String(loc.minute)+':'+String(loc.second)+" 00";
  }
   return "t1=" + imei + DELIM + uptime;
}
String formEnd() 
{
	char buf[13] ;

	EEPROM.get(Address_tel1, buf);
	String master_tel1(buf);
	//con.println(master_tel1);

	EEPROM.get(Address_SMS_center, SMS_center);   //Получить из EEPROM СМС центр

	return DELIM + master_tel1 + DELIM + SIMCCID;

}

bool gprs_send(String data) 
{
  con.print(F("Requesting "));               
 
  if (ssl_set == true)
  {
	/*  con.print(urlssl);
	  con.print('?');
	  con.println(data);

	  gprs.httpConnectStr(urlssl, data);*/
  }
  else
  {
	  con.print(url1);
	  con.print('?');
	  con.println(data);
	  gprs.httpConnectStr(url1, data);
  }
  count++;
  send_ok = false;
    
  while (gprs.httpIsConnected() == 0) 
  {
	con.write('.');
	for (byte n = 0; n < 25 && !gprs.available(); n++) 
	{
	  delay(15);
	}
  }
  if (gprs.httpState == HTTP_ERROR) 
  {
	con.println(F("Connect error"));
		errors++;
		errorAllmem();
	
	if (errors > 20)
	  {
			con.println(F("Number of transmission errors exceeded"));
			delay(3000);
			resetFunc();          // вызываем reset после 20 ошибок
	  }
	delay(3000);
	return; 
  }
  
  con.println();
  gprs.httpRead();
  int ret;
  while ((ret = gprs.httpIsRead()) == 0) 
  {
	// может сделать что-то здесь, во время ожидания
  }
  if (gprs.httpState == HTTP_ERROR) 
  {
	errors++;
	errorAllmem();
	if (errors > 20)
	  {
			con.println(F("The number of server errors exceeded 20"));
			delay(3000);
			resetFunc();         // вызываем reset после 20 ошибок
	  }
	delay(3000);
	return; 
  }

  // Теперь мы получили сообщение от сайта.
   con.print(F("[Payload] "));                            //con.print("[Payload] ");
   con.println(gprs.buffer);
   String command = gprs.buffer;                          // Получить строку данных с сервера
   String commEXE = command.substring(0, 2);              // Выделить строку с командой
   int var = commEXE.toInt();                             // Получить номер команды. Преобразовать строку команды в число 
   send_ok = true;                                        // Команда принята успешно
   if(var == 1)                                           // Выполнить команду 1
	{
		String commData = command.substring(2, 10);       // Выделить строку с данными
		unsigned long interval1 = commData.toInt();       // Преобразовать строку данных в число 
		con.println(interval1);
		if(interval1 > 10 && interval1 < 86401)           // Ограничить интервалы от 10  секунд до 24 часов.
		{
		  if(interval1!=interval)                         // Если информиция не изменилась - не писать в EEPROM
		  {
			 if(!time_set)                                // Если нет команды фиксации интервала от СМС 
			 {
				//interval = interval1;                     // Переключить интервал передачи на сервер
				//EEPROM.put(Address_interval, interval);   // Записать интервал EEPROM , полученный от сервера
			 }
		  }
		}
		con.println(interval);
	}
	
	else if(var == 2)                                  // Выполнить команду 2
	{
		command.remove(0, 2);                          // Получить данные номера телефона от сервера
		EEPROM.get(Address_tel1, data_tel);            // Получить номер телефона из EEPROM
		String num_tel(data_tel);
		if (command != num_tel)                        // Если информиция не изменилась - не писать в EEPROM
		{
			 con.println(F("no compare"));               //Serial.println("no compare");
			for(int i=0;i<13;i++)
			{
				EEPROM.write(i+Address_tel1,command[i]);
			}
		}
	}
	
	else if(var == 3)                                  // Выполнить команду 3
	{
	


	}
	else if(var == 4)                                  // Выполнить команду 4
	{
	


	}
	else if(var == 5)                                  // Выполнить команду 5
	{
		  EEPROM.put(Address_errorAll, 0);             // Сбросить счетчик ошибок
	}
	else if(var == 6)                                  // Выполнить команду 6
	{
		command.remove(0, 2);                          // Получить данные номера телефона от сервера
		EEPROM.get(Address_SMS_center, data_tel);      // Получить из EEPROM СМС центр
		String num_tel(data_tel);
		if (command != num_tel)                        // Если информиция не изменилась - не писать в EEPROM
		{
			Serial.println(F("no compare"));
			for(int i=0;i<13;i++)
			{
				EEPROM.write(i+Address_SMS_center,command[i]);
			}
		}
	}
	else if(var == 7)                                  // Выполнить команду 7
	{
		time_set = false;                              // Снять фиксацию интервала заданного СМС
	}
	else if(var == 8)                                  // Выполнить команду 8
	{
		//  Здесь и далее можно добавить до 90 команд  
	}
	else
	{
		// здесь можно что то выполнить если команда не пришла
	}
	
  // Показать статистику
  con.print(F("Total:"));                  //con.print("Total:");
  con.print(count);
  if (errors)                               // Если есть ошибки - сообщить
  {
	con.print(F(" Errors:")); //con.print(" Errors:");
	con.print(errors);
  }
  con.println();
  return send_ok;
}

void errorAllmem()                            // Запись всех ошибок в память EEPROM
{
  int error_All;
  EEPROM.get(Address_errorAll, error_All);
  error_All++;
  EEPROM.put(Address_errorAll, error_All);            
}

int freeRam ()                                   // Определить свободную память
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setTime(String val, String f_phone)
{
  if (val.indexOf(F("Timeset")) > -1)         // 
  {
	 interval = 40;                                     // Установить интервал 40 секунд
	 time_set = true;                                   // Установить фиксацию интервала заданного СМС
	 Serial.println(interval);
  } 
  else if (val.indexOf(F("Restart")) > -1) 
  {
	  Serial.print(f_phone);
	  Serial.print("..");
	  Serial.println(F("Restart"));
	  SMS_delete();
	  delay(2000);
	  resetFunc();                                     //вызываем reset
  } 
  else if (val.indexOf(F("Timeoff")) > -1) 
  {
	 time_set = false;                              // Снять фиксацию интервала заданного СМС
	 Serial.println(F("Timeoff"));
	 SMS_delete();
  } 
  else if (val.indexOf(F("Sslon")) > -1)
  {
	  EEPROM.write(Address_ssl, true);                // Включить шифрование
	  Serial.println(F("HTTP SSL ON"));
	  SMS_delete();
	  delay(2000);
	  resetFunc();
  }
  else if (val.indexOf(F("Ssloff")) > -1)
  {
	  EEPROM.write(Address_ssl, false);                  // Отключить шифрование
	  Serial.println(F("HTTP SSL OFF"));
	  SMS_delete();
	  delay(2000);
	  resetFunc();
  }
  else
  {
	  Serial.println(F("Unknown command"));         // Serial.println("Unknown command");
	  if (gprs.deleteSMS(1))
	  SMS_delete();
  }
}

void SMS_delete()
{
	if (gprs.deleteSMS(1))
	{
		con.println(F("SMS delete"));                    //  con.print("SMS:");
	}
}

void check_blink()
{
	unsigned long current_M = millis();

	wdt_reset();
	
	metering_NETLIGHT = current_M - metering_temp; // переделать для уменьшения
	metering_temp = current_M;
	//Serial.println(metering_NETLIGHT);
	if (metering_NETLIGHT > 3055 && metering_NETLIGHT < 3070)
	{
		state_device = 2;                 // 2 - Зарегистрировано в сети
		count_blink2++;
		if(count_blink2 > 20)    resetFunc(); // Что то пошло не так с регистрацией на станции
	}
	else if (metering_NETLIGHT > 855 && metering_NETLIGHT < 870)
	{
		state_device = 1;                // 1 Не зарегистрирован в сети, поиск

		count_blink1++;
		if (count_blink1 > 120)    resetFunc(); // Что то пошло не так с регистрацией на станции
	}
	else if (metering_NETLIGHT > 350 && metering_NETLIGHT < 370)
	{
		state_device = 3;                // 3 - GPRS связь установлена
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

bool check_ping()
{
	con.print(F("Ping -> "));
	con.print(url_ping);
	if (gprs.ping(url_ping))
	{
		con.println(F(".. Ok!"));
		return true;
	}
	con.println(F(".. false!"));
	return false;
}
void ping()
{
	int count_ping = 0;
	while (1)
	{
		if (check_ping())
		{
			return;
		}
		else
		{
			count_ping++;
			if (count_ping > 7) resetFunc(); // 7 попыток. Что то пошло не так с интернетом
		}
		delay(1000);
	}
}

void start_init()
{
	bool setup_ok = false;
	uint8_t count_init = 0;
	do
	{
		con.println(F("Initializing....(May take 5-10 seconds)"));

		digitalWrite(SIM800_RESET_PIN, LOW);                      // Сигнал сброс в исходное состояние
		digitalWrite(LED13, LOW);
		digitalWrite(PWR_On, HIGH);                               // Кратковременно отключаем питание модуля GPRS
		delay(2000);
		digitalWrite(LED13, HIGH);
		digitalWrite(PWR_On, LOW);
		delay(1500);
		digitalWrite(SIM800_RESET_PIN, HIGH);                     // Производим сброс модема после включения питания
		delay(1000);
		digitalWrite(SIM800_RESET_PIN, LOW);
		int count_status = 0;
		while (digitalRead(STATUS) == LOW)
		{
			count_status++;
			if(count_status > 100) resetFunc(); // 100 попыток. Что то пошло не так программа перезапуска  если модуль не включился
			delay(100);
		}
		delay(2000);
		con.println(F("Power SIM800 On"));

		GPRSSerial->begin(19200);                               // Скорость обмена с модемом SIM800C

		while (!gprs.begin(*GPRSSerial))
		{
			Serial.println(F("Couldn't find module GPRS"));
			while (1);
		}
		con.println(F("OK"));                  // 


		if (gprs.getIMEI())                       // Получить IMEI
		{
			con.print(F("\nIMEI:"));
			//imei = gprs.buffer;                 // Отключить на время отладки
			//gprs.cleanStr(imei);                // Отключить на время отладки
			con.println(imei);
		}
		else
		{
			return;  // IMEI не определился
		}

		if (gprs.getSIMCCID())                       // Получить Номер СИМ карты
		{
			con.print(F("\nSIM CCID:"));
			SIMCCID = gprs.buffer;                 // Отключить на время отладки
			gprs.cleanStr(SIMCCID);                // Отключить на время отладки
			con.println(SIMCCID);
		}
		else
		{
			//return;  // SIMCCID не определился
		}

		if (gprs.getGMR())                       // Получить номер прошивки
		{
			con.print(F("\nSoftware release:"));
			String GMR  = gprs.buffer;                 // 
			gprs.cleanStr(GMR);                // 
			con.println(GMR);
		}

		

		while (state_device != 2)  // Ожидание регистрации в сети
		{
			delay(1000);
			// Уточнить программу перезапуска  если модуль не зарегистрировался через 60 секунд
		}
		delay(1000);
		Serial.print(F("\nSetting up network..."));

		char n = gprs.getNetworkStatus();
		
		Serial.print(F("\nNetwork status "));
		Serial.print(n);
		Serial.print(F(": "));
		if (n == '0') Serial.println(F("\nNot registered"));                      // 0 – не зарегистрирован, поиска сети нет
		if (n == '1') Serial.println(F("\nRegistered (home)"));                   // 1 – зарегистрирован, домашняя сеть
		if (n == '2') Serial.println(F("\nNot registered (searching)"));          // 2 – не зарегистрирован, идёт поиск новой сети
		if (n == '3') Serial.println(F("\nDenied"));                              // 3 – регистрация отклонена
		if (n == '4') Serial.println(F("\nUnknown"));                             // 4 – неизвестно
		if (n == '5') Serial.println(F("\nRegistered roaming"));                  // 5 – роуминг

		if (n == '1' || n == '5')                                                 // Если домашняя сеть или роуминг
		{
			if (state_device == 2)                 // Проверить аппаратно подключения модема к оператору
			{
				delay(2000);
				do
				{
					int signal = gprs.getSignalQuality();
					Serial.print(F("rssi ..")); Serial.println(signal);
					delay(1000);
					Serial.println(F("GPRS connect .."));
					byte ret = gprs.setup();                                              // Подключение к GPRS
					//Serial.print("ret - "); Serial.print(ret);
					if (ret == 0)
					{
						while (state_device != 3)  // Ожидание регистрации в сети
						{
							delay(50);
							// Уточнить программу перезапуска  если модуль не зарегистрировался не зарегистрировался через 60 секунд
						}
						Serial.println(F("\nGPRS connect OK!+"));
						//setColor(COLOR_GREEN);                 // Включить зеленый светодиод
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
							//setColor(COLOR_GREEN);                 // Включить зеленый светодиод
							setup_ok = true;
						}
					}
				} while (!setup_ok);
			}
		}
	} while (count_init > 20 || setup_ok == false);    // 20 попыток зарегистрироваться в сети
}

void setup()
{
	wdt_disable(); // бесполезна¤ строка до которой не доходит выполнение при bootloop Не уверен!!
	con.begin(speed_Serial);
	con.println(F("\n SIM800 setup start"));     

	pinMode(SIM800_RESET_PIN, OUTPUT);
	pinMode(LED13, OUTPUT);
	pinMode(PWR_On, OUTPUT);

	pinMode(LED_RED,  OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
	pinMode(LED_GREEN,OUTPUT);
	pinMode(NETLIGHT ,INPUT);                      // Индикация NETLIGHT
	pinMode(STATUS ,INPUT);                        // Индикация STATUS

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

	attachInterrupt(1,check_blink, RISING);            // Включить прерывания. Индикация состояния модема
	delay(3000);
	wdt_enable(WDTO_8S);                               // Для тестов не рекомендуется устанавливать значение менее 8 сек.

	start_init();
	
	con.println(F("OK"));            
	for (;;) 
	{
		if (gprs.httpInit()) break;                        // Все нормально, модуль ответил , Прервать попытки и выйти из цикла
		con.print(">");
		con.println(gprs.buffer);                          // Не получилось, ("ERROR") 
		String stringError = gprs.buffer;             
		if (stringError.indexOf(F("ERROR")) > -1)          
			{
				con.print(F("\nNo internet connection"));     
				delay(1000);
				resetFunc();                                //вызываем reset при отсутствии доступа к серверу
			}
		gprs.httpUninit();                                  // Не получилось, попробовать снова 
		delay(1000);
	}

	if(EEPROM.read(0)!=34)
	{
		con.println (F("Start clear EEPROM"));               //  
		for(int i = 0; i<1023;i++)
		{
			EEPROM.write(i,0);
		}
		EEPROM.write(0,34);
		EEPROM.put(Address_interval, interval);                  // строка начальной установки интервалов
		EEPROM.put(Address_tel1, "+79852517615");             
			//EEPROM.put(Address_SMS_center, "4556w6072556w6");
		EEPROM.write(Address_ssl, false);
		con.println (F("Clear EEPROM End"));                              
	}

	SMS_center = "4556w6072556w6";                               // SMS_center = "SMS.RU";
	//EEPROM.put(Address_interval, interval);                    // Закоментировать строку после установки интервалов
	EEPROM.put(Address_SMS_center, SMS_center);                  // Закоментировать строку после установки СМС центра
	EEPROM.get(Address_interval, interval);                      // Получить из EEPROM интервал
	//EEPROM.get(Address_SMS_center, SMS_center);                // Получить из EEPROM СМС центр
	ssl_set = EEPROM.read(Address_ssl);							 // Устанивить признак шифрования
	con.print(F("Interval sec:"));
	con.println(interval);
	con.print(F("SMS_center .."));
	con.println(SMS_center);

	con.print(F("\nfree memory: "));
	con.println(freeRam());

	if (ssl_set==true)
	{
		if (gprs.HTTP_ssl(true))
		{
			con.println(F("\nHTTP_ssl: set ON successfully!"));
		}
		else
		{
			con.println(F("\nHTTP_ssl: set ON false!"));
		}
	}
	else
	{
		if (gprs.HTTP_ssl(false))
		{
			con.println(F("\nHTTP_ssl: set OFF successfully!"));
		}
		else
		{
			con.println(F("\nHTTP_ssl: set OFF false!"));
		}

	}
	if (gprs.val.indexOf("REC READ") > -1)               //если обнаружена старая  СМС 
	{
		if (gprs.deleteSMS(0))
		{
			con.println(F("All SMS delete"));                    // 
		}
	}

	delay(2000);
	ping();

	setColor(COLOR_GREEN);                                      // Включить зеленый светодиод

	con.println(F("\nSIM800 setup end"));
	//sendTemps();
	time = millis();                                              // Старт отсчета суток
	
}

void loop()
{
 if(digitalRead(STATUS) == LOW)  resetFunc();   // Что то пошло не так, питание отключено

 if (gprs.checkSMS()) 
  {
	con.print(F("SMS:"));                    
	con.println(gprs.val); 
	
	if (gprs.val.indexOf("REC UNREAD") > -1)  //если обнаружена новая  СМС 
	{    
	//------------- поиск кодового слова в СМС 
	char buf[16] ;

	EEPROM.get(Address_tel1, buf);                                         // Восстановить телефон хозяина 1
	String master_tel1(buf);

	//EEPROM.get(Address_SMS_center, buf);                                   // Восстановить телефон СМС центра
	//String master_SMS_center(buf);
	String master_SMS_center = "4556w6072556w6";
	//con.println(master_SMS_center);

	  if (gprs.val.indexOf(master_tel1) > -1)                              //если СМС от хозяина 1
	  {   
		con.println(F("Commanda tel1"));
		setTime(gprs.val, master_tel1);
	  }
	  else if(gprs.val.indexOf(master_SMS_center) > -1)                    //если СМС от хозяина 2
	  {
		con.println(F("SMS centr"));
		setTime(gprs.val, master_SMS_center);
	  }
	  else
	  {
		  con.println(F("phone ignored"));            
	  }
	}
	
	if (gprs.val.indexOf("REC READ") > -1)               //если обнаружена старая  СМС 
	{
		if (gprs.deleteSMS(0))
		{
			con.println(F("SMS delete"));                    //  con.print("SMS:");
		}
	}

	gprs.val = "";
		
  }
 
 
	unsigned long currentMillis = millis();
	if(!time_set)                                                               // 
	{
		 EEPROM.get( Address_interval, interval);                               // Получить интервал из EEPROM Address_interval
	}
	if ((unsigned long)(currentMillis - previousMillis) >= interval*1000) 
	{
		con.print(F("Interval sec:"));                                       
		con.println((currentMillis-previousMillis)/1000);
		setColor(COLOR_BLUE);
		previousMillis = currentMillis;
		sendTemps();
		setColor(COLOR_GREEN);
		con.print(F("\nfree memory: "));                                 
		con.println(freeRam());
	}

	currentMillis = millis();

	if ((unsigned long)(currentMillis - previousPing) >= time_ping * 1000)
	{
		con.print(F("Interval ping sec:"));
		con.println((currentMillis - previousPing) / 1000);
		setColor(COLOR_BLUE);
		previousPing = currentMillis;
		ping();
		setColor(COLOR_GREEN);
	}

	if(millis() - time > time_day*1000) resetFunc();                       //вызываем reset интервалом в сутки
	delay(500);
}
