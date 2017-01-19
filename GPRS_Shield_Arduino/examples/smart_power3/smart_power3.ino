// библиотека для работы с GPRS устройством

/* СМС   -     On          включить реле
 * СМС   -     Off         выключить реле
 * СМС   -     State       запросить состояние реле
 * СМС   -     can1on      включить  канал №1
 * СМС   -     can1off     выключить канал №1
 * СМС   -     State1      запросить состояние канала №1
 * СМС   -     can2on      включить  канал №2
 * СМС   -     can2off     выключить канал №2
 * СМС   -     State2      запросить состояние канала №2
 * СМС   -     can3on      включить  канал №3
 * СМС   -     can3off     выключить канал №3
 * СМС   -     State3      запросить состояние канала №3
 * СМС   -     Temp        запросить состояние датчиков температуры, влажности, газа, освещенности
 * 
 * 
 * Индикация приема СМС светодиодом
 * 
 * 
 */

#include <GPRS_Shield_Arduino.h>

// библиотека для эмуляции Serial порта
// она нужна для работы библиотеки GPRS_Shield_Arduino
#include <SoftwareSerial.h>
// библиотека для работы с датчиком DHT11
#include <dht11.h>

// даём разумное имя для пина к которому подключен датчик DHT11
#define DHT11_PIN 12
// даём разумное имя для пина к которому подключен датчик влажности почвы
#define MOISTURE_PIN A0
// даём разумное имя для пина к которому подключен датчик уровня CO2
#define MQ2_PIN A2
// даём разумное имя для пина к которому подключен датчик уровня освещённости
#define LIGHT_PIN A4


// размер массива, содержащий TCP-запрос
#define LEN 350
// буфер для отправки данных на народный мониторинг
// согласно установленной сервисом форме
char tcpBuffer[LEN];

// переменная температуры воздуха
int temp = 0;
// переменная влажности воздуха
int humi = 0;
// переменная влажности почвы
int moisture = 0;
// переменная уровня CO2
int mq2 = 0;
// переменная уровня освещённости
int light = 0;

// создаём объект класса dht11
dht11 DHT;


#define _pkPin    2           // Управление питанием модуля GPRS. Уточнить !!
#define _stPin    3           // Управление питанием модуля GPRS. Уточнить !! 
#define PIN_TX    7           // Подключить  к выводу 7 сигнал RX модуля GPRS
#define PIN_RX    8           // Подключить  к выводу 8 сигнал TX модуля GPRS
#define BAUDRATE  9600


// длина сообщения
#define MESSAGE_LENGTH 160

#define MESSAGE_ON   "Power is On"                   // текст сообщения о включении розетки
#define MESSAGE_OFF  "Power is Off"                  // текст сообщения о выключении розетки
#define CHANNEL1_ON  "Channel1 is On"                // текст сообщения о включении канала №1
#define CHANNEL1_OFF "Channel1 is Off"               // текст сообщения о выключении канала №1
#define CHANNEL2_ON  "Channel2 is On"                // текст сообщения о включении канала №2
#define CHANNEL2_OFF "Channel2 is Off"               // текст сообщения о выключении канала №2
#define CHANNEL3_ON  "Channel3 is On"                // текст сообщения о включении канала №3
#define CHANNEL3_OFF "Channel3 is Off"               // текст сообщения о выключении канала №3


#define MESSAGE_ERROR  "Error...unknown command!"    // текст сообщения об ошибке распознавания команды

#define RELAY 5                                      // Пин, к которому подключено реле
#define CHANNEL1 9                                   // Пин, к которому подключен канал №1
#define CHANNEL2 10                                  // Пин, к которому подключен канал №2
#define CHANNEL3 11                                  // Пин, к которому подключен канал №3


#define LED13 13                                     // Индикация светодиодом
// номер сообщения в памяти сим-карты
int messageIndex = 0;

// текст сообщения
char message[MESSAGE_LENGTH];
// номер, с которого пришло сообщение
char phone[16];
// дата отправки сообщения
char datetime[24];

bool stateRelay    = false;
bool stateCHANNEL1 = false;
bool stateCHANNEL2 = false;
bool stateCHANNEL3 = false;


// создаём объект класса GPRS и передаём ему скорость 9600 бод;
// с помощью него будем давать команды GPRS шилду
GPRS gprs(_pkPin, _stPin, PIN_TX,PIN_RX,BAUDRATE);
//GPRS gprs(9600);



