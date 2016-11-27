
/*
���������� �������� ��������� ��� ��������� ������ ������� ����������������� ���������.
����������� ������ ������� � ��� ������ ���������, 7 ��� ��������� (+ � -).
���������� ����� � ������ ���� ��������� ��������� ��������/��������� � ���� �������� �� ����������.
����� �� ���������� �������� ��� �� ������ �����.
�� ������ �����, �������� ������ � �� ��������� � ������� �� 7 ������� ��� ����������� ����������� � ���� ��������, �� � ����� �� ����������� ��� �� �������� �����.
��� �������� ��� ������������� ��� � ����������� ������� ����� �������, ��� � ����� ����� �� ��������� ������� ������������ �����, �� �� ������.
����� ��� ���������:
��� ������ ���������� ����� ���� ������� (������, ��������, ������ ���, � 2 �����������), ��������� ��� ������ ���������� ��� ���������
������� ������� ��� ��� 1-2�. (��� ������� � ��������� ���� �������).
�� �������� ������, ��� �������, ���� ���������� � ����������� �����, 5 ����� �������. �������������� �� ����� ����� ������, ����� ������, � �����. 
� ��������� 2 ������ �� ��������� �������� (��������� ��� ����� ����������).
������������� �� ������������� �� �����������. �� ���������� ��������������� � ����� ����� � ���������, � �� �������� � ����� �� �������. 
����� ������� ����� ������ �������� ���� ��������, ���� �������� �� ��������.
�� ������ �����������, ���� �������� 5 ������� ��� ���� (������, ��������, ������ ���, � 2 �����������) � ��� ��� ����.
��� ���������� ������� � ���� ����� ��� ���������� �������� ���� ULN2003 ��� ULN2803.
������ �������� ����� ��������� ����������� ���� NPN.

 ���� �������� / ������ ������� IRremote, ��������� ���� Arduinos.
�*
�* ������� # 1 ������ ����� ������������ ��, ������������ � ������ ������� (3).
�* Arduino # 2 ������ ����� ��-�������� / �����������, ������������� � �������
�* �������� ����� (11) � ������� ���������, ������������ � �������� 3.
�*
�* ����:
�* Arduino # 1 ����� ����� 2 �������, � ����� ��������� ����� ���������.
�* ��� ��������� ��� ��������.
�* Arduino # 2 ����� ����� � ������� �� ������� ���� ���� ������� ���������� �������
�* (��� ������������� � # 1). �� ����� ������� ���� �� �����
�* �������. �� ����� �������������� ��� ��������� � ����������������� �����. ��� �����
�* ����� ������� ������ ����� ���������, ������� ����� ������ ������ ���, ����� ����
�* ��������. ���� ���� ������, �� ��� ����� ������ � ������� 5 ������.
�*
�* ���� ��������, ���� ��������� ������ 19 ���, ������ �����, � ����� �����������.
�* ���� ������ �������, ���� ��������� ����� � ������� 5 ������.
�*
�* ���� ��������� ������������� ����������, ����� ����� �������� ������������ � �������
�* ��������, ��� ���� �� ���������� �������, ������� ����� ���������
�* �����������. �� ������ ���������� ���������������� ���� � ��������� ��� �������.

 */

#include <IRremote.h>

#define out_run_stop   4          // ������ �������� STOP
#define out_run_rear   5          // ������ �������� ����� 
#define out_fara_size  6          // ������ �������� ��������
#define out_fara_left  7          // ������ �������� ���� �����
#define out_fara_right 8          // ������ �������� ���� ������
#define out_rele1      9          // ������ �������� ���������� ���� 1
#define out_rele2     10          // ������ �������� ���������� ���� 2

#define in_run_stop   A0          // ������ ������� STOP
#define in_run_rear   A1          // ������ ������� ����� 
#define in_fara_size  A2          // ������ ������� ��������
#define in_fara_left  A3          // ������ ������� ���� �����
#define in_fara_right A4          // ������ ������� ���� ������
#define in_rele1      A5          // ������ ������� ���������� ���� 1
#define in_rele2      A6          // ������ ������� ���������� ���� 2

int RECV_PIN = 11;                // ������ ����������� �������� ��� ����������
int LED_PIN  = 3;                 // ����/����� ����- ��� ����������

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

#define RECEIVER 1
#define SENDER 2
#define ERROR 3

int mode;

