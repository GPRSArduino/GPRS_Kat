/*************************************************************************
* SIM800 GPRS/HTTP Library
* Distributed under GPL v2.0
* Written by Stanley Huang <stanleyhuangyc@gmail.com>
* For more information, please visit http://arduinodev.com
*************************************************************************/

#include "SIM800.h"
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>


bool CGPRS_SIM800::begin(Stream &port)
{
	SIM_SERIAL = &port;
	int16_t timeout = 7000;

	while (timeout > 0)
	{
		while (SIM_SERIAL->available()) SIM_SERIAL->read();
		if (sendCommandS(F("AT")))
		{
			break;
		}
		delay(500);
		timeout -= 500;
	}

	if (timeout <= 0)
	{

		sendCommandS(F("AT"));   
		delay(100);
		sendCommandS(F("AT"));              
		delay(100);
		sendCommandS(F("AT"));               
		delay(100);
	}
	
	if (sendCommandS(F("AT")))               
	{
		sendCommandS(F("AT+IPR=19200"));                	 // Установить скорость обмена
		delay(100);
		sendCommandS(F("ATE0"));                             // Отключить эхо 
		delay(100);
		sendCommandS(F("AT+CFUN=1"));                        // 1 – нормальный режим (по умолчанию). Второй параметр 1 – перезагрузить (доступно только в нормальном режиме, т.е. параметры = 1,1)
		delay(100);																
		sendCommandS(F("AT+CMGF=1"));                        // режим кодировки СМС - обычный (для англ.)
		delay(100);														
		sendCommandS(F("AT+CLIP=1"));                        // включаем АОН
		delay(100);													
		sendCommandS(F("AT+CSCS=\"GSM\""));                  // режим кодировки текста
		delay(100);															
		//sendCommandS(F("AT+CNMI=2,2"));                    // отображение смс в терминале сразу после приема (без этого сообщения молча падают в память)tln("AT+CSCS=\"GSM\""); 
		delay(100);									 
		//sendCommandS(F("AT+CMGDA=\"DEL ALL\""));           // AT+CMGDA=«DEL ALL» команда удалит все сообщения
		//delay(100);
		//sendCommandS(F("AT+GMR"));                         // Номер прошивки
		//delay(100);
		sendCommandS(F("AT + CMGF = 1"));                    // sets the SMS mode to text
		//sendCommandS(F("AT+CPMS=\"SM\",\"SM\",\"SM\""));   // selects the memory
		return true;
	}
	return false;
}

byte CGPRS_SIM800::setup()
{
	                                                               // Неуспешная регистрация
}

byte CGPRS_SIM800::connect_GPRS()
{
	for (byte n = 0; n < 30; n++)
	{
		if (!sendCommandS(F("AT+SAPBR=3,1,\"Contype\",\"GPRS\""))) return 1;           

		SIM_SERIAL->print(F("AT+SAPBR=3,1,\"APN\",\""));
		SIM_SERIAL->print(apn);
		SIM_SERIAL->println('\"');
		if (!sendCommand(0))   return 2;

		SIM_SERIAL->print(F("AT+SAPBR=3,1,\"USER\",\""));
		SIM_SERIAL->print(user);
		SIM_SERIAL->println('\"');
		if (!sendCommand(0))   return 2;

		SIM_SERIAL->print(F("AT+SAPBR=3,1,\"PWD\",\""));
		SIM_SERIAL->print(pwd);
		SIM_SERIAL->println('\"');
		if (!sendCommand(0))   return 2;

		SIM_SERIAL->print(F("AT+CGDCONT=1,\"IP\",\""));
		SIM_SERIAL->print(cont);
		SIM_SERIAL->println('\"');
		if (!sendCommand(0))   return 2;

		timeout = 10000;
		sendCommandS(F("AT+SAPBR=1,1"));  timeout = 2000;   return 0;                 // установка GPRS связи
	}
	return 3;                                                                         // Неуспешная регистрация
}

