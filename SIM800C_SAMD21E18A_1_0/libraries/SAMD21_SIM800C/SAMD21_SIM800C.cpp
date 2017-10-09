/***********************************
This is our GPS library

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
BSD license, check license.txt for more information
All text above must be included in any redistribution
****************************************/

#include <SAMD21_SIM800C.h>

void SAMD21_SIM800C::begin(uint32_t baud)
{
    SIM800CHwSerial->begin(baud);
	int16_t timeout = 7000;

	while (timeout > 0)
	{
		while (SIM800CHwSerial->available()) SIM800CHwSerial->read();
		if (sendCommandS(F("AT")))
		{
			break;
		}
		delay(500);
		timeout -= 500;
	}
  delay(10);
}


char SAMD21_SIM800C::read(void) 
{

}


// Constructor when using HardwareSerial
SAMD21_SIM800C::SAMD21_SIM800C(HardwareSerial *ser) 
{
   SIM800CHwSerial = ser; // ...override gpsHwSerial with value passed.
}



void SAMD21_SIM800C::sendCommand(const char *str) 
{
    SIM800CHwSerial->println(str);
}
// read a Hex value and return the decimal equivalent
uint8_t SAMD21_SIM800C::parseHex(char c) {
    if (c < '0')
      return 0;
    if (c <= '9')
      return c - '0';
    if (c < 'A')
       return 0;
    if (c <= 'F')
       return (c - 'A')+10;
    // if (c > 'F')
    return 0;
}


byte SAMD21_SIM800C::connect_GPRS()
{
	for (byte n = 0; n < 10; n++)
	{
		if (!sendCommandS(F("AT+SAPBR=3,1,\"Contype\",\"GPRS\""))) return 1;           

		SIM800CHwSerial->print(F("AT+SAPBR=3,1,\"APN\",\""));
		SIM800CHwSerial->print(apn);
		SIM800CHwSerial->println('\"');
		if (!sendCommandS(no))   return 2;   

		SIM800CHwSerial->print(F("AT+SAPBR=3,1,\"USER\",\""));
		SIM800CHwSerial->print(user);
		SIM800CHwSerial->println('\"');
		if (!sendCommandS(no))   return 2;

		SIM800CHwSerial->print(F("AT+SAPBR=3,1,\"PWD\",\""));
		SIM800CHwSerial->print(pwd);
		SIM800CHwSerial->println('\"');
		if (!sendCommandS(no))   return 2;

		SIM800CHwSerial->print(F("AT+CGDCONT=1,\"IP\",\""));
		SIM800CHwSerial->print(cont);
		SIM800CHwSerial->println('\"');
		if (!sendCommandS(no))   return 2;

		timeout = 15000;
		Serial.print(F("Connect GPRS..")); Serial.println(n+1);
		sendCommandS(F("AT+SAPBR=1,1"));  timeout = 2000;   return 0;                 // установка GPRS связи
	}
	return 3;                                                                         // Неуспешная регистрация
}

bool SAMD21_SIM800C::connect_IP_GPRS()
{
	for (byte n = 0; n < 30; n++)
	{
		timeout = 15000;
		if (sendCommandS(F("AT+SAPBR=2,1"))) timeout = 2000;  return true;                             // получить IP адрес
		delay(500);
	}
	return false;
}


void SAMD21_SIM800C::cleanStr(String & str) 
{
  str.replace("OK", "");
  str.replace("\"", "");
  str.replace("\n", "");
  str.replace("\r", "");
  str.trim();
}

uint8_t SAMD21_SIM800C::getNetworkStatus()
{
	for (byte n = 0; n < 30; n++)
	{
		if (sendCommandS(F("AT+CREG?")))                       // Тип регистрации сети
		{
			// Первый параметр:
			// 0 – нет кода регистрации сети
			// 1 – есть код регистрации сети
			// 2 – есть код регистрации сети + доп параметры
			// Второй параметр:
			// 0 – не зарегистрирован, поиска сети нет
			// 1 – зарегистрирован, домашняя сеть
			// 2 – не зарегистрирован, идёт поиск новой сети
			// 3 – регистрация отклонена
			// 4 – неизвестно
			// 5 – роуминг
		
			char *p = strstr(buffer, "0,");                        // Получить второй параметр
			if (p)
			{
				char mode = *(p + 2);
#if DEBUG
				DEBUG.print("Mode:");
				DEBUG.println(mode);
#endif
				return mode;
			}
		}
		delay(1000);
	}
}


