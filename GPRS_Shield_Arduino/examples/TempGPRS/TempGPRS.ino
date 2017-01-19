// библиотека для работы с GPRS устройством
#include <GPRS_Shield_Arduino.h>

// библиотека для эмуляции Serial-порта
// она нужна для работы библиотеки GPRS_Shield_Arduino
#include <SoftwareSerial.h>

// библиотека для работы с датчиком DHT11
#include <dht11.h>

// даём разумное имя для пина к которому подключен датчик DHT11
#define DHT11_PIN 11
// даём разумное имя для пина к которому подключен датчик влажности почвы
#define MOISTURE_PIN A0
// даём разумное имя для пина к которому подключен датчик уровня CO2
#define MQ2_PIN A2
// даём разумное имя для пина к которому подключен датчик уровня освещённости
#define LIGHT_PIN A4

// IMEI GPRS Shield, он указан на лицевой стороне шилда
// !!! по IMEI устройство будет идентифицироваться в проекте
// !!! поэтому он должен быть уникальным
#define IMEI "868204005187692"

// часть запроса в специальном формате для народного мониторинга, содержащая:
// IMEI устройства, название фирмы и GPS-координаты
#define CLIENT "#"IMEI"#Амперка#55.7467#37.6627#2.0\r\n"

// интервал между отправками данных в миллисекундах (5 минут)
#define INTERVAL 300000

// размер массива, содержащий TCP-запрос
#define LEN 370
// буфер для отправки данных на народный мониторинг
// согласно установленной сервисом форме
char tcpBuffer[LEN];

// переменная для хранения времени работы программы
// с последнего запуска отправки данных на сервер
unsigned long previousMillis = 0;

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
// создаём объект класса GPRS и передаём ему скорость 9600 бод
// с помощью него будем давать команды GPRS-шилду
//GPRS gprs(9600);
GPRS gprs(_pkPin, _stPin, PIN_TX,PIN_RX,BAUDRATE);

void setup()
{
  // открываем последовательный порт для мониторинга действий в программе
  Serial.begin(9600);
}

void loop()
{
  // включаем GPRS-шилд
  gprs.powerUpDown();

  // проверяем есть ли связь с GPRS устройством
  while (!gprs.init()) {
    // если связи нет, выводим сообщение об ошибке и ждём 1 секунду
    // процесс повторяется в цикле
    // пока не появится ответ от GPRS устройства
    Serial.print("Init error\r\n");
    delay(1000);
  }
  // выводим сообщение об удачной синхронизации
  Serial.println("Init OK");
  delay(3000);

  // пытаемся установить GPRS-соединение
  // с заданными настройками, которые предоставляются операторами связи
  while (!gprs.join("internet.beeline.ru", "beeline", "beeline")) {
    // если GPRS-соединения нет
    // выводим сообщение об ошибке и ждём 1 секунду
    // процесс повторяется в цикле
    // пока не появится положительный ответ от GPRS устройства
  //  Serial.println("Gprs join network error");
    delay(1000);
  }
  // выводим сообщение об удачной установке GPRS-соединения
  Serial.println("GPRS OK");

  // получаем  и выводим локальный IP адрес
  Serial.print("IP Address is ");
  Serial.println(gprs.getIPAddress());

  // пытаемся подключиться к серверу
  // указывая тип соединения, адрес сервера и номер порта
  while (!gprs.connect(TCP, "narodmon.ru", 8283)) {
    // если сервер не отвечает или отвечает ошибкой
    // выводим сообщение об ошибке и ждём 1 секунду
    // процесс повторяется в цикле
    // пока не появится положительный ответ от сервера
      Serial.println("Connect error");
      delay(1000);
  }
  // выводим сообщение об удачном подключении к серверу
  Serial.println("Connect success");

  // вызываем функцию считывания всех показателей с датчиков
  readSensors();
  // выводим показания датчиков в последовательный порт
  serialPrint();
  // вызываем функцию, которая формирует и отправляет tcp-запрос
  // в специальном формате для «народного мониторинга»
  tcpRequest();
  // разрываем все GPRS-соединения
  gprs.close();
  // деактивируем интерфейс GPRS
  gprs.disconnect();
  // выводим сообщение об удачном завершении операции
  Serial.println("OK");
  // выключаем GPRS-шилд
  gprs.powerOff();

  // проверяем не прошел ли нужный интервал времени
  while (millis() - previousMillis < INTERVAL) {
  // ждём 5 минут
  }
  //если прошел, то сохраняем текущее время
  previousMillis = millis();
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
  strcat(tcpBuffer, CLIENT);
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

  // добавляем к строке tcpBuffer два символа «##»,
  // которые свидетельствуют об окончании запроса
  strcat(tcpBuffer, "##");

  // отправляем массив TCP-запроса на сервис «народного мониторинга»
  gprs.send(tcpBuffer);

  // очищаем буфер
  clearTcpBuffer();
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
  strcat(tcpBuffer, IMEI);
  strcat(tcpBuffer, "T01");
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
  strcat(tcpBuffer, "Температура");
  strcat(tcpBuffer, "\r\n");
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
  strcat(tcpBuffer, IMEI);
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
  strcat(tcpBuffer, "Влажность");
  strcat(tcpBuffer, "\r\n");
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
  strcat(tcpBuffer, IMEI);
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
  strcat(tcpBuffer, "Влажность почвы");
  strcat(tcpBuffer, "\r\n");
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
  strcat(tcpBuffer, IMEI);
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
  strcat(tcpBuffer, "Газ CO2");
  strcat(tcpBuffer, "\r\n");
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
  strcat(tcpBuffer, IMEI);
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
  strcat(tcpBuffer, "Освещение");
  strcat(tcpBuffer, "\r\n");
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
//  Serial.print("temp = ");
//  Serial.println(temp);
//  Serial.print("humi = ");
//  Serial.println(humi);
//  Serial.print("moisture = ");
//  Serial.println(moisture);
//  Serial.print("CO2 = ");
//  Serial.println(mq2);
//  Serial.print("light = ");
//  Serial.println(light);
//  Serial.println("");
}