void setup()
{
  // настраиваем пин реле в режим выхода,
  pinMode(RELAY,    OUTPUT);
  pinMode(CHANNEL1, OUTPUT);
  pinMode(CHANNEL2, OUTPUT);
  pinMode(CHANNEL3, OUTPUT);
  pinMode(LED13,    OUTPUT);
  
  // подаём на пин реле «низкий уровень» (размыкаем реле)
  digitalWrite(RELAY,    LOW);
  digitalWrite(CHANNEL1, LOW);
  digitalWrite(CHANNEL2, LOW);
  digitalWrite(CHANNEL3, LOW);
  digitalWrite(LED13,    LOW);

  // gprs.powerUpDown();             // !! Уточнить, временно отключил. Включаем GPRS-шилд
  // открываем последовательный порт для мониторинга действий в программе
  Serial.begin(9600);
  while (!Serial) {
    // ждём, пока не откроется монитор последовательного порта
    // для того, чтобы отследить все события в программе
  }
  // проверяем, есть ли связь с GPRS-устройством
  while (!gprs.init()) {
    // если связи нет, ждём 1 секунду
    // и выводим сообщение об ошибке;
    // процесс повторяется в цикле,
    // пока не появится ответ от GPRS устройства
    delay(1000);
   // Serial.print("Init error\r\n");
  }
  // вывод об удачной инициализации GPRS Shield
 // Serial.println("GPRS init success");
}


void loop()
{
  // проверяем наличие непрочитанных сообщений
  // и находим их номер в памяти сим-карты
  messageIndex = gprs.isSMSunread();
  if (messageIndex > 0) 
  {
    // если есть хотя бы одно непрочитанное сообщение,
    // читаем его
   // gprs.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime);
    gprs.readSMS(message, phone, datetime);
    gprs.readSMS(messageIndex, message, MESSAGE_LENGTH);
    // Удаляем прочитанное сообщение из памяти Сим-карты
    gprs.deleteSMS(messageIndex);

    // выводим номер, с которого пришло смс
   // Serial.print("From number: ");
  //  Serial.println(phone);

    // выводим дату, когда пришло смс
  //  Serial.print("Datetime: ");
  //  Serial.println(datetime);

    // выводим текст сообщения
  //  Serial.print("Recieved Message: ");
  //  Serial.println(message);
    // вызываем функцию изменения состояния реле
    // в зависимости от текста сообщения
    digitalWrite(LED13,    HIGH);
    setRelay(phone, message);
    delay(1000);
    digitalWrite(LED13,    LOW);
  }
}