bool SAMD21_SIM800C::getIMEI()
{
  if (sendCommandS(F("AT+GSN")) == 1)               // (sendCommand("AT+GSN", "OK\r", "ERROR\r") == 1) 
  {
	char *p = strstr(buffer, "\r");                 //Функция strstr() возвращает указатель на первое вхождение в строку, 
											        //на которую указывает str1, строки, указанной str2 (исключая завершающий нулевой символ).
											        //Если совпадений не обнаружено, возвращается NULL.
	  if (p) 
	  {
		p += 2;
		 char *s = strchr(p, '\r');                 // Функция strchr() возвращает указатель на первое вхождение символа ch в строку, 
											        //на которую указывает str. Если символ ch не найден,
											        //возвращается NULL. 
		 if (s) *s = 0;   strcpy(buffer, p);
		 return true;
	  }
  }
  return false;
}
bool SAMD21_SIM800C::getSIMCCID()
{
	if (sendCommandS(F("AT+CCID")) == 1)             // (sendCommand("AT+CCID", "OK\r", "ERROR\r") == 1)
	{      
		char *p = strstr(buffer, "\r");              //Функция strstr() возвращает указатель на первое вхождение в строку, 
												     //Если совпадений не обнаружено, возвращается NULL.
		if (p)
		{
			p += 2;
			char *s = strchr(p, '\r');          // Функция strchr() возвращает указатель на первое вхождение символа ch в строку, 
												//на которую указывает str. Если символ ch не найден,
												//возвращается NULL. 
			if (s) *s = 0;   strcpy(buffer, p);
			int i=0;

			for (i=0;i<120;i++)
			{
				if (buffer[i] == 'f')  break;
				if (buffer[i] >= '0'&&buffer[i] <= '9') // если цифру найдено - то выводим ее
				{
					buffer1[i] = buffer[i];
					//Serial.print(buffer1[i]);
				}
			}
			return true;
		}
	}
	return false;
}


bool SAMD21_SIM800C::getOperatorName()
{
  // display operator name
  if (sendCommandS(F("AT+COPS?")) == 1)   
  {
	  char *p = strstr(buffer, ",\"");
	  if (p)
	  {
		  p += 2;
		  char *s = strchr(p, '\"');
		  if (s) *s = 0;
		  strcpy(buffer, p);

		  String OperatorName = buffer;
		  if (OperatorName.indexOf(F("MTS")) > -1)
		  {
			  apn  = F("internet.mts.ru");
			  user = F("mts");
			  pwd  = F("mts");
			  cont = F("internet.mts.ru");
			  Serial.println(F("MTS"));
			  operator_Num = 0;                                  // Порядковый номер оператора МТС
		  }
		  else if (OperatorName.indexOf(F("Bee")) > -1)
		  {
			  apn  = F("internet.beeline.ru");
			  user = F("beeline");
			  pwd  = F("beeline");
			  cont = F("internet.beeline.ru");
			  Serial.println(F("Beeline"));
			  operator_Num = 1;                                  // Порядковый номер оператора Beeline
		  }
		  else if (OperatorName.indexOf(F("Mega")) > -1)
		  {
			  apn  = F("internet");
			  user = "";
			  pwd  = "";
			  cont = F("internet");
			  Serial.println(F("MEGAFON"));
			  operator_Num = 2;                                  // Порядковый номер оператора Megafon
		  }
		  Serial.println(apn);
		  return true;
	  }
  }
  return false;
}


