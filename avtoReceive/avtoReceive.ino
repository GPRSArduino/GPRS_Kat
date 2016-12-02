
/*

��������� ���������!!!


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
 *
 * ������� # 1 ������ ����� ������������ ��, ������������ � ������ ������� (3).
 * Arduino # 2 ������ ����� ��-�������� / �����������, ������������� � �������
 * �������� ����� (11) � ������� ���������, ������������ � �������� 3.
 *
 * ����:
 * Arduino # 1 ����� ����� 2 �������, � ����� ��������� ����� ���������.
 * ��� ��������� ��� ��������.
 * Arduino # 2 ����� ����� � ������� �� ������� ���� ���� ������� ���������� �������
 * (��� ������������� � # 1). �� ����� ������� ���� �� �����
 * �������. �� ����� �������������� ��� ��������� � ����������������� �����. ��� �����
 * ����� ������� ������ ����� ���������, ������� ����� ������ ������ ���, ����� ����
 * ��������. ���� ���� ������, �� ��� ����� ������ � ������� 5 ������.
 *
 * ���� ��������, ���� ��������� ������ 19 ���, ������ �����, � ����� �����������.
 * ���� ������ �������, ���� ��������� ����� � ������� 5 ������.
 *
 * ���� ��������� ������������� ����������, ����� ����� �������� ������������ � �������
 * ��������, ��� ���� �� ���������� �������, ������� ����� ���������
 * �����������. �� ������ ���������� ���������������� ���� � ��������� ��� �������.

 1 2FD807F
 2 2FD40BF
 3 2FDC03F
 4 2FD20DF
 5 2FDA05F
 6 2FD609F
 7 2FDE01F
 8 2FD10EF
 9 2FD906F
 0 2FD00FF
 +v 2FD58A7
 -v 2FD7887
 +p 2FDD827
 -p 2FDF807
 AV 2FD28D7
fav 2FD8877
 */

#include <IRremote.h>
#include <Wire.h>

#define out_run_stop   4          // ������ �������� STOP
#define out_fara_size  5          // ������ �������� ��������
#define out_run_rear   6          // ������ �������� ����� 
#define out_fara_left  7          // ������ �������� ���� �����
#define out_fara_right 8          // ������ �������� ���� ������
#define out_rele1      9          // ������ �������� ���������� ���� 1
#define out_rele2     10          // ������ �������� ���������� ���� 2

#define in_test       12          // ������ ������� ������ ���� 

bool stop = false;
bool size = false;
int stop_size = 60;

int RECV_PIN = 11;

IRrecv irrecv (RECV_PIN);

decode_results results;


void set_pin()
{
    pinMode(out_run_stop,  OUTPUT);     // ������ �������� STOP
    pinMode(out_fara_size, OUTPUT);     // ������ �������� ��������
    pinMode(out_run_rear,  OUTPUT);     // ������ �������� ����� 
    pinMode(out_fara_left, OUTPUT);     // ������ �������� ���� �����
    pinMode(out_fara_right,OUTPUT);     // ������ �������� ���� ������
    pinMode(out_rele1,     OUTPUT);     // ������ �������� ���������� ���� 1
    pinMode(out_rele2,     OUTPUT);     // ������ �������� ���������� ���� 2

    digitalWrite(out_run_stop,  LOW);   // ������ �������� STOP ���������
    digitalWrite(out_fara_size, LOW);   // ������ �������� �������� ���������
    digitalWrite(out_run_rear,  LOW);   // ������ �������� ����� ��������� 
    digitalWrite(out_fara_left, LOW);   // ������ �������� ���� ����� ���������
    digitalWrite(out_fara_right,LOW);   // ������ �������� ���� ������ ���������
    digitalWrite(out_rele1,     LOW);   // ������ �������� ���������� ���� 1 ���������
    digitalWrite(out_rele2,     LOW);   // ������ �������� ���������� ���� 2 ���������
	
	pinMode(in_test,       INPUT);     // ������ ������� ������ ����
	digitalWrite(in_test,   HIGH);     // ������ ������� ������ ���� ���������� ��������
}