void setRelay(char f_phone[], char f_message[])
{
  if (strcmp(f_message, "On") == 0) 
  {
    // если сообщение — с текстом «On»,
    // выводим сообщение в Serial
    // и подаём на замыкаем реле
  //  Serial.println("OK! Power On");
    digitalWrite(RELAY, HIGH);
    stateRelay = true;
    // на номер, с которого пришёл запрос,
    // отправляем смс с текстом о включении питания
    gprs.sendSMS(f_phone, MESSAGE_ON);
  } 
  else if (strcmp(f_message, "Off") == 0) 
  {
    // если пришло сообщение с текстом «Off»,
    // выводим сообщение в Serial
    // и размыкаем реле
  //  Serial.println("OK! Power Off");
    digitalWrite(RELAY, LOW);
    stateRelay = false;
    // на номер, с которого пришёл запрос
    // отправляем смс с текстом о выключении питания
    gprs.sendSMS(f_phone, MESSAGE_OFF);
  }
  else if (strcmp(f_message, "can1on") == 0) 
  {
    // если сообщение — с текстом «can1on»,
    // выводим сообщение в Serial
    // и подаём на замыкаем реле
  //  Serial.println("OK! Channel 1 On");
    digitalWrite(CHANNEL1, HIGH);
    stateCHANNEL1 = true;
    // на номер, с которого пришёл запрос,
    // отправляем смс с текстом о включении канала №1
    gprs.sendSMS(f_phone, CHANNEL1_ON);
  } 
  else if (strcmp(f_message, "can1off") == 0) 
  {
    // если пришло сообщение с текстом «can1off»,
    // выводим сообщение в Serial
    // и размыкаем реле
  //  Serial.println("OK! Channel 1 Off");
    digitalWrite(CHANNEL1, LOW);
   stateCHANNEL1 = false;
    // на номер, с которого пришёл запрос
    // отправляем смс с текстом о выключении канала №1
    gprs.sendSMS(f_phone, CHANNEL1_OFF);
  }
 else if (strcmp(f_message, "can2on") == 0) 
  {
    // если сообщение — с текстом «can2on»,
    // выводим сообщение в Serial
    // и подаём на замыкаем реле
  //  Serial.println("OK! Channel 2 On");
    digitalWrite(CHANNEL2, HIGH);
    stateCHANNEL2 = true;
    // на номер, с которого пришёл запрос,
    // отправляем смс с текстом о включении канала №2
    gprs.sendSMS(f_phone, CHANNEL2_ON);
  } 
  else if (strcmp(f_message, "can2off") == 0) 
  {
    // если пришло сообщение с текстом «can2off»,
    // выводим сообщение в Serial
    // и размыкаем реле
  //  Serial.println("OK! Channel 2 Off");
    digitalWrite(CHANNEL2, LOW);
   stateCHANNEL2 = false;
    // на номер, с которого пришёл запрос
    // отправляем смс с текстом о выключении канала №2
    gprs.sendSMS(f_phone, CHANNEL2_OFF);
  }
 else if (strcmp(f_message, "can3on") == 0) 
  {
    // если сообщение — с текстом «can3on»,
    // выводим сообщение в Serial
    // и подаём на замыкаем реле
  //  Serial.println("OK! Channel 3 On");
    digitalWrite(CHANNEL3, HIGH);
    stateCHANNEL3 = true;
    // на номер, с которого пришёл запрос,
    // отправляем смс с текстом о включении канала №3
    gprs.sendSMS(f_phone, CHANNEL3_ON);
  } 
  else if (strcmp(f_message, "can3off") == 0) 
  {
    // если пришло сообщение с текстом «can3off»,
    // выводим сообщение в Serial
    // и размыкаем реле
  //  Serial.println("OK! Channel 3 Off");
    digitalWrite(CHANNEL3, LOW);
   stateCHANNEL3 = false;
    // на номер, с которого пришёл запрос
    // отправляем смс с текстом о выключении канала №3
    gprs.sendSMS(f_phone, CHANNEL3_OFF);
  }
  
  else if (strcmp(f_message, "State") == 0) 
  {
    // если пришло сообщение с текстом «State»,
    // отправляем сообщение с состоянием реле
    if (stateRelay) 
    {
   //   Serial.println("State: Power On");
      gprs.sendSMS(f_phone, MESSAGE_ON);
    } else 
    {
     // Serial.println("State: Power Off");
      gprs.sendSMS(f_phone, MESSAGE_OFF);
    }
  } 
   else if (strcmp(f_message, "State1") == 0) 
  {
    // если пришло сообщение с текстом «Stat1e»,
    // отправляем сообщение с состоянием канала1
    if (stateCHANNEL1) 
    {
   //   Serial.println("State: Channel 1 On");
      gprs.sendSMS(f_phone, CHANNEL1_ON);
    } 
    else 
    {
    //  Serial.println("State: Channel 1 Off");
      gprs.sendSMS(f_phone,  CHANNEL1_OFF);
    }
  } 
   else if (strcmp(f_message, "State2") == 0) 
  {
    // если пришло сообщение с текстом «State2»,
    // отправляем сообщение с состоянием канала2
    if (stateCHANNEL2) 
    {
    //  Serial.println("State: Channel 2 On");
      gprs.sendSMS(f_phone,  CHANNEL2_ON);
    } 
    else 
    {
     // Serial.println("State: Channel 2 Off");
      gprs.sendSMS(f_phone,  CHANNEL2_OFF);
    }
  } 
   else if (strcmp(f_message, "State3") == 0) 
  {
    // если пришло сообщение с текстом «State3»,
    // отправляем сообщение с состоянием канала3
    if (stateCHANNEL3) 
    {
    //  Serial.println("State: Channel 3 On");
      gprs.sendSMS(f_phone,  CHANNEL3_ON);
    }
    else 
    {
    //  Serial.println("State: Channel 3 Off");
      gprs.sendSMS(f_phone,  CHANNEL3_OFF);
    }
  } 
 else if (strcmp(f_message, "Temp") == 0) 
  {
    // если пришло сообщение с текстом «Temp»,
    // выводим сообщение в Serial
    // и выводим состояние датчиков
    readSensors();   // Получить состояние датчиков
    tcpRequest();
    gprs.sendSMS(f_phone, tcpBuffer);
   // очищаем буфер
     clearTcpBuffer();

  //  Serial.println("OK! Channel 3 Off");
  }
  else 
  {
    // если сообщение содержит неизвестный текст,
    // отправляем сообщение с текстом об ошибке
   // Serial.println("Error... unknown command!");
    gprs.sendSMS(f_phone, MESSAGE_ERROR);
  }
}