byte SAMD21_SIM800C::ping_connect_internet()
{
	int count_connect = 0;                                                // Счетчик количества попыток проверки подключения Network registration (сетевому оператору)
	for (;;)                                                              // Бесконечный цикл пока не наступит, какое то состояние для выхода
	{
		if (sendCommandS(F("AT+CREG?")) == 1) break;                      // Все нормально, в сети оператора зарегистрированы , Прервать попытки и выйти из цикла
		Serial.print(">");
		Serial.println(buffer);                                           // Не получилось, ("ERROR") 
		String stringError = buffer;
		if (stringError.indexOf(F("ERROR")) > -1)
		{
			Serial.println(F("\nNo GPRS connection"));
			delay(1000);
		}
		delay(1000);                                                     // Подождать секунду.
		count_connect++;
		if (count_connect > 60)
		{
			reboot(errors); //break;                                   //вызываем reset при отсутствии доступа к сетевому оператору в течении 60 секунд
		}
	}
	delay(1000);
	count_connect = 0;                                                 // Счетчик количества попыток проверки подключения Attach from GPRS service
	for (;;)                                                           // Бесконечный цикл пока не наступит, какое то состояние для выхода
	{
		if (sendCommandS(F("AT+CGATT?")) == 1) break;                  // Все нормально, модуль подключен к GPRS service , Прервать попытки и выйти из цикла
		Serial.print(F(">"));
		Serial.println(buffer);                                          // Не получилось, ("ERROR") 
		String stringError = buffer;
		if (stringError.indexOf(F("ERROR")) > -1)
		{
			Serial.println(F("\nNo GPRS connection"));
			delay(1000);
		}
		delay(1000);                                                     // Подождать секунду.
		count_connect++;
		if (count_connect > 60)
		{
			reboot(errors);                               //вызываем reset при отсутствии доступа к  GPRS service в течении 60 секунд
		}
	}

	//++++++++++++++++ Проверки пройдены, подключаемся к интернету по протоколу TCP для проверки ping ++++++++++++

	switch (operator_Num)                                                  // Определяем вариант подключения в зависимости от оператора
	{
	case 0:
		sendCommandS(F("AT+CSTT=\"internet.mts.ru\""));                    //Настроить точку доступа MTS. При повторных пингах будет выдавать ошибку. Это нормально потому что данные уже внесены.
		break;
	case 1:
		sendCommandS(F("AT+CSTT=\"internet.beeline.ru\""));                //Настроить точку доступа  beeline
		break;
	case 2:
		sendCommandS(F("AT+CSTT=\"internet\""));                           //Настроить точку доступа Megafon
		break;
	}
	delay(1000);
	timeout = 10000;
	sendCommandS(F("AT+CIICR"));                                           // Поднимаем протокол Bring Up Wireless Connection with GPRS  
	timeout = 2000;
	return 0;
}


bool SAMD21_SIM800C::ping(const char* url)
{
	sendCommandS(F("AT+CIFSR"));                                          //Получить локальный IP-адрес
	delay(1000);
	SIM800CHwSerial->print(F("AT+CIPPING=\""));                                // Отправить команду ping
	SIM800CHwSerial->print(url);
	SIM800CHwSerial->println('\"');
	delay(5000);

	//++++++++++++++++++++++++++ Ожидаем ответ сайта на ping  ++++++++++++++++++++++++++++++++++++++++   
	 expected1 = "+CIPPING";
	 expected2 = ERROR_r;
	 timeout   = 10000;
	if (sendCommandS(no) == 1)            // Ответ на ping получен 
	{
		SIM800CHwSerial->print(F("AT+CIPSHUT"));                           // Закрыть соединение
		expected1 = OK_r;
		expected2 = ERROR_r;
		timeout = 2000;
		return true;
	}
	expected1 = OK_r;
	expected2 = ERROR_r;
	timeout = 2000;
	SIM800CHwSerial->print(F("AT+CIPSHUT"));                             // Ошибка, что то не так пошло. На всякий случай закрываем соединение
	return false;
}

