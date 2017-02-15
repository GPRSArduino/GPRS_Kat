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
#include <MsTimer2.h> 

#define con Serial
#define speed_Serial 115200
static const char* url1 = "http://trm7.xyz/r.php";
//static const char* url1   = "http://vps3908.vps.host.ru/recieveReadings.php";
static const char* urlssl = "https://trm7.xyz/r.php";
static const char* url_ping = "www.yandex.ru";

#define PIN_TX           7                              // Подключить  к выводу 7 сигнал RX модуля GPRS
#define PIN_RX           8                              // Подключить  к выводу 8 сигнал TX модуля GPRS

SoftwareSerial SIM800CSS = SoftwareSerial(PIN_RX, PIN_TX);
SoftwareSerial *GPRSSerial = &SIM800CSS;
 

#define PWR_On           5                              // Включение питания модуля SIM800
#define SIM800_RESET_PIN 6                              // Сброс модуля SIM800
#define LED13           13                              // Индикация светодиодом
#define NETLIGHT         3                              // Индикация NETLIGHT
#define STATUS           9                              // Индикация STATUS
												
//#define COMMON_ANODE                                  // Если светодиод с общим катодом - раскомментировать
#define LED_RED      10                                 // Индикация светодиодом RED
#define LED_BLUE     15                                 // Индикация светодиодом BLUE
#define LED_GREEN    14                                 // Индикация светодиодом GREEN

#define COLOR_NONE LOW, LOW, LOW                        // Отключить все светодиоды
#define COLOR_GREEN LOW, HIGH, LOW                      // Включить зеленый светодиод
#define COLOR_BLUE LOW, LOW, HIGH                       // Включить синий светодиод
#define COLOR_RED HIGH, LOW, LOW                        // Включить красный светодиод
volatile int stateLed = LOW;                            // Состояние светодиода при прерывистой индикации на старте
volatile int state_device = 0;                          // Состояние модуля при запуске 

												        // 1 - Не зарегистрирован в сети, поиск
												        // 2 - Зарегистрировано в сети
												        // 3 - GPRS связь установлена
volatile int metering_NETLIGHT       = 0;
volatile unsigned long metering_temp = 0;
volatile int count_blink1            = 0;               // Счетчик попыток подключиться к базовой станции
volatile int count_blink2            = 0;               // Счетчик попыток подключиться к базовой станции
bool send_ok                         = false;           // Признак успешной передачи данных
bool count_All_reset                 = false;           // Признак выполнения команды сброса счетчика ошибок.

CGPRS_SIM800 gprs;
uint32_t count      = 0;
uint32_t errors     = 0;
//String imei       = "";
String imei         = "861445030362268";                // Тест IMEI
String CSQ          = "";                               // Уровень сигнала приема
String SMS_center   = "";
String zero_tel     = "";
String SIMCCID      = "";
String CMTE         = "";                               // Внутренний датчик температуры
#define DELIM "@"

unsigned long time             = 0;                     // Переменная для суточного сброса
unsigned long time_day         = 86400;                 // Переменная секунд в сутках
unsigned long previousMillis   = 0;                     //  
unsigned long interval         = 60;                    // Интервал передачи данных 60 секунд
//unsigned long interval         = 300;                   // Интервал передачи данных 5 минут
bool time_set                  = false;                 // Фиксировать интервал заданный СМС
bool ssl_set                   = false;                 // Признак шифрования
unsigned long time_ping        = 180;                   // Интервал проверки ping 6 минут. 
unsigned long previousPing     = 0;                     // Временный Интервал проверки ping

int Address_tel1       = 100;                           // Адрес в EEPROM телефона 1
int Address_ssl        = 120;                           // Адрес в EEPROM признака шифрования
int Address_errorAll   = 160;                           // Адрес в EEPROM счетчика общих ошибок
int Address_interval   = 200;                           // Адрес в EEPROM величины интервала
int Address_SMS_center = 220;                           // Адрес в EEPROM SMS центра

char data_tel[16];                                      // Буфер для номера телефоа

uint8_t oneWirePins[]={16, 17, 4};                      //номера датчиков температуры DS18x20. Переставляя номера можно устанавливать очередность передачи в строке.
													    // Сейчас первым идет внутренний датчик.
uint8_t oneWirePinsCount=sizeof(oneWirePins)/sizeof(int);

