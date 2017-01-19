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
		if (sendCommand("AT"))
		{
			break;
		}
		delay(500);
		timeout -= 500;
	}

	if (timeout <= 0)
	{
		sendCommand("AT");
		delay(100);
		sendCommand("AT");
		delay(100);
		sendCommand("AT");
		delay(100);
	}


	if (sendCommand("AT"))
	{
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[1])));
		sendCommand(bufcom);         	                                //	sendCommand("AT+IPR=19200");   // ���������� �������� ������
		delay(100);
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[0])));
		//sendCommand("ATE0");                                          // ��������� ��� 
		sendCommand(bufcom);
		delay(100);
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[2])));
		sendCommand(bufcom);                                            // 1 � ���������� ����� (�� ���������). ������ �������� 1
		delay(100);																//sendCommand("AT+CFUN=1");                                     // 1 � ���������� ����� (�� ���������). ������ �������� 1 � ������������� (�������� ������ � ���������� ������, �.�. ��������� = 1,1)
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[3])));
		sendCommand(bufcom);                                            // ����� ��������� ��� - ������� (��� ����.)
		delay(100);																//sendCommand("AT+CMGF=1");                                     // ����� ��������� ��� - ������� (��� ����.)
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[4])));
		sendCommand(bufcom);                                            // �������� ���
		delay(100);																//sendCommand("AT+CLIP=1");                                     // �������� ���
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[5])));
		sendCommand(bufcom);                                            // ����� ��������� ������
		delay(100);																//sendCommand("AT+CSCS=\"GSM\"");                               // ����� ��������� ������
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[6])));
		//sendCommand(bufcom);                                            // ����������� ��� � ��������� ����� ����� ������ (��� ����� 
		delay(100);															//sendCommand("AT+CNMI=2,2");                                   // ����������� ��� � ��������� ����� ����� ������ (��� ����� ��������� ����� ������ � ������)tln("AT+CSCS=\"GSM\""); 
		sendCommand("AT+CMGDA=\"DEL ALL\"");                            // AT+CMGDA=�DEL ALL� ������� ������ ��� ���������
		delay(100);
		//sendCommand("AT+CMGDA=\"DEL ALL\"");                            // AT+CMGDA=�DEL ALL� ������� ������ ��� ���������
		//delay(100);
		return true;
	}
	return false;
}


