/*************************************************************************
* SIM800 GPRS/HTTP Library
* Distributed under GPL v2.0
* Written by Stanley Huang <stanleyhuangyc@gmail.com>
* For more information, please visit http://arduinodev.com
*************************************************************************/

#include <Arduino.h>
#include <avr/pgmspace.h>

// change this to the pin connect with SIM800 reset pin
#define PIN_TX           7                             // Подключить  к выводу 7 сигнал RX модуля GPRS
#define PIN_RX           8                             // Подключить  к выводу 8 сигнал TX модуля GPRS


// define DEBUG to one serial UART to enable debug information output
//#define DEBUG Serial


const char  txt_AT[]                 PROGMEM  = "ATE0";
const char  txt_IPR[]                PROGMEM  = "AT+IPR=19200";
const char  txt_CFUN[]               PROGMEM  = "AT+CFUN=1";
const char  txt_CMGF1[]              PROGMEM  = "AT+CMGF=1";
const char  txt_CLIP[]               PROGMEM  = "AT+CLIP=1";
const char  txt_CSCS[]               PROGMEM  = "AT+CSCS=\"GSM\"";
const char  txt_CNMI[]               PROGMEM  = "AT+CNMI=2,2";
const char  txt_CREG[]               PROGMEM  = "AT+CREG?";
const char  txt_CSQ[]                PROGMEM  = "AT+CSQ";
const char  txt_CGATT[]              PROGMEM  = "AT+CGATT?";
const char  txt_SAPBR0[]             PROGMEM  = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"";
const char  txt_internet_mts_ru[]    PROGMEM  = "internet.mts.ru";
const char  txt_MTSB[]               PROGMEM  = "MTS";
const char  txt_mts[]                PROGMEM  = "mts";
const char  txt_BeelineB[]           PROGMEM  = "Beeline";
const char  txt_internet_beeline[]   PROGMEM  = "internet.beeline.ru";
const char  txt_beeline[]            PROGMEM  = "beeline";
const char  txt_MegaFon[]            PROGMEM  = "MegaFon";
const char  txt_internet[]           PROGMEM  = "internet"; 
const char  txt_MEGAFONB[]           PROGMEM  = "MEGAFON";
const char  txt_TELE2[]              PROGMEM  = "TELE2";
const char  txt_internet_TELE2[]     PROGMEM  = "internet.TELE2.ru";
const char  txt_SAPBR1[]             PROGMEM  = "AT+SAPBR=3,1,\"APN\",\"";
const char  txt_SAPBR2[]             PROGMEM  = "AT+SAPBR=3,1,\"USER\",\""; 
const char  txt_SAPBR3[]             PROGMEM  = "AT+SAPBR=3,1,\"PWD\",\"";
const char  txt_CGDCONT[]            PROGMEM  = "AT+CGDCONT=1,\"IP\",\"";
const char  txt_SAPBR4[]             PROGMEM  = "AT+SAPBR=1,1";
const char  txt_SAPBR5[]             PROGMEM  = "AT+SAPBR=2,1";
const char  txt_CMGF2[]              PROGMEM  = "AT+CMGF=1";
const char  txt_CPMS[]               PROGMEM  = "AT+CPMS=\"SM\",\"SM\",\"SM\"";
const char  txt_GSN[]                PROGMEM  = "AT+GSN";
const char  txt_COPS[]               PROGMEM  = "AT+COPS?";
const char  txt_OK[]                 PROGMEM  = "OK\r";
const char  txt_ERROR[]              PROGMEM  = "ERROR\r"; 
const char  txt_CIPGSMLOC[]          PROGMEM  = "AT+CIPGSMLOC=1,1";
const char  txt_HTTPTERM[]           PROGMEM  = "AT+HTTPTERM";
const char  txt_HTTPINIT[]           PROGMEM  = "AT+HTTPINIT";
const char  txt_HTTPPARA1[]          PROGMEM  = "AT+HTTPPARA=\"CID\",1";
const char  txt_HTTPPARA2[]          PROGMEM  = "AT+HTTPPARA=\"URL\",\"";
const char  txt_HTTPACTION1[]        PROGMEM  = "AT+HTTPACTION=0";
const char  txt_HTTPPARA3[]          PROGMEM  = "AT+HTTPPARA=\"URL\",\"";
const char  txt_HTTPACTION2[]        PROGMEM  = "AT+HTTPACTION=0";
const char  txt_200[]                PROGMEM  = "0,200";
const char  txt_60[]                 PROGMEM  = "0,60";
const char  txt_HTTPREAD1[]          PROGMEM  = "AT+HTTPREAD";
const char  txt_HTTPREAD2[]          PROGMEM  = "+HTTPREAD: ";
const char  txt_Error1[]             PROGMEM  = "Error";
const char  txt_r_n[]                PROGMEM  = "\r\n";