bool CGPRS_SIM800::connect_IP_GPRS()
{
	for (byte n = 0; n < 30; n++)
	{
		if (sendCommandS(F("AT+SAPBR=2,1"))) return true;                             // получить IP адрес
	}
	return false;
}

void  CGPRS_SIM800::close_GPRS()
{

	sendCommand("AT + CIPSHUT");  // закрываем GPRS-сессию у оператора

}




void CGPRS_SIM800::cleanStr(String & str) 
{
  str.replace("OK", "");
  str.replace("\"", "");
  str.replace("\n", "");
  str.replace("\r", "");
  str.trim();
}

uint8_t CGPRS_SIM800::getNetworkStatus()
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


bool CGPRS_SIM800::getIMEI()
{
  if (sendCommandS(F("AT+GSN")) == 1)               // (sendCommand("AT+GSN", "OK\r", "ERROR\r") == 1) 
  {
	char *p = strstr(buffer, "\r");          //Функция strstr() возвращает указатель на первое вхождение в строку, 
											 //на которую указывает str1, строки, указанной str2 (исключая завершающий нулевой символ).
											 //Если совпадений не обнаружено, возвращается NULL.
	  if (p) 
	  {
		p += 2;
		
		 // char *s = strstr(buffer, "OK");  // Ищем завершения операции
		 char *s = strchr(p, '\r');       // Функция strchr() возвращает указатель на первое вхождение символа ch в строку, 
											//на которую указывает str. Если символ ch не найден,
											//возвращается NULL. 
		 if (s) *s = 0;   strcpy(buffer, p);
		 return true;
	  }
  }
  return false;
}
bool CGPRS_SIM800::getSIMCCID()
{
	if (sendCommandS(F("AT+CCID")) == 1)             // (sendCommand("AT+CCID", "OK\r", "ERROR\r") == 1)
	{      
		char *p = strstr(buffer, "\r");          //Функция strstr() возвращает указатель на первое вхождение в строку, 
               								     //Если совпадений не обнаружено, возвращается NULL.
		if (p)
		{
			p += 2;
			char *s = strchr(p, '\r');          // Функция strchr() возвращает указатель на первое вхождение символа ch в строку, 
											    //на которую указывает str. Если символ ch не найден,
											    //возвращается NULL. 
			if (s) *s = 0;   strcpy(buffer, p);
			return true;
		}
	}
	return false;
}


bool CGPRS_SIM800::getOperatorName()
{
  // display operator name
  if (sendCommandS(F("AT+COPS?")) == 1)   // if (sendCommand("AT+COPS?", "OK\r", "ERROR\r") == 1) 
  {
	  char *p = strstr(buffer, ",\"");
	  if (p)
	  {
		  p += 2;
		  char *s = strchr(p, '\"');
		  if (s) *s = 0;
		  strcpy(buffer, p);

		  String OperatorName = buffer;
		  Serial.println(buffer);

		  if (OperatorName.indexOf(F("MTS")) > -1)
		  {
			  strcpy_P(buffer, (char*)pgm_read_word(&(table_message[4])));
			  apn = buffer;
			  user = "mts";
			  pwd = "mts";
			  cont = buffer;
			  Serial.println(F("MTS"));
			  operator_Num = 0;                                  // Порядковый номер оператора МТС
		  }
		  else if (OperatorName.indexOf(F("Bee")) > -1)
		  {
			  strcpy_P(buffer, (char*)pgm_read_word(&(table_message[6])));
			  apn = buffer;
			  user = "beeline";
			  pwd = "beeline";
			  cont = buffer;
			  Serial.println(F("Beeline"));
			  operator_Num = 1;                                  // Порядковый номер оператора Beeline
		  }
		  else if (OperatorName.indexOf(F("Mega")) > -1)
		  {
			  strcpy_P(buffer, (char*)pgm_read_word(&(table_message[8])));
			  apn = buffer;
			  user = "";
			  pwd = "";
			  cont = buffer;
			  Serial.println(F("MEGAFON"));
			  operator_Num = 2;                                  // Порядковый номер оператора Megafon
		  }
		  return true;
	  }
  }
  return false;
}