bool SAMD21_SIM800C::checkSMS()
{
	expected1 = "+CMGR:";
	expected2 = ERROR_r;

	if (sendCommandS(F("AT+CMGR=1")) == 1)                            //  отправляет команду "AT+CMGR=1", поиск ответного сообщения +CMGR:
	{ 
		while (SIM800CHwSerial->available())                                //есть данные от GSM модуля
		{
			ch = SIM800CHwSerial->read();
			val += char(ch);                                           //сохраняем входную строку в переменную val
			delay(10);
		}
		expected1 = OK_r;
		expected2 = ERROR_r;
		return true;
	}
	 expected1 = OK_r;
	 expected2 = ERROR_r;
	 return false; 
}

void SAMD21_SIM800C::send_sms(String text, String phone)  //процедура отправки СМС
{
	Serial.println(F("SMS send started"));
	SIM800CHwSerial->print("AT+CMGS=\"");                                // Отправить SMS
	delay(100);
	SIM800CHwSerial->print(phone);                                // Отправить SMS
	delay(100);
	SIM800CHwSerial->print("\"\r\n");                                // Отправить SMS
	delay(1000);
	SIM800CHwSerial->print(text);
	delay(100);
	SIM800CHwSerial->println((char)26);
	delay(100);
	Serial.println(phone);
	Serial.println(F("SMS send complete"));
	delay(2000);
}


bool SAMD21_SIM800C::deleteSMS(int n_sms)
{

	if (n_sms > 0)
	{
		if(sendCommandS(F("AT+CMGD=1")) == 1)                      //  remove the SMS
		return true;
	}
	else
	{
		if(sendCommandS(F("AT+CMGDA=\"DEL ALL\"")) == 1)          // remove the SMS
		return true;
	}
	return false;
}

byte SAMD21_SIM800C::getSignalQuality()
{
  sendCommandS(F("AT+CSQ"));                                      // Уровень сигнала
  char *p = strstr(buffer, "CSQ:");
  if (p) 
  {
	int n = atoi(p+5);
	if (n == 99 || n == -1) return 0;
	return n ;
  } else 
  {
   return 0; 
  }
}

void SAMD21_SIM800C::httpUninit()
{
  sendCommandS(F("AT+HTTPTERM"));          
}

bool SAMD21_SIM800C::httpInit()
{
	timeout = 10000;
	if  (!sendCommandS(F("AT+HTTPINIT")) || !sendCommandS(F("AT+HTTPPARA=\"CID\",1")))  //if  (!sendCommand("AT+HTTPINIT", 10000) || !sendCommand("AT+HTTPPARA=\"CID\",1", 5000)) 
	{
		httpState = HTTP_DISABLED;
		timeout = 2000;
		return false;
	}
	httpState = HTTP_READY;
	timeout = 2000;
	return true;
}

bool SAMD21_SIM800C::httpConnect(const char* url, const char* args)
{
	SIM800CHwSerial->print(F("AT+HTTPPARA=\"URL\",\""));                    
	SIM800CHwSerial->print(url);
	if (args) 
	{
		SIM800CHwSerial->print('?');
		SIM800CHwSerial->print(args);
	}

	SIM800CHwSerial->println('\"');
	if (sendCommandS(no))
	{
		// Starts GET action
		SIM800CHwSerial->println(F("AT+HTTPACTION=0"));                         
		httpState = HTTP_CONNECTING;
		m_bytesRecv = 0;
		m_checkTimer = millis();
	}
	else 
	{
		httpState = HTTP_ERROR;
	}
	return false;
}

bool SAMD21_SIM800C::httpConnectStr(const char* url, String args)
{
	SIM800CHwSerial->print(F("AT+HTTPPARA=\"URL\",\""));   
	SIM800CHwSerial->print(url);
	if (args) 
	{
		SIM800CHwSerial->print('?');
		SIM800CHwSerial->print(args);
	}

	SIM800CHwSerial->println('\"');
	delay(500);
	if (sendCommandS(no))
	{
		SIM800CHwSerial->println(F("AT+HTTPACTION=0"));              
		httpState = HTTP_CONNECTING;
		m_bytesRecv = 0;
		m_checkTimer = millis();
	}
	else 
	{
		httpState = HTTP_ERROR;
	}
	return false;
}