byte CGPRS_SIM800::setup()
{
  for (byte n = 0; n < 30; n++)
  {
	  sendCommand("AT+CIPSHUT",20000);
	
	 // if (!sendCommand("AT+CGATT=1")) return 2;    // ����������� � GPRS

	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[9])));
	  if (!sendCommand(bufcom)) return 2;   // if (!sendCommand("AT+CGATT?"))     // ����������� � GPRS

	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[10])));
	  if (!sendCommand(bufcom)) return 3;   // if (!sendCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\""))     return 3;// 
	 
	  getOperatorName();
	  String OperatorName = buffer;
	  cleanStr(OperatorName);
	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[12]))); //"MTS"
	  if (OperatorName.indexOf(bufcom) > -1)
	  {
		  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[11])));
		  apn = bufcom;
		  //apn  = "internet.mts.ru";
		  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[13])));
		  user = bufcom;
		  pwd = bufcom;
		  /*	user = "mts";
			  pwd  = "mts";*/
		  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[11])));
		  cont = bufcom;
		  //cont = "internet.mts.ru";
		  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[12]))); //"MTS"
		  Serial.println(bufcom);
	  }
	  else if (OperatorName.indexOf("Bee Line GSM") > -1)
	  {
		  strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[15])));
		  apn = bufcom1;                                                 //apn = "internet.beeline.ru";
		  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[16])));
		  user = bufcom;                                                 //user = "beeline";
		  pwd = bufcom;                                                  //pwd = "beeline";
		  cont = bufcom1;                                                //cont = "internet.beeline.ru";
		  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[14])));
		  Serial.println(bufcom);                                        //Serial.println("Beeline");
	  }
	  else if (OperatorName.indexOf("MegaFon") > -1)
	  {
		  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[17])));
		  strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[18])));
		  apn = bufcom1;	                                                   //apn = "internet";
		  user = "";
		  pwd = "";
		  cont = bufcom1;	                                                   //cont = "internet";
		  Serial.println(bufcom);                                            //Serial.println("MEGAFON");
	  }
	
	  //  ��������� ��� ����������:
	  //  ��� - APN internet.mts.ru ��� ������������ � ������ mts , ����� ������� *99#
	  //  ������� - APN internet ��� ������������ � ������ internet , ����� ������� *99#
	  //  ������ - APN internet.beeline.ru ��� ������������ � ������ beeline , ����� ������� *99# - ��� ��� ����� �� ��������
	  //  ������ - APN home.beeline.ru ��� ������������ � ������ beeline , ����� ������� *99# - ��� ����������� ��� ��� ������

	  //  AT+CGDCONT=1,"IP","home.beeline.ru" � ���������. 
	  //  ��� ��� �� �������� ������ AT+CGDCONT=1,"IP","internet.beeline.ru" 
	  //  ��� �������� AT+CGDCONT=1,"IP","internet"
	  //  ��� ��� AT+CGDCONT=1,"IP","internet.mts.ru"
	 
	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[22])));
	  SIM_SERIAL->print(bufcom);                                       //SIM_SERIAL->print("AT+SAPBR=3,1,\"APN\",\"");
	  SIM_SERIAL->print(apn);
	  SIM_SERIAL->println('\"');
	  if (!sendCommand(0))   return 4;

	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[23])));
	  SIM_SERIAL->print(bufcom);                                       //SIM_SERIAL->print("AT+SAPBR=3,1,\"USER\",\"");
	  SIM_SERIAL->print(user);
	  SIM_SERIAL->println('\"');
	  if (!sendCommand(0))   return 4;

	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[24])));
	  SIM_SERIAL->print(bufcom);                                       //SIM_SERIAL->print("AT+SAPBR=3,1,\"PWD\",\"");
	  SIM_SERIAL->print(pwd);
	  SIM_SERIAL->println('\"');
	  if (!sendCommand(0))   return 4;

	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[25])));
	  SIM_SERIAL->print(bufcom);                                      //SIM_SERIAL->print("AT+CGDCONT=1,\"IP\",\"");
	  SIM_SERIAL->print(cont);
	  SIM_SERIAL->println('\"');
	  if (!sendCommand(0))   return 4;

	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[26])));
	  sendCommand(bufcom, 10000);                                    //sendCommand("AT+SAPBR=1,1", 10000);                     // ��������� GPRS �����
	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[27])));
	  sendCommand(bufcom, 10000);                                    //sendCommand("AT+SAPBR=2,1", 10000);                     // ���������� IP �����

	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[28])));
	  sendCommand(bufcom);                                           //sendCommand("AT+CMGF=1");                               // sets the SMS mode to text
	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[29])));
	  sendCommand(bufcom);                                           //sendCommand("AT+CPMS=\"SM\",\"SM\",\"SM\"");            // selects the memory
	  return 0;                                                   // �������� �����������
  }
	return 5;                                                   // ���������� �����������
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
		strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[7])));
		if (sendCommand(bufcom, 2000))
		{
			// if (sendCommand("AT+CREG?", 2000))  // ��� ����������� ����
			// ������ ��������:
			// 0 � ��� ���� ����������� ����
			// 1 � ���� ��� ����������� ����
			// 2 � ���� ��� ����������� ���� + ��� ���������
			// ������ ��������:
			// 0 � �� ���������������, ������ ���� ���
			// 1 � ���������������, �������� ����
			// 2 � �� ���������������, ��� ����� ����� ����
			// 3 � ����������� ���������
			// 4 � ����������
			// 5 � �������
		
			char *p = strstr(buffer, "0,");    // �������� ������ ��������
			if (p)
			{
				char mode = *(p + 2);
#if DEBUG
				DEBUG.print("Mode:");
				DEBUG.println(mode);
#endif
								
				if (mode == '1' || mode == '5')
				{
					strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[8])));
					sendCommand(bufcom, 1000); 	//sendCommand("AT+CSQ",1000); 
					char *p = strstr(buffer, "CSQ: ");
					return mode;
				}
			}
		}
		delay(1000);
	}
}


bool CGPRS_SIM800::getIMEI()
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[30])));
	sendCommand(bufcom);                                          //sendCommand("AT+GSN");
	delay(1000);

  if (sendCommand("AT+GSN", "OK\r", "ERROR\r") == 1) 
  {
	char *p = strstr(buffer, "\r");          //������� strstr() ���������� ��������� �� ������ ��������� � ������, 
											 //�� ������� ��������� str1, ������, ��������� str2 (�������� ����������� ������� ������).
											 //���� ���������� �� ����������, ������������ NULL.
	  if (p) 
	  {
		p += 2;
		
		 // char *s = strstr(buffer, "OK");  // ���� ���������� ��������
		 char *s = strchr(p, '\r');       // ������� strchr() ���������� ��������� �� ������ ��������� ������� ch � ������, 
											//�� ������� ��������� str. ���� ������ ch �� ������,
											//������������ NULL. 
		 if (s) *s = 0;   strcpy(buffer, p);
		 return true;
	  }
  }
  return false;
}