bool CGPRS_SIM800::ping(const char* url)
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[9])));     //sendCommand("AT+CGATT?", 1000);  Проверить подключение к сервису GPRS
	sendCommand(bufcom, 1000);                                       // Переделать, добавить проверку подключения к интернету
	delay(100);

	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[7])));     //"AT+CREG?"  проверим регистрацию сети
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[33])));    // "ERROR\r"
	if (sendCommand(bufcom, "OK\r", combuf1) == 1)                   // в сети зарегистрированы
	{
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[9])));     //"AT+CGATT?"  проверим GPRS аттач
		//strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[33])));    // "ERROR\r"

		if (sendCommand(bufcom, "OK\r", combuf1) == 1)                   // Attach or Detach from GPRS Service
		{

			//strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[53])));
			//sendCommand(bufcom, 3000);                                       //sendCommand("AT+CIFSR", 3000);   Получить локальный IP-адрес
			//delay(1000);

			strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[54])));    // SIM_SERIAL->print("AT+CIPPING=\"");
			SIM_SERIAL->print(bufcom);                                       // SIM_SERIAL->print("AT+CIPPING=\"");
			SIM_SERIAL->print(url);
			SIM_SERIAL->println('\"');

			strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[55])));
			strcpy_P(combuf1, (char*)pgm_read_word(&(table_message[50])));
			delay(10000);

			// Ожидаем ответ сайта на ping 
			if (sendCommand(0, bufcom, combuf1, 6000) == 1) // (sendCommand(0, "+CIPPING", "ERROR",3000) == 1)
			{
				return true;
			}

			//sendCommand(bufcom);                             // sendCommand("AT+CSQ");



		}
	/*	else
		{



		}*/

	}








	//strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[51])));
	//sendCommand(bufcom, 1000);      //
	//sendCommand("AT+CSTT=\"internet.mts.ru\"", 1000);//Настроить точку доступа ????
	/*SIM_SERIAL->print("AT+CSTT=\"");
	SIM_SERIAL->print(apn);
	SIM_SERIAL->print('\"');*/
	//delay(1000);
	//strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[52])));
	//sendCommand(bufcom, 1000);                  // sendCommand("AT+CIICR", 1000); Установить GPRS-соединение   ????   
	//delay(1000);
	//strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[53])));
	//sendCommand(bufcom, 3000);                  //sendCommand("AT+CIFSR", 3000);   Получить локальный IP-адрес
	//delay(1000);
	//strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[54])));
	//SIM_SERIAL->print(bufcom);              // SIM_SERIAL->print("AT+CIPPING=\"");
	//SIM_SERIAL->print(url);
	//SIM_SERIAL->println('\"');
	//strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[55])));
	//strcpy_P(combuf1, (char*)pgm_read_word(&(table_message[50])));
	//delay(10000);

	//// Ожидаем ответ сайта на ping 
	//if (sendCommand(0, bufcom, combuf1,6000) == 1) // (sendCommand(0, "+CIPPING", "ERROR",3000) == 1)
	//{
	//	return true;
	//}
	return false;
}

bool CGPRS_SIM800::checkSMS()
{
	// SMS: "REC UNREAD", "+79162632701", "", "17/01/21,13:51:06+12"
	//	 Timeset5
	//	 "REC READ", "+79162632701", "", "17/01/21,13:51:06+12"
	//	 Timeset5
	//	 + CMGR:
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[56])));
	strcpy_P(combuf1, (char*)pgm_read_word(&(table_message[50])));
 if (sendCommand(bufcom, "+CMGR:", combuf1) == 1)   //(sendCommand("AT+CMGR=1", "+CMGR:", "ERROR") == 1)  отправляет команду "AT+CMGR=1", поиск ответного сообщения +CMGR:
  { 
	 while (SIM_SERIAL->available())       //есть данные от GSM модуля
	 {
		 ch = SIM_SERIAL->read();
		 val += char(ch);                   //сохраняем входную строку в переменную val
		 delay(10);
	 }
	 return true;
  }
  return false; 
}