void set_pin()
{
                                       
	pinMode(out_run_stop,  OUTPUT);     // ������ �������� STOP
	pinMode(out_run_rear,  OUTPUT);     // ������ �������� ����� 
	pinMode(out_fara_size, OUTPUT);     // ������ �������� ��������
	pinMode(out_fara_left, OUTPUT);     // ������ �������� ���� �����
	pinMode(out_fara_right,OUTPUT);     // ������ �������� ���� ������
	pinMode(out_rele1,     OUTPUT);     // ������ �������� ���������� ���� 1
	pinMode(out_rele2,     OUTPUT);     // ������ �������� ���������� ���� 2
	pinMode(LED_PIN,       OUTPUT);     // ������ ����������� �������� ��� ����������

	pinMode(in_run_stop,  INPUT);       // ������ ������� STOP
	pinMode(in_run_rear,  INPUT);       // ������ ������� ����� 
	pinMode(in_fara_size, INPUT);       // ������ ������� ��������
	pinMode(in_fara_left, INPUT);       // ������ ������� ���� �����
	pinMode(in_fara_right,INPUT);       // ������ ������� ���� ������
	pinMode(in_rele1,     INPUT);       // ������ ������� ���������� ���� 1
	pinMode(in_rele2,     INPUT);       // ������ ������� ���������� ���� 2

	digitalWrite(out_run_stop,  LOW);   // ������ �������� STOP ���������
	digitalWrite(out_run_rear,  LOW);   // ������ �������� ����� ��������� 
	digitalWrite(out_fara_size, LOW);   // ������ �������� �������� ���������
	digitalWrite(out_fara_left, LOW);   // ������ �������� ���� ����� ���������
	digitalWrite(out_fara_right,LOW);   // ������ �������� ���� ������ ���������
	digitalWrite(out_rele1,     LOW);   // ������ �������� ���������� ���� 1 ���������
	digitalWrite(out_rele2,     LOW);   // ������ �������� ���������� ���� 2 ���������
	digitalWrite(LED_PIN,       LOW);   // ������ ����������� �������� ��� ���������� ���������

	digitalWrite(in_run_stop,  HIGH);   // ������ ������� STOP ���������� ��������
	digitalWrite(in_run_rear,  HIGH);   // ������ ������� �����  ���������� ��������
	digitalWrite(in_fara_size, HIGH);   // ������ ������� �������� ���������� ��������
	digitalWrite(in_fara_left, HIGH);   // ������ ������� ���� ����� ���������� ��������
	digitalWrite(in_fara_right,HIGH);   // ������ ������� ���� ������ ���������� ��������
	digitalWrite(in_rele1,     HIGH);   // ������ ������� ���������� ���� 1 ���������� ��������
	digitalWrite(in_rele2,     HIGH);   // ������ ������� ���������� ���� 2 ���������� ��������
}


void setup()
{
  Serial.begin(9600);
  set_pin();
 
  if (digitalRead(RECV_PIN) == HIGH) // �������� RECV_PIN, ����� ������, ���� �� �������� ��� ����������
  {
    mode = RECEIVER;
    irrecv.enableIRIn();
    Serial.println("Receiver mode");
  } 
  else 
  {
    mode = SENDER;
    Serial.println("Sender mode");
  }
}

// ���������, ���� ������ ����� �������, ��� ������������� �
// �����������.
// � ���������, ����� ������� � ����������� �������� � ��������� ������� ��.
void waitForGap(int gap) 
{
  Serial.println("Waiting for gap");
  while (1) 
  {
    while (digitalRead(RECV_PIN) == LOW) {}
    unsigned long time = millis();
    while (digitalRead(RECV_PIN) == HIGH) 
	{
      if (millis() - time > gap) 
	  {
        return;
      }
    }
  }
}