bool CGPRS_SIM800::getOperatorName()
{
  // display operator name
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[31])));
	strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[33])));

  if (sendCommand(bufcom, "OK\r", bufcom1) == 1)   // if (sendCommand("AT+COPS?", "OK\r", "ERROR\r") == 1) 
  {
	  char *p = strstr(buffer, ",\"");
	  if (p) 
	  {
		  p += 2;
		  char *s = strchr(p, '\"');
		  if (s) *s = 0;
		  strcpy(buffer, p);
		  return true;
	  }
  }
  return false;
}

bool CGPRS_SIM800::ping(const char* url)
{
	sendCommand("AT+CGATT?", 1000);                 // ��������� ����������� � ������� GPRS
	delay(1000);
	sendCommand("AT+CSTT=\"internet\"", 1000);      // ��������� ����� ������� ????
	delay(1000);
	sendCommand("AT+CIICR", 1000);                  // ���������� GPRS-����������   ????   
	delay(1000);
	sendCommand("AT+CIFSR", 1000);                  // �������� ��������� IP-�����
	delay(1000);

	//if (sendCommand(url, "+CIPPING", "ERROR",3000) == 1)
	//{
	//	return true;
	//}
	//return false;
	//sendCommand(url, 3000);

	SIM_SERIAL->print("AT+CIPPING=\"");
	SIM_SERIAL->print(url);
	SIM_SERIAL->println('\"');

}


bool CGPRS_SIM800::ifSMSNow(void)
{
	return sim800_check_with_cmd("AT+CMGR=1", "+CMTI: ", CMD);
}

boolean CGPRS_SIM800::sim800_check_with_cmd(const char* cmd, const char *resp, DataType type, unsigned int timeout, unsigned int chartimeout)
{
	sim800_send_cmd(cmd);
	//sendCommand("AT+CMGR=1");
	return sim800_wait_for_resp(resp, type, timeout, chartimeout);
}

void CGPRS_SIM800::sim800_send_cmd(const char* cmd)
{
	for (int i = 0; i<strlen(cmd); i++)
	{
		sim800_send_byte(cmd[i]);
	}
}

void CGPRS_SIM800::sim800_send_byte(uint8_t data)
{
	SIM_SERIAL->write(data);
}


boolean CGPRS_SIM800::sim800_wait_for_resp(const char* resp, DataType type, unsigned int timeout, unsigned int chartimeout)
{
	int len = strlen(resp);
	int sum = 0;
	unsigned long timerStart, prevChar;    //prevChar is the time when the previous Char has been read.
	timerStart = millis();
	prevChar = 0;
	while (1) {
		if (SIM_SERIAL->available()) {
			char c = SIM_SERIAL->read();
			prevChar = millis();
			sum = (c == resp[sum]) ? sum + 1 : 0;
			if (sum == len)break;
		}
		if ((unsigned long)(millis() - timerStart) > timeout * 1000UL) {
			return false;
		}
		//If interchar Timeout => return FALSE. So we can return sooner from this function.
		if (((unsigned long)(millis() - prevChar) > chartimeout) && (prevChar != 0)) {
			return false;
		}

	}
	//If is a CMD, we will finish to read buffer.
	if (type == CMD) purgeSerial();
	return true;
}




/*
bool CGPRS_SIM800::checkSMS()
{
 if (sendCommand("AT+CMGR=1", "+CMGR:", "ERROR") == 1) 
  {
	 Serial.print("**SMS**  ");
	 Serial.println(buffer);

	sendCommand(0, 100, "\r\n");
	
	if (sendCommand(0)) {
	  // remove the SMS
	 // sendCommand("AT+CMGD=1");
	  return true;
	}
  }
  return false; 
}
*/

void CGPRS_SIM800::sim800_read_buffer(char *buffer1, int count, unsigned int timeout, unsigned int chartimeout)
{
	int i = 0;
	unsigned long timerStart, prevChar;
	timerStart = millis();
	prevChar = 0;
	while (1) 
	{
		while (SIM_SERIAL->available()) 
		{
			char c = SIM_SERIAL->read();
			prevChar = millis();
			buffer1[i++] = c;
			if (i >= count)break;
		}
		if (i >= count)break;
		if ((unsigned long)(millis() - timerStart) > timeout * 1000UL) {
			break;
		}
		//If interchar Timeout => return FALSE. So we can return sooner from this function. Not DO it if we dont recieve at least one char (prevChar <> 0)
		if (((unsigned long)(millis() - prevChar) > chartimeout) && (prevChar != 0)) {
			break;
		}
	}
}