OneWire ds18x20_1(oneWirePins[0]);
OneWire ds18x20_2(oneWirePins[1]);
OneWire ds18x20_3(oneWirePins[2]);
DallasTemperature sensor1(&ds18x20_1);
DallasTemperature sensor2(&ds18x20_2);
DallasTemperature sensor3(&ds18x20_3);

void(* resetFunc) (void) = 0;                           // объявляем функцию reset

void flash_time()                                       // Программа обработчик прерывистого свечения светодиодов при старте
{
	if (state_device == 0)
	{
		setColor(COLOR_NONE);
	}
	if (state_device == 1)
	{
		stateLed = !stateLed;
		if (!stateLed)
		{
			setColor(COLOR_RED);
		}
		else
		{
			setColor(COLOR_NONE);
		}
	}

	if (state_device == 2)
	{
		stateLed = !stateLed;
		if (!stateLed)
		{
			setColor(COLOR_NONE);
		}
		else
		{
			setColor(COLOR_BLUE);
		}
	}

	if (state_device == 3)
	{
		stateLed = !stateLed;
		if (!stateLed)
		{
			setColor(COLOR_NONE);
		}
		else
		{
			setColor(COLOR_GREEN);
		}
	}
}

 void setColor(bool red, bool green, bool blue)        // Включение цвета свечения трехцветного светодиода.
 {
	  #ifdef COMMON_ANODE                              // Если светодиод с общим катодом
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

	String toSend = "t1=" + imei + DELIM + "17/2/1,21:2:28%2000" DELIM + String(t1) + DELIM + String(t2) + DELIM + String(t3) + DELIM + String(signal) + DELIM + String(errors) + DELIM + String(error_All) + formEnd() + DELIM + String(tsumma);

	//Serial.println(toSend);

	Serial.println(toSend.length());
	gprs_send(toSend);
	//int count_send = 0;
	//while (1)
	//{
	//	if (gprs_send(toSend))
	//	{
	//		return;
	//	}
	//	else
	//	{
	//		count_send++;
	//		Serial.print("Attempt to transfer data .."); Serial.println(count_send);
	//		if (count_send>5) resetFunc();                                // 5 попыток. Что то пошло не так с интернетом
	//		//ping();
	//	}
	//	delay(6000);
	//}

}


String formEnd() 
{
	char buf[13] ;

	EEPROM.get(Address_tel1, buf);
	String master_tel1(buf);

	EEPROM.get(Address_SMS_center, SMS_center);   //Получить из EEPROM СМС центр

	return DELIM + master_tel1 + DELIM + SIMCCID;

}