bool CGPRS_SIM800::deleteSMS(int n_sms)
{

	if (n_sms > 0)
	{
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[57])));
		if(sendCommand("AT+CMGD=1", "OK\r", "ERROR\r") == 1)           //sendCommand(bufcom);                // sendCommand("AT+CMGD=1");  remove the SMS
		return true;
	}
	else
	{
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[58])));	     //sendCommand("AT+CMGDA=\"DEL ALL\""); remove the SMS
		if(sendCommand("AT+CMGDA=\"DEL ALL\"", "OK\r", "ERROR\r") == 1)
		return true;

	}
	return false;
}

int CGPRS_SIM800::getSignalQuality()
{
  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[8])));
  sendCommand(bufcom);                             // sendCommand("AT+CSQ");
  char *p = strstr(buffer, "CSQ:");
  if (p) {
	int n = atoi(p+5);
	if (n == 99 || n == -1) return 0;
	return n ;
  } else {
   return 0; 
  }
}

void CGPRS_SIM800::httpUninit()
{
  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[35])));
  sendCommand(bufcom);          // sendCommand("AT+HTTPTERM");
}

bool CGPRS_SIM800::httpInit()
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[36])));
	strcpy_P(combuf1, (char*)pgm_read_word(&(table_message[37])));
	if  (!sendCommand(bufcom, 10000) || !sendCommand(combuf1, 5000))  //if  (!sendCommand("AT+HTTPINIT", 10000) || !sendCommand("AT+HTTPPARA=\"CID\",1", 5000)) 
	{
		httpState = HTTP_DISABLED;
		return false;
	}
	httpState = HTTP_READY;

	return true;
}

bool CGPRS_SIM800::httpConnect(const char* url, const char* args)
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[38])));
	SIM_SERIAL->print(bufcom);                    //SIM_SERIAL->print("AT+HTTPPARA=\"URL\",\"");
	SIM_SERIAL->print(url);
	if (args) 
	{
		SIM_SERIAL->print('?');
		SIM_SERIAL->print(args);
	}

	SIM_SERIAL->println('\"');
	if (sendCommand(0))
	{
		// Starts GET action
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[39])));
		SIM_SERIAL->println(bufcom);                         //SIM_SERIAL->println("AT+HTTPACTION=0");
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

bool CGPRS_SIM800::httpConnectStr(const char* url, String args)
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[40])));
	SIM_SERIAL->print(bufcom);    //SIM_SERIAL->print("AT+HTTPPARA=\"URL\",\"");
	SIM_SERIAL->print(url);
	if (args) 
	{
		SIM_SERIAL->print('?');
		SIM_SERIAL->print(args);
	}

	SIM_SERIAL->println('\"');
	delay(500);
	if (sendCommand(0))
	{
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[41])));
		SIM_SERIAL->println(bufcom);              //SIM_SERIAL->println("AT+HTTPACTION=0");
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
// Возвращает 0 для прогресса в, 1 для успеха, 2 для ошибки

byte CGPRS_SIM800::httpIsConnected()
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[42])));
	strcpy_P(combuf1, (char*)pgm_read_word(&(table_message[43])));
	byte ret = checkbuffer(bufcom,combuf1, 10000);           // byte ret = checkbuffer("0,200", "0,60", 10000);
	if (ret >= 2) 
	{
		httpState = HTTP_ERROR;
		return -1;
	}
	return ret;
}

void CGPRS_SIM800::httpRead()
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[44])));
	SIM_SERIAL->println(bufcom);     //SIM_SERIAL->println("AT+HTTPREAD");
	httpState = HTTP_READING;
	m_bytesRecv = 0;
	m_checkTimer = millis();
}
// check if HTTP connection is established
// return 0 for in progress, -1 for error, number of http payload bytes on success
// Проверить, если соединение HTTP установлено
// Возвращает значение 0 для продолжается, -1 для ошибки, количество байтов полезной нагрузки HTTP на успех

