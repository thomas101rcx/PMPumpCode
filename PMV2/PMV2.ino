#include <Wire.h> 
#include <SPI.h> //for SD card
#include "RTClib.h"
#include <SD.h> 
#include <stdint.h> 
#include <SD_t3.h>
 

float TARGET_FLOW = 4.0;
float avgFlow = 0;
static uint32_t counter = 1;
static uint32_t restartcounter = 0;
int timecounter = 0;
int runningtime = 0;
int timeleft = 0;
bool restart = true;
String year, month, day, second, hour, minute;
String writeString;
File myFile;
const char * buffer = "Flowrate.txt";
RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  pinMode(A6, OUTPUT);
  SPI.begin();
  delay(1000); // need this for print statement within setup 
  //RTC setup

  if (!rtc.begin()) {
    Serial.println("Can't fine RTC");
    while (1);

  } else {
    Serial.println("RTC initialized successfully");
  }

  DateTime now = rtc.now(); // Catch the time on RTC for now
  DateTime PCTime = DateTime(__DATE__, __TIME__); // Catch the time on PC for now

  // If any discrepencies , update with the time on PC 
  // Manually change this code when the timezone is different uncomment the rtc.adjust(DateTime(__DATE__, __TIME__));
  // Upload it again to Arduino and check if the time is correct
  // Comment out rtc.adjust(DateTime(__DATE__, __TIME__)); lastly, Upload the entire code again
  if (now.unixtime() < PCTime.unixtime()) {
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  Wire.begin();
  rtc.begin();

  //Pump setup


  //SD card setup
  if (SD.begin(10) == false) {
    Serial.println("It didn't initialized");
  } else {
    Serial.println("SD card Initialized successfully");
  }

  //Check if the power has been cut off for the past 48 hours

  //This is to read the last counter value from previous starting point
  //int a = sdRead(buffer);
  //Serial.println(a);

  if (sdRead(buffer) == 0) {
    //When we can start a new Trial , writes in The inital starting time (Tstart) to SD card
    year = String(now.year(), DEC);
    month = String(now.month(), DEC);
    day = String(now.day(), DEC);
    hour = String(now.hour(), DEC);
    minute = String(now.minute(), DEC);
    second = String(now.second(), DEC);
    String logHeader = year + "/" + month + "/" + day + " " + hour + ":" + minute + ":" + second;
    sdLog(buffer, "FlowRate_4.0: New Logging Session - " + logHeader);
    Serial.println(logHeader);
    restart = false;
  } else {
    //Read from previous trial that doesn't last 48 hours, start from the point and continue until 48 hours
    runningtime = sdRead(buffer);
    timeleft = 2880 - runningtime;
    restartcounter = runningtime;
    restart = true;
  }
}

//Writes to pump A, takes a float from 0 to 1

void writePumpA(float p) {
  p = max(p, 0);
  p = min(1, p);
  uint8_t power = p * 255;
  analogWrite(A6, power);
}

void Return_Flow_Rate() {
  float curFlow = 0;
  uint16_t sensorvalue = 0;
  sensorvalue = analogRead(A3);
  float Vo = sensorvalue * (3.3 / 1023.0);

  //This transfer function is for 0 - 5 SLPM flow sensor
  curFlow = 1.25 * (1.5 * Vo - 1.22);
  avgFlow += (curFlow - avgFlow) / 50000;
  //Serial.println(avgFlow);
}

//Writes into SD card
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

//Read the last tiemcounter from SD card when power is shut off
int sdRead(const char * fileName) {
  File myfile = SD.open(fileName);
  int timecount = 0;
  int timecountarray[20] = {0};
  if (myfile) {
    while (myfile.available()) {
      String line = myfile.readStringUntil('\n');
      int spaceIndex = line.indexOf(' ');
      // Search for the next space just after the first
      int secondspaceIndex = line.indexOf(' ', spaceIndex + 1);
      int thirdspaceIndex = line.indexOf(' ', secondspaceIndex + 1);
      String firstValue = line.substring(0, spaceIndex);
      String secondValue = line.substring(spaceIndex + 1, secondspaceIndex);
      String thirdValue = line.substring(secondspaceIndex + 1, thirdspaceIndex); // To the end of the string
      String fourthValue = line.substring(thirdspaceIndex);
      timecount = fourthValue.toInt();
      timecountarray[0] = timecount;
    }
    myfile.close();
  }
  int result = timecountarray[0];
  return result;
}

void loop() {

  static float pwmhigh = 0.5; // For 0.6 LPM
  //The reason to set pwmhigh and pwmlow is to start with a initial value for the feedback loop to either add up the error or subtract the error, reaching the desire power output -> desire flowrate.
  for (uint32_t i = 0; i < 1000; i++) {
    delayMicroseconds(100);
    Return_Flow_Rate();
  }
  Serial.println(avgFlow);

  if (millis() >= 0) {
    float errorHigh = TARGET_FLOW - avgFlow;
    pwmhigh += errorHigh / 100;
    
    pwmhigh = max(pwmhigh, 0); // For pwmhigh < 0
    pwmhigh = min(1, pwmhigh); // For pwmhigh > 1
    
    //Serial.println(errorHigh);
    // 100 is a time constant that tells us how precise do we want to get to the desire flow rate/ power output.
    Serial.println(pwmhigh);
    writePumpA(pwmhigh);
  }

  //Every minute log the data into SD card , "Time + Flowrate + Counter" for desire time,  ex: 48 hours
  
  if (restart == false && millis() % 60000 == 0 && avgFlow >= 0.2) {
    DateTime now = rtc.now();
    year = String(now.year(), DEC);
    //Convert from Now.year() long to Decimal String object
    month = String(now.month(), DEC);
    day = String(now.day(), DEC);
    hour = String(now.hour(), DEC);
    minute = String(now.minute(), DEC);
    second = String(now.second(), DEC);
    writeString = year + "/" + month + "/" + day + " " + hour + ":" + minute + ":" + second + " ";
    sdLog(buffer, writeString + avgFlow + " " + counter);
    Serial.println(writeString);
    counter++;
  }

  // run from when the power shut off

  if (restart == true && millis() % 60000 == 0 && avgFlow >= 0.2) {
    DateTime now = rtc.now();
    year = String(now.year(), DEC);
    //Convert from Now.year() long to Decimal String object
    month = String(now.month(), DEC);
    day = String(now.day(), DEC);
    hour = String(now.hour(), DEC);
    minute = String(now.minute(), DEC);
    second = String(now.second(), DEC);
    writeString = year + "/" + month + "/" + day + " " + hour + ":" + minute + ":" + second + " ";
    sdLog(buffer, writeString + avgFlow + " " + restartcounter);
    Serial.println(writeString);
    restartcounter++;
  }

  //Restart counter will stop once it reaches 48 hours
  
  if (restartcounter > 2880) {
    writePumpA(0);
  }

  //Turn off pump once it reach 48 hours

  if (counter > 2880) {
    writePumpA(0);
  }
}