const char* const table_message[] PROGMEM =
{
 txt_AT,                      // 0 "AT";
 txt_IPR,                     // 1 "AT+IPR=19200";
 txt_CFUN,                    // 2 "AT+CFUN=1"
 txt_CMGF1,                   // 3 "AT+CMGF=1"
 txt_CLIP,                    // 4 "AT+CLIP=1"
 txt_CSCS,                    // 5 "AT+CSCS=\"GSM\""
 txt_CNMI,                    // 6 "AT+CNMI=2,2"
 txt_CREG,                    // 7 "AT+CREG?"
 txt_CSQ,                     // 8 "AT+CSQ"
 txt_CGATT,                   // 9 "AT+CGATT?"
 txt_SAPBR0,                  // 10 "AT+SAPBR=3,1,\"Contype\",\"GPRS\""
 txt_internet_mts_ru,         // 11 "internet.mts.ru"
 txt_MTSB,                    // 12 "MTS";
 txt_mts,                     // 13 "mts
 txt_BeelineB,                // 14 "Beeline";
 txt_internet_beeline,        // 15 "internet.beeline.ru";
 txt_beeline,                 // 16 "beeline";
 txt_MegaFon,                 // 17 "MegaFon";
 txt_internet,                // 18 "internet"; 
 txt_MEGAFONB,                // 19 "MEGAFON";
 txt_TELE2,                   // 20 "TELE2";
 txt_internet_TELE2,          // 21 "internet.TELE2.ru";
 txt_SAPBR1,                  // 22 "AT+SAPBR=3,1,\"APN\",\"";
 txt_SAPBR2,                  // 23 "AT+SAPBR=3,1,\"USER\",\""; 
 txt_SAPBR3,                  // 24 "AT+SAPBR=3,1,\"PWD\",\"";
 txt_CGDCONT,                 // 25 "AT+CGDCONT=1,\"IP\",\"";
 txt_SAPBR4,                  // 26 "AT+SAPBR=1,1";
 txt_SAPBR5,                  // 27 "AT+SAPBR=2,1";
 txt_CMGF2,                   // 28 "AT+CMGF=1";
 txt_CPMS,                    // 29 "AT+CPMS=\"SM\",\"SM\",\"SM\"";
 txt_GSN,                     // 30 "AT+GSN";
 txt_COPS,                    // 31 "AT+COPS?";
 txt_OK,                      // 32 "OK\r";
 txt_ERROR,                   // 33 "ERROR\r"; 
 txt_CIPGSMLOC,               // 34 "AT+CIPGSMLOC=1,1";
 txt_HTTPTERM,                // 35 "AT+HTTPTERM";
 txt_HTTPINIT,                // 36 "AT+HTTPINIT";
 txt_HTTPPARA1,               // 37 "AT+HTTPPARA=\"CID\",1";
 txt_HTTPPARA2,               // 38 "AT+HTTPPARA=\"URL\",\"";
 txt_HTTPACTION1,             // 39 "AT+HTTPACTION=0";
 txt_HTTPPARA3,               // 40 "AT+HTTPPARA=\"URL\",\"";
 txt_HTTPACTION2,             // 41 "AT+HTTPACTION=0";
 txt_200,                     // 42 "0,200" ;
 txt_60,                      // 43"0,60";
 txt_HTTPREAD1,               // 44 "AT+HTTPREAD";
 txt_HTTPREAD2,               // 45 "+HTTPREAD: ";
 txt_Error1,                  // 46 "Error";
 txt_r_n                      // 47 "\r\n";  
};


typedef enum {
    HTTP_DISABLED = 0,
    HTTP_READY,
    HTTP_CONNECTING,
    HTTP_READING,
    HTTP_ERROR,
} HTTP_STATES;

typedef struct {
  float lat;
  float lon;
  uint8_t year; /* year past 2000, e.g. 15 for 2015 */
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} GSM_LOCATION;

class CGPRS_SIM800 {
public:
    CGPRS_SIM800():httpState(HTTP_DISABLED) {}
    // initialize the module
    bool init(int PWR_On,int SIM800_RESET_PIN,int LED13);
    // setup network
    byte setup();
	// byte setup(const char* apn, const char* user, const char* pwd);
    // get network operator name
    bool getOperatorName();
	bool getIMEI();
    // check for incoming SMS
    bool checkSMS();
	bool checkSMSU();
    // get signal quality level (in dB)
    int getSignalQuality();
    // get GSM location and network time
    bool getLocation(GSM_LOCATION* loc);
    // initialize HTTP connection
    bool httpInit();
    // terminate HTTP connection
    void httpUninit();
    // connect to HTTP server
    bool httpConnect(const char* url, const char* args = 0);
	bool httpConnectStr(const char* url, String args = "");

    // check if HTTP connection is established
    // return 0 for in progress, 1 for success, 2 for error
    byte httpIsConnected();
    // read data from HTTP connection
    void httpRead();
    // check if HTTP connection is established
    // return 0 for in progress, -1 for error, bytes of http payload on success
    int httpIsRead();
    // send AT command and check for expected response
    byte sendCommand(const char* cmd, unsigned int timeout = 2000, const char* expected = 0);
    // send AT command and check for two possible responses
    byte sendCommand(const char* cmd, const char* expected1, const char* expected2, unsigned int timeout = 2000);
    // toggle low-power mode
    bool sleep(bool enabled)
    {
      return sendCommand(enabled ? "AT+CFUN=0" : "AT+CFUN=1");
    }
    // check if there is available serial data
    bool available();
	void cleanStr(String & str);

    //{
    // // return SIM_SERIAL.available(); 
    //}
    char buffer[128];
    byte httpState;
	String val = "";

private:
    byte checkbuffer(const char* expected1, const char* expected2 = 0, unsigned int timeout = 2000);
    void purgeSerial();
    byte m_bytesRecv;
    uint32_t m_checkTimer;
	String apn  = "";
    String user = "";
    String pwd  = "";
    String cont = "";
	int _PWR_On           ;                            // Включение питания модуля SIM800
    int _SIM800_RESET_PIN ;                            // Сброс модуля SIM800
    int _LED13            ;                            // Индикация светодиодом
	char bufcom[40];
	char bufcom1[20];
	int ch = 0;
};

