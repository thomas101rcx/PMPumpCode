#include <Wire.h>
float avgFlowlow = 0;
float avgFlowhigh = 0;

//The purpose of this code is two test the two flow sensors on PCB. It will have four outputs on the serial monitor of a given time. 
// Flow meter for high flow rate 0.6 LPM aldahyde
//1.Raw data frim analogread 
//2.transferred value from raw data which corresponds to flow rate 
// Flow meter for low flow rate 0.2 LPM sorbent 
//3.Raw data frim analogread 
//4.transferred value from raw data which corresponds to flow rate 


void setup() {
  Serial.begin(9600);
}


void Return_Low_Flow_Rate() {
  float curFlow = 0;
  uint16_t sensorvalue = 0;
  sensorvalue = analogRead(A4);
  Serial.println(sensorvalue);
 // Serial.println("The analog output for lowflow  is " + sensorvalue );
  float Vo = sensorvalue * (3.3 / 1023.0);
  curFlow = 2*Vo - 2;
  avgFlowlow += (curFlow - avgFlowlow) / 32;
 // Serial.print("The current average Flow recorded from the lowflow sensor is ");
 //Serial.println(avgFlowlow);
}


void Return_High_Flow_Rate() {
  float curFlow = 0;
  uint16_t sensorvalue = 0;
  sensorvalue = analogRead(A6);
  Serial.println("The analog output for highflow is "+ sensorvalue);
  float Vo = sensorvalue * (5.0 / 1023.0);
  Serial.println(sensorvalue);
  curFlow = 0.75 * (((Vo / 5) - 0.5) / 0.4);
  avgFlowhigh += (curFlow - avgFlowhigh) / 32;
  Serial.print("The current average Flow recorded from the highflow sensor is ");
  Serial.println(avgFlowhigh);
}

void loop(){
  Return_Low_Flow_Rate();
  //Return_High_Flow_Rate();
}