void recv_avto()      // ����� � ����������� ����
{
	if (irrecv.decode(&results)) 
	{
		Serial.println(results.value, HEX);            //"����������" �������� ���

//1	
		if (results.value == 0x2FD807F) 
		{  
			digitalWrite(out_run_stop, HIGH);         // 1 ������ �������� STOP on
			stop = true;
			analogWrite(out_fara_size, 255);          // 3 ������ �������� �������� on 
		}   
		else if (results.value == 0x2FD40BF)        // ������ �������� STOP off
		{  
			digitalWrite(out_run_stop, LOW);         // 1 ������ �������� STOP on
			stop = false;
			if(size)
			{
                analogWrite(out_fara_size, stop_size);          // 3 ������ �������� �������� on 
			}
			else
			{
				analogWrite(out_fara_size, 0);          // ������ �������� STOP off
			}

		}   
//2
		else if (results.value == 0x2FDC03F) 
		{  
			stop_size = 60;
			size = true;
			if(!stop)
		    {
		    	analogWrite(out_fara_size, stop_size);          // 3 ������ �������� �������� on 
	        }
		}   
		else if (results.value == 0x2FDA05F) 
		{ 
			size = false;
			if(!stop)
			{
				analogWrite(out_fara_size, 0);              // 5 ������ �������� �������� off
			}
		}   
//3
		if (results.value == 0x2FD609F) 
		{  
			digitalWrite(out_run_rear, HIGH);       // 6 ������ �������� ����� on
		}   
		else if (results.value == 0x2FDE01F) 
		{  
			digitalWrite(out_run_rear, LOW);        // 7 ������ �������� ����� off
		}   
//4
		else if (results.value == 0x2FD10EF) 
		{  
			digitalWrite(out_fara_left, HIGH);     // 8 ������ �������� ���� ����� on
		}   
		else if (results.value == 0x2FD906F) 
		{ 
			digitalWrite(out_fara_left, LOW);      // 9 ������ �������� ���� ����� off
		}  
//5		
		else if (results.value == 0x2FD00FF) 
		{  
			digitalWrite(out_fara_right, HIGH);    // 0 ������ �������� ���� ������ on
		}   
		else if (results.value == 0x2FD58A7) 
		{  
			digitalWrite(out_fara_right, LOW);     // v+ ������ �������� ���� ������ off 
		}   
//6
		else if (results.value == 0x2FD7887) 
		{  
			digitalWrite(out_rele1, HIGH);        // v- ������ �������� ���������� ���� 1 on
		}   
		else if (results.value == 0x2FDD827) 
		{ 
			digitalWrite(out_rele1, LOW);   // +p ������ �������� ���������� ���� 1 off
		}  
//7
		else if (results.value == 0x2FDF807) 
		{  
			digitalWrite(out_rele2, HIGH);  // -p ������ �������� ���������� ���� 2 on
		}   
		else if (results.value == 0x2FD28D7) 
		{  
			digitalWrite(out_rele2, LOW);   // AV ������ �������� ���������� ���� 2 off
		}   

		irrecv.resume();                   // Receive the next value
	}
	delay(100);
}

void setup()
{
  Serial.begin(115200);
   set_pin();
  irrecv.enableIRIn();             // Start the receiver
    Serial.println("  - - - START - - -   ");
}

void loop()
{

	if(digitalRead(in_test) == false)          // ����������� ��������
	{
		for(int i = 4; i<11;i++)
		{
			digitalWrite(i, HIGH);              // ������  
			delay(300); 
			digitalWrite(i, LOW);               // ������  
			delay(300);
		}
		for(int i = 0; i<256;i++)
		{
				analogWrite(5, i);              // ������  
	  			delay(20);
		}
		delay(1000);
		analogWrite(5, 0);                    // ������  
	}
	else
	{
		recv_avto();                          // ������� ������
		delay(100);
	}

}