bool gprs_send(String data) 
{
  con.print(F("Requesting .. Wait"));      

  connect_internet_HTTP();


  int count_init = 0;                                    // Счетчик количества попыток подключиться к HTTP
  for (;;)                                               // Бесконечный цикл пока не наступит, какое то состояние для выхода
  {
	  if (gprs.httpInit()) break;                        // Все нормально, модуль ответил , Прервать попытки и выйти из цикла
	  con.print(">");
	  con.println(gprs.buffer);                          // Не получилось, ("ERROR") 
	  String stringError = gprs.buffer;
	  if (stringError.indexOf(F("ERROR")) > -1)          
	  {
		  con.println(F("\nNo internet connection"));
		  delay(1000);
	  }
	  gprs.httpUninit();                                 // Не получилось, разединить и  попробовать снова 
	  delay(1000);                                       // Подождать секунду.
	  count_init++;
	  if(count_init > 60)  resetFunc();                 //вызываем reset при отсутствии доступа к серверу в течении 60 секунд
  }

  if (ssl_set == true)
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
 
  if (ssl_set == true)
  {
	  con.print(urlssl);
	  con.print('?');
	  con.println(data);

	  gprs.httpConnectStr(urlssl, data);
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
			con.println(F("Errors exceeded"));
			delay(3000);
			resetFunc();          // вызываем reset после 20 ошибок
	  }
	delay(3000);
	return; 
  }
  
  con.println();
  gprs.httpRead();
  int ret;
  while ((ret = gprs.httpIsRead()) == 0)  //  Ожидаем сообщение HTTP
  {
	// может сделать что-то здесь, во время ожидания
  }

  if (gprs.httpState == HTTP_ERROR)        // Ответ не пришел
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
   con.print(F("[Payload] "));                        
   con.println(gprs.buffer);
   String val = gprs.buffer;                            // Получить строку данных с сервера
  // gprs.httpUninit();                                    // Разорвать соединение

   int p0[5];
  // String val = "&010145&0202+79162632701&0303+79162632701&0400123456789#";  // Пример строки, принятой с сервера
   send_ok = true;                                                             // Команда принята успешно

   if (val.indexOf("&") > -1)              // Определить адреса (позиции) команд в строке 
   {
	   p0[0] = val.indexOf("&01");         // Адрес 1 команды
	   p0[1] = val.indexOf("&02");         // Адрес 2 команды
	   p0[2] = val.indexOf("&03");         // Адрес 3 команды
	   p0[3] = val.indexOf("&04");         // Адрес 4 команды
	   p0[4] = val.indexOf('#');           // Адрес конца команд
   }

   for (int i=0;i<4;i++)
   {
	  String comm = val.substring(p0[i]+3, p0[i]+5);          // Выделить строку с номером команды
	  int comm1 = comm.toInt(); 
	  Serial.println(comm1);
	  comm = val.substring(p0[i] + 5, p0[i+1]);               // Выделить строку с данными
	  int len_str = comm.length();
	  comm[len_str] = '\0';
	  Serial.println(comm);

	  run_command(comm1, comm);                               // Последовательно выполнить все команды

   }

  // Показать статистику
  con.print(F("Total: "));                                   
  con.print(count);
  if (errors)                                           // Если есть ошибки - сообщить
  {
	con.print(F(" Errors: "));                                
	con.print(errors);
  }
  con.println();
  Serial.print("Inteval: ");
  Serial.println(interval);
  gprs.httpUninit();                                    // Разорвать соединение

  return send_ok;
}


void connect_internet_HTTP()
{
	bool setup_ok = false;
	int count_init = 0;
	do
	{
		Serial.println(F("Internet HTTP connect .."));
		
		byte ret = gprs.connect_GPRS();                                              // Подключение к GPRS
		Serial.print(F("ret - ")); Serial.print(ret);
		if (ret == 0)
		{
			while (state_device != 3)  // Ожидание регистрации в сети
			{
				delay(50);
				// Уточнить программу перезапуска  если модуль не зарегистрировался не зарегистрировался через 60 секунд
			}

			con.println(F("Wait IP"));
			gprs.connect_IP_GPRS();                             // Получить IP адрес
			Serial.println(F("Internet connect OK!-"));
			//setColor(COLOR_GREEN);                            // Включить зеленый светодиод
			setup_ok = true;
		}
		else                                                    // Модуль не подключиля к интернету
		{
			count_init++;                                        // Увеличить счетчик количества попыток
			if(count_init > 120) resetFunc();                    // вызываем reset после 120 ошибок

			Serial.println(F("\nFailed connect internet"));
			delay(1000);
			if (state_device == 3)                              // Модуль одумался и все таки подключиля к интернету
			{
				Serial.println(F("Internet connect OK!-"));
				//setColor(COLOR_GREEN);                        // Включить зеленый светодиод
				setup_ok = true;
			}
		}
	} while (!setup_ok);             // 
}




