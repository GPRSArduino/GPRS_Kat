
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

#define in_run_stop   14          // ������ ������� STOP
#define in_fara_size  15          // ������ ������� ��������
#define in_run_rear   16          // ������ ������� ����� 
#define in_fara_left  17          // ������ ������� ���� �����
#define in_fara_right 18          // ������ ������� ���� ������
#define in_rele1      19          // ������ ������� ���������� ���� 1
#define in_rele2      A6          // ������ ������� ���������� ���� 2

#define in_test       12          // ������ ������� ������ ���� 
#define in_send_rec    2          // ������ ������� ����������� �������� ��� ����������

bool b_run_stop   = false ;       // ������ ������� STOP
bool b_run_rear   = false ;       // ������ ������� ����� 
bool b_fara_size  = false ;       // ������ ������� ��������
bool b_fara_left  = false ;       // ������ ������� ���� �����
bool b_fara_right = false ;       // ������ ������� ���� ������
bool b_rele1      = false ;       // ������ ������� ���������� ���� 1
bool b_rele2      = false ;       // ������ ������� ���������� ���� 2 

bool t_run_stop   = false ;       // ������ ������� STOP
bool t_run_rear   = false ;       // ������ ������� ����� 
bool t_fara_size  = false ;       // ������ ������� ��������
bool t_fara_left  = false ;       // ������ ������� ���� �����
bool t_fara_right = false ;       // ������ ������� ���� ������
bool t_rele1      = false ;       // ������ ������� ���������� ���� 1
bool t_rele2      = false ;       // ������ ������� ���������� ���� 2

IRsend irsend;

int RECV_PIN = 11;

IRrecv irrecv (RECV_PIN);

decode_results results;

unsigned long AC_CODE_TO_SEND;

int o_r = 0;
int r_send = 20;

void ac_send_code(unsigned long code)
{
  Serial.print("code to send : ");
  Serial.print(code, BIN);
  Serial.print(" : ");
  Serial.println(code, HEX);
  irsend.sendLG(code, 28);
}

void set_pin()
{
    pinMode(out_run_stop,  OUTPUT);     // ������ �������� STOP
    pinMode(out_run_rear,  OUTPUT);     // ������ �������� ����� 
    pinMode(out_fara_size, OUTPUT);     // ������ �������� ��������
    pinMode(out_fara_left, OUTPUT);     // ������ �������� ���� �����
    pinMode(out_fara_right,OUTPUT);     // ������ �������� ���� ������
    pinMode(out_rele1,     OUTPUT);     // ������ �������� ���������� ���� 1
    pinMode(out_rele2,     OUTPUT);     // ������ �������� ���������� ���� 2

    pinMode(in_run_stop,  INPUT);       // ������ ������� STOP
    pinMode(in_run_rear,  INPUT);       // ������ ������� ����� 
    pinMode(in_fara_size, INPUT);       // ������ ������� ��������
    pinMode(in_fara_left, INPUT);       // ������ ������� ���� �����
    pinMode(in_fara_right,INPUT);       // ������ ������� ���� ������
    pinMode(in_rele1,     INPUT);       // ������ ������� ���������� ���� 1
    pinMode(in_rele2,     INPUT);       // ������ ������� ���������� ���� 2
	
	pinMode(in_test,      INPUT);       // ������ ������� ������ ����
	pinMode(in_send_rec,  INPUT);       // ������ �������  ����������� �������� ��� ����������

    digitalWrite(out_run_stop,  LOW);   // ������ �������� STOP ���������
    digitalWrite(out_run_rear,  LOW);   // ������ �������� ����� ��������� 
    digitalWrite(out_fara_size, LOW);   // ������ �������� �������� ���������
    digitalWrite(out_fara_left, LOW);   // ������ �������� ���� ����� ���������
    digitalWrite(out_fara_right,LOW);   // ������ �������� ���� ������ ���������
    digitalWrite(out_rele1,     LOW);   // ������ �������� ���������� ���� 1 ���������
    digitalWrite(out_rele2,     LOW);   // ������ �������� ���������� ���� 2 ���������
 
    digitalWrite(in_run_stop,  HIGH);   // ������ ������� STOP ���������� ��������
    digitalWrite(in_run_rear,  HIGH);   // ������ ������� �����  ���������� ��������
    digitalWrite(in_fara_size, HIGH);   // ������ ������� �������� ���������� ��������
    digitalWrite(in_fara_left, HIGH);   // ������ ������� ���� ����� ���������� ��������
    digitalWrite(in_fara_right,HIGH);   // ������ ������� ���� ������ ���������� ��������
    digitalWrite(in_rele1,     HIGH);   // ������ ������� ���������� ���� 1 ���������� ��������
    digitalWrite(in_rele2,     HIGH);   // ������ ������� ���������� ���� 2 ���������� ��������

	digitalWrite(in_test,      HIGH);   // ������ ������� ������ ���� ���������� ��������
	digitalWrite(in_send_rec,  HIGH);   // ������ �������  ����������� �������� ��� ����������
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
		}   
		else if (results.value == 0x2FD40BF) 
		{  
			digitalWrite(out_run_stop, LOW);         // 2 ������ �������� STOP off
		}   
