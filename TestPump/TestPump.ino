#include <Wire.h> 
#include <RTClib.h>
#include <SD.h>

RTC_DS3231 RTC;



//The purpose of this code is to test if the Pump works as expected or not. When you run this code, both of the pumps will start from 0% duty cycle (fully off)  until 100 % duty cycle (completely on). 
//What that means is that both pumps will start from the smallest flow rate 0.0 LPM to its maximum flow rate depending on the Pump's characteristic ( usually 2 LPM).
//You should expect an increased of pump noise level.

float a = 0;

float TARGET_FLOW = 4.0;
float avgFlow = 0;

void setup() {
  Serial.begin(9600);
  pinMode(A6, OUTPUT);
  Wire.begin();
  RTC.begin();
}

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

  //This is for 0 -10 SLPM Flow sensor
  //curFlow = 2.5*(1.5*Vo - 1.24);
  //This is for 0 -5 SLPM Flow sensor
  curFlow = 1.25*(1.5*Vo - 1.24);
  avgFlow += (curFlow - avgFlow) / 50000;

}


void loop() {

  static float pwmhigh = 0.5; // For 0.6 LPM
  //The reason to set pwmhigh and pwmlow is to start with a initial value for the feedback loop to either add up the error or subtract the error, reaching the desire power output -> desire flowrate.
  
  for(uint32_t i = 0; i < 1000; i++)
  {
    delayMicroseconds(10);
    Return_Flow_Rate();
  }
  Serial.println(avgFlow);

  if (millis() >= 0) {
    float errorHigh = TARGET_FLOW - avgFlow;
    pwmhigh = max(pwmhigh, 0); // For pwmhigh < 0
    pwmhigh = min(1, pwmhigh); // For pwmhigh > 1
    pwmhigh += errorHigh / 100;
    // 100 is a time constant that tells us how precise do we want to get to the desire flow rate/ power output.
    writePumpA(pwmhigh);
  }


}