void run_command(int command, String data)
{
	unsigned long interval1 = 0;
	switch (command) 
	{
		case 1:
			interval1 = data.toInt();                              // Преобразовать строку данных в число 
			con.println(interval1);
			if (interval1 > 29 && interval1 < 86401)               // Ограничить интервалы от 29  секунд до 24 часов.
			{
				if (interval1 != interval)                         // Если информиция не изменилась - не писать в EEPROM
				{
					if (!time_set)                                 // Если нет команды фиксации интервала от СМС 
					{
						interval = interval1;                      // Переключить интервал передачи на сервер
						EEPROM.put(Address_interval, interval1);   // Записать интервал EEPROM , полученный от сервера
					}
				}
			}
			con.println(interval);
			break;
		case 2:
			EEPROM.get(Address_tel1, data_tel);             // Получить номер телефона из EEPROM
			Serial.println(data_tel);
			if (data != data_tel)                           // Если информиция не изменилась - не писать в EEPROM
			{
				con.println(F("no compare"));               //Serial.println("no compare");
				char buf[16];
				for (int i = 0; i<13; i++)
				{
					EEPROM.write(i + Address_tel1, data[i]);
				}
				EEPROM.get(Address_tel1, buf);
				Serial.println(buf);
			}
			else
			{
				con.println(F("Tel compare"));                
			}
			break;
		case 3:
			EEPROM.get(Address_SMS_center, data_tel);      // Получить из EEPROM СМС центр
			if (data != data_tel)                        // Если информиция не изменилась - не писать в EEPROM
			{
				Serial.println(F("no compare"));
				for (int i = 0; i<13; i++)
				{
					EEPROM.write(i + Address_SMS_center, data[i]);
				}
			}

			break;
		case 4:
			if (count_All_reset = false)                   // Признак выполнения команды сброса счетчика ошибок
			{
				count_All_reset == true;                   // Команда сброса выполнена. Повторный сброс возможен после перезагрузки
				EEPROM.put(Address_errorAll, 0);             // Сбросить счетчик ошибок Предусмотреть блокировку повторной записи???
			}
			break;
		case 5:
			time_set = false;                              // Снять фиксацию интервала заданного СМС
			break;
		//case 6:
		//	//выполняется когда  var равно 6
		//	break;
		//case 7:
		//	//выполняется, когда var равно 7
		//	break;
		//case 8:
		//	//выполняется когда  var равно 8
		//	break;
		default:
			break;
			// выполняется, если не выбрана ни одна альтернатива
			// default необязателен
	}
}

void errorAllmem()                                // Запись всех ошибок в память EEPROM
{
  int error_All;
  EEPROM.get(Address_errorAll, error_All);
  error_All++;
  EEPROM.put(Address_errorAll, error_All);            
}

int freeRam()                                   // Определить свободную память
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setTime(String val, String f_phone)
{
  if (val.indexOf(F("Timeset")) > -1)                   // 
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
	  delay(2000);
	  resetFunc();                                     //вызываем reset
  } 
  else if (val.indexOf(F("Timeoff")) > -1) 
  {
	 time_set = false;                              // Снять фиксацию интервала заданного СМС
	 Serial.println(F("Timeoff"));
  } 
  else if (val.indexOf(F("Sslon")) > -1)
  {
	  EEPROM.write(Address_ssl, true);                // Включить шифрование
	  Serial.println(F("HTTP SSL ON"));
	  delay(2000);
	  resetFunc();
  }
  else if (val.indexOf(F("Ssloff")) > -1)
  {
	  EEPROM.write(Address_ssl, false);                  // Отключить шифрование
	  Serial.println(F("HTTP SSL OFF"));
	  delay(2000);
	  resetFunc();
  }
  else
  {
	  Serial.println(F("Unknown command"));         // Serial.println("Unknown command");
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
		state_device = 2;                                                     // 2 - Зарегистрировано в сети
		count_blink2++;
		if (count_blink2 > 50)
		{
			state_device = 0;
			MsTimer2::stop();                                                 // Включить таймер прерывания
	    	resetFunc();                                                      // Что то пошло не так с регистрацией на станции
	    }
	}
	else if (metering_NETLIGHT > 855 && metering_NETLIGHT < 870)
	{
		state_device = 1;                // 1 Не зарегистрирован в сети, поиск
		count_blink1++;
		if (count_blink1 > 120) 
		{
			state_device = 0;
			MsTimer2::stop();                                                 // Включить таймер прерывания
			resetFunc();                   // Что то пошло не так с регистрацией на станции
		}
	}
	else if (metering_NETLIGHT > 350 && metering_NETLIGHT < 370)
	{
		state_device = 3;                // 3 - GPRS связь установлена

                                      
	}
}


void ping()
{
	int count_ping = 0;
	int count_wait = 0;                                                // Счетчик количества попыток проверки подключения Network registration (сетевому оператору)
	byte ret = gprs.ping_connect_internet();                     

	if(ret !=0)   resetFunc();                                        // Что то пошло не так с интернетом

	while (state_device != 3)                                         // Ожидание подключения к интернету
	{
		delay(50);
		count_wait++;
		if (count_wait > 3000)  resetFunc();                           //вызываем reset при отсутствии доступа к  интернету
	}

	while (1)
	{
		if (check_ping())
		{
			return;
		}
		else
		{
			count_ping++;
			if (count_ping > 7) resetFunc();                           // 7 попыток. Что то пошло не так с интернетом
		}
		delay(1000);
	}
}




