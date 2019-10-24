#include <Arduino.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
//#include "User_Setup.h"

// Definición de pines
#define SD_SDI_BUS_1_PIN 11
#define SD_SDI_BUS_2_PIN 12
#define SD_SDI_BUS_3_PIN 13
#define SD_SS_PIN 9

//#define RTC_SCL_PIN A5
//#define RTC_SDA_PIN A4

#define SENSOR_PIN A0

#define LED_BAR_1 2
#define LED_BAR_2 3
#define LED_BAR_3 4
#define LED_BAR_4 5
#define LED_BAR_5 6
#define LED_BAR_6 7
#define LED_BAR_7 8
#define LED_BAR_8 10
#define LED_BAR_9 A3
#define LED_BAR_10 A2
#define LED_BAR_HELPER_PIN A1

//Definición de especificaciones
#define VERSION "1.0.2"
#define SERIAL_BAUDRATE 9600
#define FILE_NAME "sensor.csv"
#define WAIT_TIME_MS 5000

int bar[10] = {
        LED_BAR_1,
        LED_BAR_2,
        LED_BAR_3,
        LED_BAR_4,
        LED_BAR_5,
        LED_BAR_6,
        LED_BAR_7,
        LED_BAR_8,
        LED_BAR_9,
        LED_BAR_10
};

DateTime dt_rtc_sync;
RTC_DS3231 rtc;

/************* UTILITES **************/
void printToBar(int value){
    int x = 0;
    for(int i : bar){
        if (x < value){
            digitalWrite(i, LOW);
            x++;
            continue;
        }

        digitalWrite(i, HIGH);
    }
}

void printStartSequence(){
    printToBar(0);

    for(int c = 0; c <= 8; c++) {
        digitalWrite(bar[c], LOW);
        delay(100);
    }

    printToBar(10);

    for(int c = 0; c <= 8; c++) {
        digitalWrite(bar[c], HIGH);
        delay(100);
    }

    printToBar(0);
    delay(500);
    printToBar(10);
    delay(500);
    printToBar(0);
    delay(500);
}

void failureSequence_Blocking(){
    while(true){
        printToBar(10);
        delay(1000);
        printToBar(0);
        delay(1000);
    }
}

void printToSD(int val) {
    File file = SD.open(FILE_NAME, FILE_WRITE);
    if (!file){
        Serial.println("ERROR: No se pudo abrir el archivo");
        failureSequence_Blocking();
    }
    dt_rtc_sync = rtc.now();

    //Fecha
    if(dt_rtc_sync.day() < 10){
        file.print("0");
    }
    file.print(dt_rtc_sync.day());
    file.print("/");
    if(dt_rtc_sync.month() < 10){
        file.print("0");
    }
    file.print(dt_rtc_sync.month());
    file.print("/");
    if(dt_rtc_sync.year() < 10){
        file.print("0");
    }
    file.print(dt_rtc_sync.year());
    file.print(",");

    //Hora
    if(dt_rtc_sync.hour() < 10){
        file.print("0");
    }
    file.print(dt_rtc_sync.hour());
    file.print(":");
    if(dt_rtc_sync.minute() < 10){
        file.print("0");
    }
    file.print(dt_rtc_sync.minute());
    file.print(":");
    if(dt_rtc_sync.second() < 10){
        file.print("0");
    }
    file.print(dt_rtc_sync.second());
    file.print(",");

    //Temperatura
    file.print(rtc.getTemperature());
    file.print(",");

    //Valor
    file.println(val);
    file.close();
}

/*************************************/

/*************** SETUP ***************/
void setup() {

    //Configuración de pines
    /*pinMode(SD_SDI_BUS_1_PIN, INPUT);
    pinMode(SD_SDI_BUS_2_PIN, OUTPUT);
    pinMode(SD_SDI_BUS_3_PIN, OUTPUT);
    pinMode(SD_SS_PIN, OUTPUT);*/

    pinMode(SENSOR_PIN, INPUT);

    pinMode(LED_BAR_HELPER_PIN, OUTPUT);
    digitalWrite(LED_BAR_HELPER_PIN, HIGH);

    for(int i : bar){
        pinMode(i, OUTPUT);
    }

    //Iniciamos la Serial
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println("Sernsor de calidad del aire - Demo");
    Serial.print("Versión: ");
    Serial.println(VERSION);

    printStartSequence();

    Serial.println();
    Serial.println("Iniciando...");
    Serial.println("/********************************/");
    Serial.println();

    //Iniciamos conexión con el modulo SD
    Serial.println("Conectando con el módulo SD..");

    if (!SD.begin(SD_SS_PIN)){
        Serial.println("ERROR: No se pudo establecer una conexión con el módulo SD");
        failureSequence_Blocking();
    }

    else {
        Serial.println("Error al abrir el archivo");
    }

    Serial.println("SD CONN AOK");

    Serial.println("Preparando SD para escribir datos..");

    if(!SD.exists(FILE_NAME)){
        File file = SD.open(FILE_NAME, FILE_WRITE);
        if (!file){
            Serial.println("ERROR: No se pudo abrir el archivo");
            failureSequence_Blocking();
        }

        file.println("Fecha, Hora, Temperatura, Valor");
        file.close();
    }

    Serial.println("SD FILE AOK");

    Serial.println("Conectando con reloj de tiempo real..");

    if (!rtc.begin()){
        Serial.println("ERROR: No se pudo establecer una conexión con el módulo RTC");
        failureSequence_Blocking();
    }

    dt_rtc_sync = RTC_DS3231::now();

    if (dt_rtc_sync.year() < 2019){
        Serial.print("ERROR: RTC no ha sido configurado correctamente. Año reportado: ");
        Serial.println(dt_rtc_sync.year());
        failureSequence_Blocking();
    }

    Serial.print("Fecha: ");
    Serial.print(dt_rtc_sync.day());
    Serial.print("/");
    Serial.print(dt_rtc_sync.month());
    Serial.print("/");
    Serial.print(dt_rtc_sync.year());
    Serial.print(" ");

    Serial.print(" Hora: ");
    Serial.print(dt_rtc_sync.hour());
    Serial.print(":");
    Serial.println(dt_rtc_sync.minute());

    Serial.println("RTC AOK");
    Serial.println();
    Serial.println("Inicio exitoso, comenzando");
    Serial.println("/********************************/");
}
/*************************************/

/*************** LOOP ****************/
void loop() {
    int sensor_value = analogRead(SENSOR_PIN);
    int sensor_bar_map = int(map(sensor_value, 0, 150, 1, 10));

    Serial.print("Valor sensor: ");
    Serial.print(sensor_value);
    Serial.print(", ajustado: ");
    Serial.print(sensor_bar_map);
    Serial.print(", temperatura: ");
    Serial.println(rtc.getTemperature());

    printToBar(sensor_bar_map);
    printToSD(sensor_value);

    delay(WAIT_TIME_MS);
}
/*************************************/

