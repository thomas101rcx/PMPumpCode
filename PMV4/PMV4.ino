#include <Wire.h> 
#include <SPI.h> //for SD card
#include "RTClib.h"
#include <SD.h> 
#include <stdint.h> 
#include <SD_t3.h> 
#include <SoftwareSerial.h>

const int TxPin = 1;
float TARGET_FLOW = 4.0;
float avgFlow = 0;
static uint32_t counter = 1;
static uint32_t restartcounter = 0;
int timecounter = 0;
int runningtime = 0;
int timeleft = 0;
unsigned long lastTime;
double errSum, lastErr;
bool restart = true;
String year, month, day, second, hour, minute;
String writeString;
File myFile;
const char * buffer = "Flowrate.txt";
RTC_DS3231 rtc;
SoftwareSerial LCD = SoftwareSerial(255, TxPin);
void setup() {
  //Set up Serial port for LCD and USB
  Serial.begin(9600);
  pinMode(A7, OUTPUT);
  pinMode(TxPin, OUTPUT);
  pinMode(A2, INPUT);
  pinMode(A1, INPUT);
  digitalWrite(TxPin, HIGH);
  LCD.begin(9600);
  delay(1000); // need this for print statement within setup 
  LCD.write(17);

  //RTC setup
  if (!rtc.begin()) {
    Serial.println("Can't fine RTC");
    //Clear screen
    LCD.write(12);
    delay(5);
    //Print out statement, display for 2 seconds
    LCD.print("Can't find RTC");
    LCD.write(13);
    delay(2000);
    //Clear screen
    LCD.write(12);
    while (1);
  } else {
    Serial.println("RTC initialized successfully");
    LCD.write(12);
    delay(5);
    LCD.print("RTC initialized successfully");
    LCD.write(13);
    delay(2000);
    LCD.write(12);
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

  SPI.begin();
  //SD card setup
  if (SD.begin(10) == false) {
    Serial.println("It didn't initialized");
    LCD.write(12);
    delay(5);
    LCD.print("SD card didn't initialized");
    LCD.write(13);
    delay(2000);
    LCD.write(12);
    delay(5);
  } else {
    Serial.println("SD card initialized successfully");
    LCD.write(12);
    delay(5);
    LCD.print("SD card initialized sucessfully");
    LCD.write(13);
    delay(2000);
    LCD.write(12);
    delay(5);
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
    timeleft = 2880 - runningtime ;
    restartcounter = runningtime + 1;
    restart = true;
  }
}

//Writes to pump A, takes a float from 0 to 1

void writePumpA(float p) {
  p = max(p, 0);
  p = min(1, p);
  uint8_t power = p * 255;
  analogWrite(A7, power);
}

void Return_Flow_Rate() {
  float curFlow = 0;
  uint16_t sensorvalue = 0;
  sensorvalue = analogRead(A3);
  float Vo = sensorvalue * (3.3 / 1023.0);

  //This transfer function is for 0 - 5 SLPM flow sensor

  curFlow = 1.25 * (1.5 * Vo - 1.22);
  avgFlow += (curFlow - avgFlow) / 10000;

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

//Read the last tiemcounter from SD card when power shuts off

int sdRead(const char * fileName) {
  File myfile = SD.open(fileName);
  int timecount = 0;
  int timecountarray[20] = {
    0
  };
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

//Timer for SD logging
elapsedMillis sinceStartup;

void loop() {

  static float pwmhigh = 0.5; // For 4.0 LPM

  if(digitalRead(A1) == HIGH){
     TARGET_FLOW += 0.1;
  }
  if(digitalRead(A2) == HIGH){
     TARGET_FLOW -= 0.1;  
  }

  for (uint32_t i = 0; i < 1000; i++) {
    delayMicroseconds(3);
    Return_Flow_Rate();
  }

  Serial.println(avgFlow);


  float error = TARGET_FLOW - avgFlow;
  errSum += (error * 0.032);

  if (errSum >= 3.0) {
    errSum = 0.95;
  }

  double dErr = (error - lastErr) / 200;

  pwmhigh = 0.95 * error + 0.95 * errSum + 1.1 * dErr;
  pwmhigh = max(pwmhigh, 0); // For pwmhigh < 0
  pwmhigh = min(1, pwmhigh); // For pwmhigh > 1

  lastErr = error;
  //Serial.println(pwmhigh);
  writePumpA(pwmhigh);

  float displayFlow = avgFlow;
  //Display flow rate on LCD screen

  LCD.write(12);
  delay(5);
  LCD.print("Flow rate ");
  LCD.print((float) displayFlow, 3);
  LCD.write(13);
  delay(100);
  LCD.write(12);
  delay(5);

  //Every minute log the data into SD card , "Time + Flowrate + Counter" for desire time,  ex: 48 hours

  if (restart == false && sinceStartup >= 60000 && avgFlow >= 1.0) {
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
    LCD.write(12);
    delay(5);
    LCD.print((int) counter, 10);
    LCD.write(13);
    delay(1000);
    LCD.write(12);
  }

  // run from when the power shuts off

  if (restart == true && sinceStartup >= 60000 && avgFlow >= 1.0 ) {
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
    LCD.write(12);
    delay(5);
    LCD.print((int) restartcounter, 10);
    LCD.write(13);
    delay(1000);
    LCD.write(12);
  }

  //Every 60 seconds, restart Timer

  if (sinceStartup >= 60000) {
    sinceStartup = 0;
  }

  //Restart counter will stop once it reaches 48 hours, Turn off pump as well

  if (restartcounter > 2880 && counter > 2880) {
    pwmhigh = 0;
    writePumpA(0);
  }


}