bool check_ping()
{
	con.print(F("Ping -> "));
	con.println(url_ping);
	if (gprs.ping(url_ping))
	{
		con.println(F(".. Ok!"));
		return true;
	}
	con.println(F(".. false!"));
	return false;
}


void start_init()
{
	bool setup_ok = false;
	uint8_t count_init = 0;
	MsTimer2::start();                                                 // Включить таймер прерывания
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
		delay(1200);
		digitalWrite(SIM800_RESET_PIN, LOW);
		int count_status = 0;
		while (digitalRead(STATUS) == LOW)
		{
			count_status++;
			if(count_status > 150) resetFunc(); // 100 попыток. Что то пошло не так программа перезапуска  если модуль не включился
			delay(100);
		}
		delay(2000);
		con.println(F("Power SIM800 On"));

		GPRSSerial->begin(19200);                               // Скорость обмена с модемом SIM800C

		while (!gprs.begin(*GPRSSerial))                        // Настройка модуля SIM800C
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
			SIMCCID = gprs.buffer;                 //  
			gprs.cleanStr(SIMCCID);                //  
			con.println(SIMCCID);
		}
		else
		{
			//return;  // SIMCCID не определился
		}

		while (state_device != 2)  // Ожидание регистрации в сети
		{
			delay(1000);
			// Уточнить программу перезапуска  если модуль не зарегистрировался через 70 секунд
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
			if (state_device == 2)                                                // Проверить аппаратно подключения модема к оператору
			{
				delay(2000);
				do
				{
					int signal = gprs.getSignalQuality();
					Serial.print(F("rssi ..")); Serial.println(signal);
					delay(1000);
					Serial.println(F("GPRS connect .."));
					gprs.getOperatorName();
					setup_ok = true;

				} while (!setup_ok);             // 
			}
		}
	} while (count_init > 30 || setup_ok == false);    // 30 попыток зарегистрироваться в сети
}

void setup()
{
	wdt_disable(); // бесполезна¤ строка до которой не доходит выполнение при bootloop Не уверен!!
	con.begin(speed_Serial);
	con.println(F("\nSIM800 setup start"));     

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
	delay(1000);
	wdt_enable(WDTO_8S);                               // Для тестов не рекомендуется устанавливать значение менее 8 сек.
	MsTimer2::set(300, flash_time);                    // 30ms период таймера прерывани
	start_init();
	
	int count_init = 0;                                    // Счетчик количества попыток подключиться к HTTP
	con.println(F("OK"));   

	if(EEPROM.read(0)!=32)
	{
		con.println (F("Start clear EEPROM"));               //  
		for(int i = 0; i<1023;i++)
		{
			EEPROM.write(i,0);
		}
		EEPROM.write(0,32);
		EEPROM.put(Address_interval, interval);                  // строка начальной установки интервалов
		EEPROM.put(Address_tel1, "+79162632701");      
		//EEPROM.put(Address_tel1, "+79852517615");
		//EEPROM.put(Address_SMS_center, "4556w6072556w6");
		EEPROM.write(Address_ssl, false);
		con.println (F("Clear EEPROM End"));                              
	}

	//SMS_center = "4556w6072556w6";                               // SMS_center = "SMS.RU";
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

	if (gprs.val.indexOf("REC READ") > -1)               //если обнаружена старая  СМС 
	{
		if (gprs.deleteSMS(0))
		{
			con.println(F("All SMS delete"));                    // 
		}
	}

	delay(2000);
	ping();
	MsTimer2::stop();
	setColor(COLOR_GREEN);                                      // Включить зеленый светодиод
	sendTemps();
	time = millis();                                             // Старт отсчета суток
	con.println(F("\nSIM800 setup end"));
}

void loop()
{
 if(digitalRead(STATUS) == LOW)  resetFunc();                                 // Что то пошло не так, питание отключено

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

		//EEPROM.get(Address_SMS_center, buf);                                 // Восстановить телефон СМС центра
		//String master_SMS_center(buf);
		String master_SMS_center = "4556w6072556w6";
		//con.println(master_SMS_center);
		if (gprs.deleteSMS(1))
		{
			con.println(F("SMS delete"));                    //      con.print("SMS:");
		}

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

	if(millis() - time > time_day*1000) resetFunc();                         //вызываем reset интервалом в сутки
	delay(500);
}