// check if HTTP connection is established
// return 0 for in progress, 1 for success, 2 for error
// Проверить, если соединение HTTP установлено
// Возвращает 0 - в работе, 1 для успешно, 2 для ошибки

byte SAMD21_SIM800C::httpIsConnected()
{
	byte ret = checkbuffer("0,200", "0,60", 15000);           
	if (ret >= 2) 
	{
		httpState = HTTP_ERROR;
		return -1;
	}
	return ret;
}

void SAMD21_SIM800C::httpRead()
{
	SIM800CHwSerial->println(F("AT+HTTPREAD"));     
	httpState = HTTP_READING;
	m_bytesRecv = 0;
	m_checkTimer = millis();
}
// check if HTTP connection is established
// return 0 for in progress, -1 for error, number of http payload bytes on success
// Проверить, если соединение HTTP установлено
// Возвращает значение 0 для продолжается, -1 для ошибки, количество байтов полезной нагрузки HTTP на успех

int SAMD21_SIM800C::httpIsRead()
{
	byte ret = checkbuffer("+HTTPREAD: ", ERROR_r, 10000) == 1;
	if (ret == 1)                  // Ответ +HTTPREAD:
	{
		m_bytesRecv = 0;
		sendCommandS(no, 100, "\r\n");
		int bytes = atoi(buffer);
		sendCommandS(no);
		bytes = min(bytes, sizeof(buffer) - 1);
		buffer[bytes] = 0;
		return bytes;
	} else if (ret >= 2)           // Ответ "Error"
	{
		httpState = HTTP_ERROR;
		return -1;
	}
	return 0;  
}

boolean SAMD21_SIM800C::HTTP_ssl(boolean onoff) 
{
	timeout  = 4000;
	if (onoff)
	{
		if(sendCommandS(F("AT+HTTPSSL=1")) == 1)timeout  = 2000; return true;
	}
	else
	{
		if (sendCommandS(F("AT+HTTPSSL=0")) == 1) timeout  = 2000; return true;
	}
	timeout  = 2000; 
	return false;           
}


byte SAMD21_SIM800C::checkbuffer(const char* expected1, const char* expected2, unsigned int timeout)
{
	// Поиск в тексте, пришедшем из модуля текстов, указанных в expected1 и expected2, ожидание не дольше чем в timeout
	while (SIM800CHwSerial->available())                      // Ждем появления данных с модуля, читаем если поступают символы 
	{
		char c = SIM800CHwSerial->read();
		if (m_bytesRecv >= sizeof(buffer) - 1)           // При вызове подпрограммы m_bytesRecv сбрасывается в"0" (при применении http)
		{
													     // Если количество символов больше размера буфера - половина текста удаляется.
			m_bytesRecv = sizeof(buffer) / 2 - 1;        // buffer full, discard first half буфер заполнен, выбросьте первую половину
			memcpy(buffer, buffer + sizeof(buffer) / 2, m_bytesRecv);  // Скопировать оставшуюся половину в buffer
		}
		buffer[m_bytesRecv++] = c;                        // Записать символ в буфер на место, указанное в m_bytesRecv
		buffer[m_bytesRecv] = 0;                          // Последним в буфере записать "0"
		if (strstr(buffer, expected1))                    // Найдено первое слово  return 1;
		{
			return 1;
		}
		if (expected2 && strstr(buffer, expected2))       // Если текст в буфере равен expected2 return 2;
		{
			return 2;
		}
	}
	return (millis() - m_checkTimer < timeout) ? 0 : 3;   // Время ожидания задано в m_checkTimer используется при применении http
														  // Два варианта окончания подпрограммы 0 - уложились вовремя или 3 время вышло при неуспешном
}