int CGPRS_SIM800::httpIsRead()
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[45])));
	strcpy_P(combuf1, (char*)pgm_read_word(&(table_message[46])));
	byte ret = checkbuffer(bufcom, combuf1, 10000) == 1;//byte ret = checkbuffer("+HTTPREAD: ", "Error", 10000) == 1;
	if (ret == 1)       // Ответ +HTTPREAD:
	{
		m_bytesRecv = 0;
		// read the rest data
		sendCommand(0, 100, "\r\n");
		int bytes = atoi(buffer);
		sendCommand(0);
		bytes = min(bytes, sizeof(buffer) - 1);
		buffer[bytes] = 0;
		return bytes;
	} else if (ret >= 2)   // Ответ "Error"
	{
		httpState = HTTP_ERROR;
		return -1;
	}
	return 0;  
}

boolean CGPRS_SIM800::HTTP_ssl(boolean onoff) 
{
	if (onoff)
	{
		if(sendCommand("AT+HTTPSSL=1", "OK\r", "ERROR\r") == 1) return true;
	}
	else
	{
		if (sendCommand("AT+HTTPSSL=0", "OK\r", "ERROR\r") == 1) return true;
	}
	return false;           
}


byte CGPRS_SIM800::sendCommand(const char* cmd, unsigned int timeout, const char* expected)
{      // синтаксис - команда, 
  if (cmd)                                 // Если есть команда - отправить.
  {
	purgeSerial();                         // Очистить приемный буффер
#ifdef DEBUG
	DEBUG.print('>');
	DEBUG.println(cmd);
#endif
	SIM_SERIAL->println(cmd);                            // Отправить команду в модем
  }
  uint32_t t = millis();                                 // Записать текущее время в начале чтения ответа
  byte n = 0;
  do {                                                   // Читает ответ с модема
	if (SIM_SERIAL->available()) 
	{
	  char c = SIM_SERIAL->read();
	  if (n >= sizeof(buffer) - 1)                       // Если буффер переполнен - удалить первую часть  
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

byte CGPRS_SIM800::sendCommandS(String cmd, unsigned int timeout, const char* expected)
{      // синтаксис - команда, 
	if (cmd)                                 // Если есть команда - отправить.
	{
		purgeSerial();                         // Очистить приемный буффер
#ifdef DEBUG
		DEBUG.print('>');
		DEBUG.println(cmd);
#endif
		SIM_SERIAL->println(cmd);                            // Отправить команду в модем
	}
	uint32_t t = millis();                                 // Записать текущее время в начале чтения ответа
	byte n = 0;
	do {                                                   // Читает ответ с модема
		if (SIM_SERIAL->available())
		{
			char c = SIM_SERIAL->read();
			if (n >= sizeof(buffer) - 1)                       // Если буффер переполнен - удалить первую часть  
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





byte CGPRS_SIM800::sendCommand(const char* cmd, const char* expected1, const char* expected2, unsigned int timeout)
{        // Отправить команду и ожидать ответ при совпадении слов в буфере по строкам expected1 или expected2 в течении timeout
  if (cmd) 
  {
	purgeSerial();                     // Очистить приемный буффер
	#ifdef DEBUG
		DEBUG.print('>');
		DEBUG.println(cmd);
	#endif
	SIM_SERIAL->println(cmd);           // Отправить команду
  }
  uint32_t t = millis();                // Записать время старта
  byte n = 0;                           // Сбросить счетчик символов 
	do {
		if (SIM_SERIAL->available())    // Если буфер не пустой - читать сообщения от модуля
		{
		  char c = SIM_SERIAL->read();  // Читать сообщения от модуля
		  if (n >= sizeof(buffer) - 1)  // При переполнении буфера - урезать в 2 раза
		  {
			// buffer full, discard first half
			n = sizeof(buffer) / 2 - 1;
			memcpy(buffer, buffer + sizeof(buffer) / 2, n);
		  }
		  buffer[n++] = c;
		  buffer[n] = 0;
		  if (strstr(buffer, expected1))   // Искать по строке  expected1, указатель перемещен
		  {
			#ifdef DEBUG
				   DEBUG.print("[1]");
				   DEBUG.println(buffer);
			#endif
		   return 1;
		  }
		  if (strstr(buffer, expected2))  // Искать по строке  expected2, указатель перемещен
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
  return 0;                      // Строка expected1 или expected2 не найдена.
}




byte CGPRS_SIM800::sendCommandS(String cmd)
{        // Отправить команду и ожидать ответ при совпадении слов в буфере по строкам expected1 или expected2 в течении timeout

	//const char* expected1 = "OK\r";
	//const char* expected2 = "ERROR\r";
	//unsigned int timeout = 2000;

	if (cmd)
	{
		purgeSerial();                     // Очистить приемный буффер
#ifdef DEBUG
		DEBUG.print('>');
		DEBUG.println(cmd);
#endif
		SIM_SERIAL->println(cmd);           // Отправить команду
	}
	uint32_t t = millis();                // Записать время старта
	byte n = 0;                           // Сбросить счетчик символов 
	do {
		if (SIM_SERIAL->available())    // Если буфер не пустой - читать сообщения от модуля
		{
			char c = SIM_SERIAL->read();  // Читать сообщения от модуля
			if (n >= sizeof(buffer) - 1)  // При переполнении буфера - урезать в 2 раза
			{
				// buffer full, discard first half
				n = sizeof(buffer) / 2 - 1;
				memcpy(buffer, buffer + sizeof(buffer) / 2, n);
			}
			buffer[n++] = c;
			buffer[n] = 0;
			if (strstr(buffer, expected1))   // Искать по строке  expected1, указатель перемещен
			{
#ifdef DEBUG
				DEBUG.print("[1]");
				DEBUG.println(buffer);
#endif
				return 1;
			}
			if (strstr(buffer, expected2))  // Искать по строке  expected2, указатель перемещен
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
	return 0;                      // Строка expected1 или expected2 не найдена.
}




byte CGPRS_SIM800::checkbuffer(const char* expected1, const char* expected2, unsigned int timeout)
{
	// Поиск в тексте, пришедшем из модуля текстов, указанных в expected1 и expected2, ожидание не дольше чем в timeout
	while (SIM_SERIAL->available())                  // Ждем появления данных с модуля, читаем если поступают символы 
	{
		char c = SIM_SERIAL->read();
		if (m_bytesRecv >= sizeof(buffer) - 1)        // При вызове подпрограммы m_bytesRecv сбрасывается в"0" (при применении http)
		{
			                                          // Если количество символов больше размера буфера - половина текста удаляется.
			m_bytesRecv = sizeof(buffer) / 2 - 1;    // buffer full, discard first half буфер заполнен, выбросьте первую половину
			memcpy(buffer, buffer + sizeof(buffer) / 2, m_bytesRecv);  // Скопировать оставшуюся половину в buffer
		}
		buffer[m_bytesRecv++] = c;                   // Записать символ в буфер на место, указанное в m_bytesRecv
		buffer[m_bytesRecv] = 0;                     // Последним в буфере записать "0"
		if (strstr(buffer, expected1))               // Найдено первое слово  return 1;
		{
			return 1;
		}
		if (expected2 && strstr(buffer, expected2))  // Если текст в буфере равен expected2 return 2;
		{
			return 2;
		}
	}
	return (millis() - m_checkTimer < timeout) ? 0 : 3;   // Время ожидания задано в m_checkTimer используется при применении http
	                                                      // Два варианта окончания подпрограммы 0 - уложились вовремя или 3 время вышло при неуспешном
}

void CGPRS_SIM800::purgeSerial()    // Очистить приемный буффер
{
   while (SIM_SERIAL->available()) SIM_SERIAL->read();
}
bool CGPRS_SIM800::available()
{
	return SIM_SERIAL->available(); 
}
