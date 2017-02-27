#include <Wire.h>
#include "RTClib.h"
#include <SD.h>


RTC_DS3231 rtc;
File myFile;
const char *buffer = "HIGHFLOW.txt";

void setup() {  
 Serial.begin(9600);
  Wire.begin();
  rtc.begin();
  if (SD.begin(10) == false) {
    Serial.println("It didn't initialized");
  }
  else {
    Serial.println("SD card initialized successfully");
  }
  DateTime now = rtc.now();
  Serial.print(now.year());
  Serial.print(now.month());
  Serial.print(now.day());
  Serial.print(now.hour());
  Serial.print(now.minute());
  Serial.println(now.second());
  int a = sdRead(buffer);
  Serial.println(a);

}

void loop() {
  // put your main code here, to run repeatedly:

}

int sdRead(const char *fileName){
  File myfile = SD.open(fileName);
  int timecount = 0 ;
  int timecountarray [20];
  if(myfile){
    while (myfile.available()){
    String line =  myfile.readStringUntil('\n');
    int spaceIndex = line.indexOf(' ');
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
    myfile.close();
   }
   return timecountarray[0];
 }
