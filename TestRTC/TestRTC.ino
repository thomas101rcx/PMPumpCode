#include <Wire.h>
#include "RTClib.h"
#include <SD.h>
#include <SPI.h>
#include <SD_t3.h>


//This code is to test RTC and micro-SD card reader / micro-SD card
//The output of this code will be  the current time on RTC (YYMMDD, HourMinuteSeconds) displayed on the serial monitor and writes into a file called RTC_SD_Checker.txt

RTC_DS3231 rtc;
String year, month, day, second, hour, minute;
File myFile;
String writeString;
const char * buffer = "HAFST.txt";
const int LEDPin =13;
int ledState = LOW;
unsigned long previousMillis = 0;
const long interval = 300;

void setup() {
  Serial.begin(9600);
  if (!rtc.begin()) {
    Serial.println("Can't fine RTC");
    while (1);
  }
  Wire.begin();
  rtc.begin();
  DateTime now = rtc.now();// Catch the time on RTC for now
  DateTime PCTime = DateTime(__DATE__, __TIME__); // Catch the time on PC for now

  //If any discrepencies , update with the time on  PC
  if (now.unixtime() < PCTime.unixtime()) {
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  if (SD.begin(10) == false) {
    Serial.println("It didn't initialized");
  }
  else {
    Serial.println("SD card initialized successfully");
  }

  //char datastr[100];

  year = String(now.year(), DEC);
  //Convert from Now.year() long to Decimal String object
  month = String(now.month(), DEC);
  day = String(now.day(), DEC);
  hour = String(now.hour(), DEC);
  minute = String(now.minute(), DEC);
  second = String(now.second(), DEC);
  String logHeader = year + "/" + month + "/" + day + " " + hour + ":" + minute + ":" + second;
  sdLog(buffer, "TSI Box 1: New Logging Session - " + logHeader);
  Serial.println(logHeader);
  int a = sdRead(buffer);
  Serial.println(a);

  pinMode(LEDPin, OUTPUT);
}

void loop() {
  DateTime now = rtc.now();
  year = String(now.year(), DEC);
  //Convert from Now.year() long to Decimal String object
  month = String(now.month(), DEC);
  day = String(now.day(), DEC);
  hour = String(now.hour(), DEC);
  minute = String(now.minute(), DEC);
  second = String(now.second(), DEC);

  writeString = year + "/" + month + "/" + day + " " + hour + ":" + minute + ":" + second + " ";

  sdLog(buffer, writeString);
  Serial.println(writeString);

  delay(1000);

}


void sdLog(const char * fileName, String stringToWrite) {
  File myFile = SD.open(fileName, FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to ");
    Serial.print(fileName);
    Serial.print("...");
    myFile.println(stringToWrite);
    // close the file:
    myFile.close();
    Serial.println("done.");
    digitalWrite(13, HIGH);
    delay(300);
    digitalWrite(13, LOW);
    delay(300);
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(fileName);
  }
}

int sdRead(const char *fileName){
  File myfile = SD.open(fileName);
  int timecount = 0 ;
  int timecountarray [20] = {0};
  if(myFile){
    while (myFile.available()){
    String line =  myFile.readStringUntil('\n');
    Serial.println(line);
    int spaceIndex = line.indexOf(' ');
    Serial.println(spaceIndex);
    // Search for the next space just after the first
    int secondspaceIndex = line.indexOf(' ', spaceIndex + 1);
    int thirdspaceIndex  = line.indexOf(' ', secondspaceIndex + 1 );
    String firstValue = line.substring(0, spaceIndex);
    String secondValue = line.substring(spaceIndex+1, secondspaceIndex);
    String thirdValue = line.substring(secondspaceIndex+1, thirdspaceIndex); // To the end of the string
    String fourthValue = line.substring(thirdspaceIndex);
    timecount = fourthValue.toInt();
    timecountarray[0] = timecount;
    }
    myFile.close();
   }
   return timecountarray[0];
 }

//https://learn.adafruit.com/ds1307-real-time-clock-breakout-board-kit/understanding-the-code