void CGPRS_SIM800::readSMS(char *message, char *phone, char *datetime)
{
	/* Response is like:
	+CMT: "+79772941911","","15/12/15,01:51:24+12"
	+CMGR: "REC READ","XXXXXXXXXXX","","14/10/09,17:30:17+08"
	SMS text here


	+CMTI: "SM",2
	62632701","","17/01/20,01:14:36+12"

	*/





	int i = 0;
	int j = 0;

	//char gprsBuffer[160];

	purgeSerial();

	
	//sim900_clean_buffer(gprsBuffer, sizeof(gprsBuffer));
	sim800_read_buffer(buffer, sizeof(buffer));


	int len = strlen(buffer);
	Serial.println(buffer);

	
	if (buffer[i] == '\"') {
		i++;
		j = 0;
		while (buffer[i] != '\"') {
			phone[j++] = buffer[i++];
		}
		phone[j] = '\0';
		i++;
	}

	if (buffer[i] == ',')
		i++;

	if (buffer[i] == '\"') {
		i++;
		while (buffer[i] != '\"') {
			i++;
		}
		i++;
	}

	if (buffer[i] == ',')
		i++;

	if (buffer[i] == '\"') {
		i++;
		j = 0;
		while (buffer[i] != '\"') {
			datetime[j++] = buffer[i++];
		}
		datetime[j] = '\0';
		i++;
	}

	if (buffer[i] == '\r')
		i++;

	if (buffer[i] == '\n')
		i++;

	j = 0;
	while (i < len - 2)
		message[j++] = buffer[i++];

	message[j] = '\0';
	
}







/*

bool CGPRS_SIM800::checkSMSU()
{
 if (SIM_SERIAL->available())             //���� ������ �� GSM ������
 {          
	delay(100);                          //������, ����� ������ ������ ������� � ���� ������� ������ ��� ����� �������
	while (SIM_SERIAL->available())       //���� ������ �� GSM ������
	{    
	  ch = SIM_SERIAL->read();
	  val += char(ch);                   //��������� ������� ������ � ���������� val
	  delay(10);
	}
	Serial.println(val);
	return true;
  }
  return false; 
}
*/

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

bool CGPRS_SIM800::getLocation(GSM_LOCATION* loc)
{
  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[34])));
  if (sendCommand(bufcom, 10000)) do         // if (sendCommand("AT+CIPGSMLOC=1,1", 10000)) do 
  {
	char *p;
	if (!(p = strchr(buffer, ':'))) break;
	if (!(p = strchr(p, ','))) break;
	loc->lon = atof(++p);
	if (!(p = strchr(p, ','))) break;
	loc->lat = atof(++p);
	if (!(p = strchr(p, ','))) break;
	loc->year = atoi(++p) - 2000;
	if (!(p = strchr(p, '/'))) break;
	loc->month = atoi(++p);
	if (!(p = strchr(p, '/'))) break;
	loc->day = atoi(++p);
	if (!(p = strchr(p, ','))) break;
	loc->hour = atoi(++p);
	if (!(p = strchr(p, ':'))) break;
	loc->minute = atoi(++p);
	if (!(p = strchr(p, ':'))) break;
	loc->second = atoi(++p);
	return true;
  } while(0);
  return false;
}

void CGPRS_SIM800::httpUninit()
{
  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[35])));
  sendCommand(bufcom);          // sendCommand("AT+HTTPTERM");
}

bool CGPRS_SIM800::httpInit()
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[36])));
	strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[37])));
	if  (!sendCommand(bufcom, 10000) || !sendCommand(bufcom1, 5000))  //if  (!sendCommand("AT+HTTPINIT", 10000) || !sendCommand("AT+HTTPPARA=\"CID\",1", 5000)) 
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
// ���������, ���� ���������� HTTP �����������
// ���������� 0 ��� ��������� �, 1 ��� ������, 2 ��� ������

byte CGPRS_SIM800::httpIsConnected()
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[42])));
	strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[43])));
	byte ret = checkbuffer(bufcom,bufcom1, 10000);           // byte ret = checkbuffer("0,200", "0,60", 10000);
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
// ���������, ���� ���������� HTTP �����������
// ���������� �������� 0 ��� ������������, -1 ��� ������, ���������� ������ �������� �������� HTTP �� �����

int CGPRS_SIM800::httpIsRead()
{
	strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[45])));
	strcpy_P(bufcom1, (char*)pgm_read_word(&(table_message[46])));
	byte ret = checkbuffer(bufcom, bufcom1, 10000) == 1;//byte ret = checkbuffer("+HTTPREAD: ", "Error", 10000) == 1;
	if (ret == 1) 
	{
		m_bytesRecv = 0;
		// read the rest data
		sendCommand(0, 100, "\r\n");
		int bytes = atoi(buffer);
		sendCommand(0);
		bytes = min(bytes, sizeof(buffer) - 1);
		buffer[bytes] = 0;
		return bytes;
	} else if (ret >= 2) 
	{
		httpState = HTTP_ERROR;
		return -1;
	}
	return 0;
}

byte CGPRS_SIM800::sendCommand(const char* cmd, unsigned int timeout, const char* expected)
{
  if (cmd) 
  {
	purgeSerial();   // �������� �������� ������
#ifdef DEBUG
	DEBUG.print('>');
	DEBUG.println(cmd);
#endif
	SIM_SERIAL->println(cmd);
  }
  uint32_t t = millis();
  byte n = 0;
  do {
	if (SIM_SERIAL->available()) 
	{
	  char c = SIM_SERIAL->read();
	  if (n >= sizeof(buffer) - 1) 
	  {
		// buffer full, discard first half
		n = sizeof(buffer) / 2 - 1;
		memcpy(buffer, buffer + sizeof(buffer) / 2, n);
	  }
	  buffer[n++] = c;
	  buffer[n] = 0;
	  strcpy_P(bufcom, (char*)pgm_read_word(&(table_message[32])));
	if (strstr(buffer, expected ? expected : bufcom))    // if (strstr(buffer, expected ? expected : "OK\r")) 
	  {
#ifdef DEBUG
	   DEBUG.print("[1]");
	   DEBUG.println(buffer);
#endif
	  return n;
	  }
	}
  } while (millis() - t < timeout);
#ifdef DEBUG
   DEBUG.print("[0]");
   DEBUG.println(buffer);
#endif
  return 0;
}




byte CGPRS_SIM800::sendCommand(const char* cmd, const char* expected1, const char* expected2, unsigned int timeout)
{
  if (cmd) 
  {
	purgeSerial();   // �������� �������� ������
	#ifdef DEBUG
		DEBUG.print('>');
		DEBUG.println(cmd);
	#endif
	SIM_SERIAL->println(cmd);
  }
  uint32_t t = millis();
  byte n = 0;
	do {
		if (SIM_SERIAL->available()) 
		{
		  char c = SIM_SERIAL->read();
		  if (n >= sizeof(buffer) - 1) 
		  {
			// buffer full, discard first half
			n = sizeof(buffer) / 2 - 1;
			memcpy(buffer, buffer + sizeof(buffer) / 2, n);
		  }
		  buffer[n++] = c;
		  buffer[n] = 0;
		  if (strstr(buffer, expected1)) 
		  {
			#ifdef DEBUG
				   DEBUG.print("[1]");
				   DEBUG.println(buffer);
			#endif
		   return 1;
		  }
		  if (strstr(buffer, expected2)) 
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
  return 0;
}

byte CGPRS_SIM800::checkbuffer(const char* expected1, const char* expected2, unsigned int timeout)
{
	while (SIM_SERIAL->available()) 
	{
		char c = SIM_SERIAL->read();
		if (m_bytesRecv >= sizeof(buffer) - 1) 
		{
			// buffer full, discard first half ����� ��������, ��������� ������ ��������
			m_bytesRecv = sizeof(buffer) / 2 - 1;
			memcpy(buffer, buffer + sizeof(buffer) / 2, m_bytesRecv);
		}
		buffer[m_bytesRecv++] = c;
		buffer[m_bytesRecv] = 0;
		if (strstr(buffer, expected1)) 
		{
			return 1;
		}
		if (expected2 && strstr(buffer, expected2)) 
		{
			return 2;
		}
	}
	return (millis() - m_checkTimer < timeout) ? 0 : 3;
}

void CGPRS_SIM800::purgeSerial()    // �������� �������� ������
{
   while (SIM_SERIAL->available()) SIM_SERIAL->read();
}
bool CGPRS_SIM800::available()
{
	return SIM_SERIAL->available(); 
}