// Dumps out the decode_results structure.
// Call this after IRrecv::decode()
void dump(decode_results *results) 
{
  int count = results->rawlen;
  if (results->decode_type == UNKNOWN) 
  {
    Serial.println("Could not decode message");
  } 
  else 
  {
    if (results->decode_type == NEC) 
	{
      Serial.print("Decoded NEC: ");
    } 
    else if (results->decode_type == SONY) 
	{
      Serial.print("Decoded SONY: ");
    } 
    else if (results->decode_type == RC5) 
	{
      Serial.print("Decoded RC5: ");
    } 
    else if (results->decode_type == RC6) 
	{
      Serial.print("Decoded RC6: ");
    }
    Serial.print(results->value, HEX);
    Serial.print(" (");
    Serial.print(results->bits, DEC);
    Serial.println(" bits)");
  }
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");

  for (int i = 0; i < count; i++) {
    if ((i % 2) == 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    } 
    else {
      Serial.print(-(int)results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println("");
}


// �������� �������� ��� ������.
// ���� ����� �����������, ��������� ��� ���������� ����, �������� � ����
// ���� ����� RECEIVER, �������� ��� � ���������, ��� �� �����
// ���������� ����, �������� � ����. ��� ������, ��������� ������;
// ��� ������, ����� ��������������� �� ERROR.
// ���������� ����� ������ ����������� � ���, ��� ����������� � ����������
// ����� ������ �� �� ������ ������������, � ���������� ������ ���������, �������� ��
// ��� �������� ��� ���������.
void test(char *label, int type, unsigned long value, int bits) 
{
  if (mode == SENDER) 
  {
    Serial.println(label);
    if (type == NEC) {
      irsend.sendNEC(value, bits);
    } 
    else if (type == SONY) 
	{
      irsend.sendSony(value, bits);
    } 
    else if (type == RC5) 
	{
      irsend.sendRC5(value, bits);
    } 
    else if (type == RC6) 
	{
      irsend.sendRC6(value, bits);
    } 
    else 
	{
      Serial.print(label);
      Serial.println("Bad type!");
    }
    delay(200);
  } 
  else if (mode == RECEIVER) 
  {
    irrecv.resume(); // Receive the next value
    unsigned long max_time = millis() + 30000;
    Serial.print(label);

    // Wait for decode or timeout
    while (!irrecv.decode(&results)) 
	{
      if (millis() > max_time) 
	  {
        Serial.println("Timeout receiving data");
        mode = ERROR;
        return;
      }
    }
    if (type == results.decode_type && value == results.value && bits == results.bits) 
	{
      Serial.println (": OK");
      digitalWrite(LED_PIN, HIGH);
      delay(20);
      digitalWrite(LED_PIN, LOW);
    } 
    else 
	{
      Serial.println(": BAD");
      dump(&results);
      mode = ERROR;
    }
  }
}

// �������� ����� ��������� ��� ��������. ��� ���������� ������ ���������,
// �� ����������� ����, �������� / �������� �������� ������.
void testRaw(char *label, unsigned int *rawbuf, int rawlen) 
{
  if (mode == SENDER) 
  {
    Serial.println(label);
    irsend.sendRaw(rawbuf, rawlen, 38 /* kHz */);
    delay(200);
  } 
  else if (mode == RECEIVER ) 
  {
    irrecv.resume(); // Receive the next value
    unsigned long max_time = millis() + 30000;
    Serial.print(label);
    // Wait for decode or timeout
    while (!irrecv.decode(&results)) 
	{
      if (millis() > max_time)
	  {
        Serial.println("Timeout receiving data");
        mode = ERROR;
        return;
      }
    }

    // Received length has extra first element for gap
    if (rawlen != results.rawlen - 1) 
	{
      Serial.print("Bad raw length ");
      Serial.println(results.rawlen, DEC);
      mode = ERROR;
      return;
    }
    for (int i = 0; i < rawlen; i++) 
	{
      long got = results.rawbuf[i+1] * USECPERTICK;
      // Adjust for extra duration of marks
      if (i % 2 == 0) { 
        got -= MARK_EXCESS;
      } 
      else {
        got += MARK_EXCESS;
      }
      // See if close enough, within 25%
      if (rawbuf[i] * 1.25 < got || got * 1.25 < rawbuf[i]) 
	  {
        Serial.println(": BAD");
        dump(&results);
        mode = ERROR;
        return;
      }

    }
    Serial.println (": OK");
    digitalWrite(LED_PIN, HIGH);
    delay(20);
    digitalWrite(LED_PIN, LOW);
  }
}   

// This is the raw data corresponding to NEC 0x12345678
unsigned int sendbuf[] = { /* NEC format */
  9000, 4500,
  560, 560, 560, 560, 560, 560, 560, 1690, /* 1 */
  560, 560, 560, 560, 560, 1690, 560, 560, /* 2 */
  560, 560, 560, 560, 560, 1690, 560, 1690, /* 3 */
  560, 560, 560, 1690, 560, 560, 560, 560, /* 4 */
  560, 560, 560, 1690, 560, 560, 560, 1690, /* 5 */
  560, 560, 560, 1690, 560, 1690, 560, 560, /* 6 */
  560, 560, 560, 1690, 560, 1690, 560, 1690, /* 7 */
  560, 1690, 560, 560, 560, 560, 560, 560, /* 8 */
  560};

void loop() 
{
  if (mode == SENDER) 
  {
    delay(2000);  // Delay for more than gap to give receiver a better chance to sync.
  } 
  else if (mode == RECEIVER) 
  {
    waitForGap(1000);
  } 
  else if (mode == ERROR)
  {
    // Light up for 5 seconds for error
    digitalWrite(LED_PIN, HIGH);
    delay(5000);
    digitalWrite(LED_PIN, LOW);
    mode = RECEIVER;  // Try again
    return;
  }

  // The test suite.
  test("SONY1", SONY, 0x123, 12);
  test("SONY2", SONY, 0x000, 12);
  test("SONY3", SONY, 0xfff, 12);
  test("SONY4", SONY, 0x12345, 20);
  test("SONY5", SONY, 0x00000, 20);
  test("SONY6", SONY, 0xfffff, 20);
  test("NEC1", NEC, 0x12345678, 32);
  test("NEC2", NEC, 0x00000000, 32);
  test("NEC3", NEC, 0xffffffff, 32);
  test("NEC4", NEC, REPEAT, 32);
  test("RC51", RC5, 0x12345678, 32);
  test("RC52", RC5, 0x0, 32);
  test("RC53", RC5, 0xffffffff, 32);
  test("RC61", RC6, 0x12345678, 32);
  test("RC62", RC6, 0x0, 32);
  test("RC63", RC6, 0xffffffff, 32);

  // Tests of raw sending and receiving.
  // First test sending raw and receiving raw.
  // Then test sending raw and receiving decoded NEC
  // Then test sending NEC and receiving raw
  testRaw("RAW1", sendbuf, 67);
  if (mode == SENDER) 
  {
    testRaw("RAW2", sendbuf, 67);
    test("RAW3", NEC, 0x12345678, 32);
  } 
  else 
  {
    test("RAW2", NEC, 0x12345678, 32);
    testRaw("RAW3", sendbuf, 67);
  }
}