//2
		else if (results.value == 0x2FDC03F) 
		{  
			analogWrite(out_fara_size, 60);          // 3 ������ �������� �������� on 50%    
		}   
		else if (results.value == 0x2FD20DF) 
		{ 
			digitalWrite(out_fara_size, HIGH);       // 4 ������ �������� �������� off 100%
		}  
		else if (results.value == 0x2FDA05F) 
		{  
			digitalWrite(out_fara_size, LOW);       // 5 ������ �������� �������� off
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

void send_avto(int r)
{
  if ( r != o_r) 
  {
    switch (r) 
	{
		case 0:           //  
			AC_CODE_TO_SEND = 0x2FD807F; 
		break;
		case 1:           //  
			AC_CODE_TO_SEND = 0x2FD40BF;
		break;
	
		case 2:          //
			AC_CODE_TO_SEND = 0x2FDC03F;
		break;
		case 3:          // 
			AC_CODE_TO_SEND = 0x2FD20DF;
		break;
		case 4:         //
			AC_CODE_TO_SEND = 0x2FDA05F;
		break;
		
		case 5:         //
			AC_CODE_TO_SEND = 0x2FD609F;
		break;
		case 6:
			AC_CODE_TO_SEND = 0x2FDE01F;
		break;
		
		case 7:
			AC_CODE_TO_SEND = 0x2FD10EF;  
		break;
		case 8:
			AC_CODE_TO_SEND = 0x2FD906F; 
		break;
		
		case 9:
			AC_CODE_TO_SEND = 0x2FD00FF;
		break;
		case 10:
			AC_CODE_TO_SEND = 0x2FD58A7;
		break;
	
		case 11:
			AC_CODE_TO_SEND = 0x2FD7887;
		break;
		case 12:
			AC_CODE_TO_SEND = 0x2FDD827;
		break;
		
		case 13:
			AC_CODE_TO_SEND = 0x2FDF807;
		break;
		case 14:
			AC_CODE_TO_SEND = 0x2FD28D7;
		break;
		default:
		break;
    }
	 ac_send_code(AC_CODE_TO_SEND);
    o_r = r ;
  }
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
	if(digitalRead(in_send_rec) == false)          // ��������� ��������
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
	else                                          // ��������� ����������                 
	{
		bool in_test_t = true;
	//	digitalRead(in_test
        if(in_test_t == false)         // ����������� ����������
		{
			for(int i=0;i<15;i++)
				{
					send_avto(i);
					delay(1000); 
				}
        }
		else                                  
		{
		   t_run_stop = digitalRead(in_run_stop);
           if(t_run_stop  != b_run_stop)     // 1
			   {
				   b_run_stop = t_run_stop ;
				   if(!b_run_stop)
				   {
					   r_send = 0;
				       send_avto(r_send);
				   }
				   else
				   {
                      r_send = 1;
				      send_avto(r_send);
				   }

		       }
		    
		   t_fara_size = digitalRead(in_fara_size);
           if(t_fara_size  != b_fara_size)     // 1
			   {
				   b_fara_size = t_fara_size ;
				   if(!b_fara_size)
				   {
					   r_send = 2;
				       send_avto(r_send);
				   }
				   else
				   {
                      r_send = 4;
				      send_avto(r_send);
				   }

		       }

		   t_run_rear = digitalRead(in_run_rear);
           if(t_run_rear  != b_run_rear)     // 1
			   {
				   b_run_rear = t_run_rear;
				   if(!b_run_rear)
				   {
					   r_send = 5;
				       send_avto(r_send);
				   }
				   else
				   {
                      r_send = 6;
				      send_avto(r_send);
				   }

		       }

		   t_fara_left = digitalRead(in_fara_left);
           if(t_fara_left  != b_fara_left)     // 1
			   {
				   b_fara_left = t_fara_left;
				   if(!b_fara_left)
				   {
					   r_send = 7;
				       send_avto(r_send);
				   }
				   else
				   {
                      r_send = 8;
				      send_avto(r_send);
				   }

		       }

		   t_fara_right = digitalRead(in_fara_right);
           if(t_fara_right  != b_fara_right)     // 1
			   {
				   b_fara_right = t_fara_right;
				   if(!b_fara_right)
				   {
					   r_send = 9;
				       send_avto(r_send);
				   }
				   else
				   {
                      r_send = 10;
				      send_avto(r_send);
				   }

		       }

		   t_rele1 = digitalRead(in_rele1);
           if(t_rele1  != b_rele1)     // 1
			   {
				   b_rele1 = t_rele1;
				   if(!b_rele1)
				   {
					   r_send = 11;
				       send_avto(r_send);
				   }
				   else
				   {
                      r_send = 12;
				      send_avto(r_send);
				   }

		       }

		   	   t_rele2 = digitalRead(in_rele2);
           if(t_rele2  != b_rele2)     // 1
			   {
				   b_rele2 = t_rele2;
				   if(!b_rele2)
				   {
					   r_send = 13;
				       send_avto(r_send);
				   }
				   else
				   {
                      r_send = 14;
				      send_avto(r_send);
				   }

		       }
		  delay(200);
		}
	}
}
