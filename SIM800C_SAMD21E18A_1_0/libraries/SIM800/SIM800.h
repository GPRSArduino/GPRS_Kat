/*************************************************************************
* SIM800 GPRS/HTTP Library
* Distributed under GPL v2.0
* Written by Stanley Huang <stanleyhuangyc@gmail.com>
* For more information, please visit http://arduinodev.com
*************************************************************************/

#include <Arduino.h>
#include <avr/wdt.h>
#include <EEPROM.h>
#define PWR_On           5                              // Включение питания модуля SIM800

typedef	Stream 						FONAStreamType;


// define DEBUG to one serial UART to enable debug information output
// #define DEBUG Serial

typedef enum {
	HTTP_DISABLED = 0,
	HTTP_READY,
	HTTP_CONNECTING,
	HTTP_READING,
	HTTP_ERROR,
} HTTP_STATES;


class CGPRS_SIM800 {
public:

	CGPRS_SIM800():httpState(HTTP_DISABLED) {}
	// initialize the module
  
	bool begin(Stream &port);
	uint8_t getNetworkStatus();
	byte connect_GPRS();
	bool connect_IP_GPRS();
	// get network operator name
	bool getOperatorName();
	bool getIMEI();
	bool getSIMCCID();
	byte ping_connect_internet();
	bool ping(const char* url);
	// check for incoming SMS
	bool checkSMS();
	void send_sms(String text, String phone);  //процедура отправки СМС
	bool deleteSMS(int n_sms);
	// get signal quality level (in dB)
	byte getSignalQuality();
	
	// initialize HTTP connection
	bool httpInit();
	// terminate HTTP connection
	void httpUninit();
	// connect to HTTP server
	bool httpConnect(const char* url, const char* args = 0);
	bool httpConnectStr(const char* url, String args = "");
	boolean HTTP_ssl(boolean onoff);
	// check if HTTP connection is established
	// return 0 for in progress, 1 for success, 2 for error
	byte httpIsConnected();
	// read data from HTTP connection
	void httpRead();
	// check if HTTP connection is established
	// return 0 for in progress, -1 for error, bytes of http payload on success
	int httpIsRead();
	// send AT command and check for expected response
	byte sendCommandS(String cmd, unsigned int timeout, const char* expected);
	// send AT command and check for two possible responses
	byte sendCommandS(String cmd);
	// check if there is available serial data
	bool available();
	void cleanStr(String & str);
	void reboot(int count_error);

	char buffer[150];
	char buffer1[22];
	byte httpState;
	String val = "";
	unsigned int errors = 0;
	bool EEPROM_off = false;
	int Address_EEPROM_off = 280;                           // Адрес в EEPROM запрет записи в EEPROM

private:
	byte checkbuffer(const char* expected1, const char* expected2 = 0, unsigned int timeout = 2000);  // По умолчанию ожидание 2 секунды
	void purgeSerial();
	byte m_bytesRecv;                         // 
	uint32_t m_checkTimer;
	String apn            = "";
	String user           = "";
	String pwd            = ""; 
	String cont           = "";
	String no             = "no";
	const char* expected1 = "OK\r";
	const char* expected2 = "ERROR\r";
	const char* OK_r      = "OK\r";
	const char* ERROR_r   = "ERROR\r";
	unsigned int timeout  = 2000;
	byte operator_Num     = 0;                                  // Порядковый номер оператора
	int ch                = 0;
	int Address_errorAll = 160;                        // Адрес в EEPROM счетчика общих ошибок
	FONAStreamType *SIM_SERIAL;
	
};