// функция записи данных с датчиков в массив
// в специальном формате для «народного мониторинга»
void tcpRequest()
{
   /* помните, что при выполнении операций 
   с массивами символов, например strcat(str1, str2);
   контроль нарушения их границ не выполняется, 
   поэтому программист должен сам позаботиться
   о достаточном размере массива str1,
   позволяющем вместить как его исходное содержимое,
   так и содержимое массива str2
  */

  // добавляем к строке tcpBuffer строку CLIENT
  // strcat(tcpBuffer, CLIENT);
  // функция добавления в TCP-запрос значения температуры воздуха
  tcpTemp();
  // функция добавления в TCP-запрос значения влажности воздуха
  tcpHumi();
  // функция добавления в TCP-запрос значения влажности почвы
  tcpMoisture();
  // функция добавления в TCP-запрос состояния уровня CO2
  tcpGas();
  // функция добавления в TCP-запрос значения освещённости
  tcpLight();
}

// Функция добавление в TCP-запрос данные о температуре воздуха
void tcpTemp()
{
  // переменная для символьного представления
  // значения температуры воздуха
  char s_temp[8];
  // преобразуем целое число 10 системы исчисления
  // из переменной temp в строковое представление в массив s_temp[]
  itoa(temp, s_temp, 10);
  // добавляем к буферу символы «.00», для дробной части
  strcat(s_temp, ".00");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу уникальный серийный номера датчика
  // получаем его путём добавления к IMEI GPRS-шилда названия датчика
//  strcat(tcpBuffer, IMEI);
 // strcat(tcpBuffer, "T01");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
   // добавляем к буферу строку s_temp
  strcat(tcpBuffer, s_temp);
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу время актуальности показаний
  // если показания датчиков передаются немедленно,
  // то данный параметр передавать не надо
  strcat(tcpBuffer, " ");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу названия датчика
  strcat(tcpBuffer, "Temp ");
 // strcat(tcpBuffer, "\r\n");
}

// Функция добавление в TCP-запрос данные о влажности воздуха
void tcpHumi()
{
  // переменная для символьного представления
  // значения влажности воздуха
  char s_humi[8];
  // преобразуем целое число 10 системы исчисления
  // из переменной humi в строковое представление в массив s_humi[]
  itoa(humi, s_humi, 10);
  // добавляем к буферу символы «.00», для дробной части
  strcat(s_humi, ".00");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу уникальный серийный номера датчика
  // получаем его путём добавления к IMEI GPRS-шилда названия датчика
//  strcat(tcpBuffer, IMEI);
  strcat(tcpBuffer, "H01");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
   // добавляем к буферу строку s_humi
  strcat(tcpBuffer, s_humi);
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу время актуальности показаний
  // если показания датчиков передаются немедленно,
  // то данный параметр передавать не надо
  strcat(tcpBuffer, " ");
  strcat(tcpBuffer, "#");
  // добавляем к буферу названия датчика
  strcat(tcpBuffer, "Hum ");
  //strcat(tcpBuffer, "\r\n");
}

void  tcpMoisture()
{
  // переменная для символьного представления
  // значения влажности почвы
  char s_moisture[8];
  // преобразуем целое число 10 системы исчисления 
  // из переменной moisture в строковое представление в массив s_moisture[]
  itoa(moisture, s_moisture, 10);
  // добавляем к буферу символы «.00», для дробной части
  strcat(s_moisture, ".00");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу уникальный серийный номера датчика
  // получаем его путём добавления к IMEI GPRS-шилда названия датчика
//  strcat(tcpBuffer, IMEI);
  strcat(tcpBuffer, "H02");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
   // добавляем к буферу строку s_moisture
  strcat(tcpBuffer, s_moisture);
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу время актуальности показаний
  // если показания датчиков передаются немедленно,
  // то данный параметр передавать не надо
  strcat(tcpBuffer, " ");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу названия датчика
  strcat(tcpBuffer, "Hum ground");
 // strcat(tcpBuffer, "\r\n");
}