byte SAMD21_SIM800C::sendCommandS(String cmd, unsigned int timeout, const char* expected)
{   
	if (cmd!= "no")                                              // Если есть команда - отправить.
	{
		purgeSerial();                                           // Очистить приемный буффер
#ifdef DEBUG
		DEBUG.print('>');
		DEBUG.println(cmd);
#endif
		SIM800CHwSerial->println(cmd);                                // Отправить команду в модем
	}
	uint32_t t = millis();                                       // Записать текущее время в начале чтения ответа
	byte n = 0;
	do {                                                         // Читает ответ с модема
		if (SIM800CHwSerial->available())
		{
			char c = SIM800CHwSerial->read();
			if (n >= sizeof(buffer) - 1)                        // Если буффер переполнен - удалить первую часть  
			{ 
				// buffer full, discard first half
				n = sizeof(buffer) / 2 - 1;                      // Буфер заполнен, выбросьте первую половину
				memcpy(buffer, buffer + sizeof(buffer) / 2, n);  // Переместить вторую половину сообщения
			}
			buffer[n++] = c;                                   // Записать символ  в буфер и увеличить счетчик на 1                                    
			buffer[n] = 0;                                     // Записать 0 в конец строки
			if (strstr(buffer, expected ? expected : "OK\r"))   // возвращает указатель на первое вхождение в строку,
																// на которую указывает buffer, строки, указанной expected (исключая завершающий нулевой символ). 
																// Если совпадений не обнаружено, возвращается NULL.
			{                                                  // Переместит указатель на текст expected или "OK\r".
#ifdef DEBUG                                             
				DEBUG.print("[1]");
				DEBUG.println(buffer);                            // в буфере сообщение после отсечки указателя содержимого в expected
#endif
				return n;                                          // Позиция текущего указателя , Контрольная строка обнаружена 
			}
		}
	} while (millis() - t < timeout);                      // Считывать сообщение не более timeout миллисекунд.
#ifdef DEBUG
	DEBUG.print("[0]");
	DEBUG.println(buffer);
#endif
	return 0;                                              // Контрольная строка не обнаружена 
}
byte SAMD21_SIM800C::sendCommandS(String cmd)
{     
	if (cmd != "no")                                      // Отправить команду и ожидать ответ при совпадении слов в буфере по строкам expected1 или expected2 в течении timeout
	{
		purgeSerial();                                    // Очистить приемный буффер
#ifdef DEBUG
		DEBUG.print('>');
		DEBUG.println(cmd);
#endif
		SIM800CHwSerial->println(cmd);                         // Отправить команду
	}
	uint32_t t = millis();                                // Записать время старта
	byte n = 0;                                           // Сбросить счетчик символов 
	do {
		if (SIM800CHwSerial->available())                      // Если буфер не пустой - читать сообщения от модуля
		{
			char c = SIM800CHwSerial->read();                  // Читать сообщения от модуля
			if (n >= sizeof(buffer) - 1)                  // При переполнении буфера - урезать в 2 раза
			{
				n = sizeof(buffer) / 2 - 1;
				memcpy(buffer, buffer + sizeof(buffer) / 2, n);
			}
			buffer[n++] = c;
			buffer[n] = 0;
			if (strstr(buffer, expected1))                // Искать по строке  expected1, указатель перемещен
			{
#ifdef DEBUG
				DEBUG.print("[1]");
				DEBUG.println(buffer);
#endif
				return 1;
			}
			if (strstr(buffer, expected2))                 // Искать по строке  expected2, указатель перемещен
			{
#ifdef DEBUG
				DEBUG.print("[2]");
				DEBUG.println(buffer);
#endif
				return 2;
			}
		}
	} while (millis() - t < timeout);
#if DEBUG
	DEBUG.print("[0]");
	DEBUG.println(buffer);
#endif
	return 0;                                            // Строка expected1 или expected2 не найдена.
}

void SAMD21_SIM800C::purgeSerial()                          // Очистить приемный буффер
{
   while (SIM800CHwSerial->available()) SIM800CHwSerial->read();
}
bool SAMD21_SIM800C::available()
{
	return SIM800CHwSerial->available(); 
}