void  tcpGas()
{
  // переменная для символьного представления значения CO2
  char s_mq2[8];
  // преобразуем целое число 10 системы исчисления
  // из переменной mq2 в строковое представление в массив s_mq2[]
  itoa(mq2, s_mq2, 10);
  // добавляем к буферу символы «.00», для дробной части
  strcat(s_mq2, ".00");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу уникальный серийный номера датчика
  // получаем его путём добавления к IMEI GPRS-шилда названия датчика
//  strcat(tcpBuffer, IMEI);
  strcat(tcpBuffer, "MQ2");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
   // добавляем к буферу строку s_gas
  strcat(tcpBuffer, s_mq2);
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу время актуальности показаний
  // если показания датчиков передаются немедленно,
  // то данный параметр передавать не надо
  strcat(tcpBuffer, " ");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу названия датчика
  strcat(tcpBuffer, "CO2 ");
 // strcat(tcpBuffer, "\r\n");
}

void  tcpLight()
{
  // переменная для символьного представления значения освещённости
  char s_light[8];
  // преобразуем целое число 10 системы исчисления
  // из переменной light в строковое представление в массив s_light[]
  itoa(light, s_light, 10);
  // добавляем к буферу символы «.00», для дробной части
  strcat(s_light, ".00");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу уникальный серийный номера датчика
  // получаем его путём добавления к IMEI GPRS-шилда названия датчика
//  strcat(tcpBuffer, IMEI);
  strcat(tcpBuffer, "L01");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
   // добавляем к буферу строку s_light
  strcat(tcpBuffer, s_light);
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу время актуальности показаний
  // если показания датчиков передаются немедленно,
  // то данный параметр передавать не надо
  strcat(tcpBuffer, " ");
  // добавляем к буферу разделительный символ «#»
  strcat(tcpBuffer, "#");
  // добавляем к буферу названия датчика
  strcat(tcpBuffer, "Sun");
  //strcat(tcpBuffer, "\r\n");
}

// функция считывания показателей с датчиков
void readSensors()
{
  // считывание данных с датчика DHT11
  DHT.read(DHT11_PIN);
  // присваивание переменной temp значения температуры воздуха
  temp = DHT.temperature;
  // присваивание переменной humi значения влажности воздуха
  humi = DHT.humidity;
  // считывание значения с датчика влажности почвы
  moisture = analogRead(MOISTURE_PIN);
  // считывание значения с датчика уровня CO2
  mq2 = analogRead(MQ2_PIN);
  // считывание значения с датчика уровня освещённости
  light = analogRead(LIGHT_PIN);

  // преобразовываем аналоговое 10-битное значение
  // датчика влажности почвы в диапазон (от 0% до 100%)
  moisture = map(moisture, 0, 1023, 0, 100);
  // преобразовываем аналоговое 10-битное значение
  // датчика уровня CO2 в диапазон (от 0ppm до 8000ppm)
  mq2 = map(mq2, 0, 1023, 0, 8000);
  // преобразовываем аналоговое 10-битное значение
  // датчика уровня освещённости в диапазон (от 0Lx до 2000Lx)
  light = map(light, 0, 1023, 2000, 0);
}
void clearTcpBuffer()
{
  for (int t = 0; t < LEN; t++) {
    // очищаем буфер,
    // присваивая всем индексам массива значение 0
    tcpBuffer[t] = 0;
  }
}

// функция вывода значения датчиков в последовательный порт
void  serialPrint()
{
 // Serial.print("temp = ");
 // Serial.println(temp);
 // Serial.print("humi = ");
 //  Serial.println(humi);
 // Serial.print("moisture = ");
 // Serial.println(moisture);
 // Serial.print("CO2 = ");
 // Serial.println(mq2);
 // Serial.print("light = ");
 // Serial.println(light);
 // Serial.println("");
}










