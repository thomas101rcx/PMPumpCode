

/* 
   NOTE: you must: #include <NewSoftSerial.h>
  BEFORE including the ParallaxLCD.h header
 */

#include "ParallaxLCD.h"

ParallaxLCD lcd(2,2,16); // desired pin, rows, cols

void setup () {

  lcd.setup();
  delay(1000);
  lcd.backLightOn();
  lcd.at(1,4,"Milliseconds\0");
  delay(1000);
  lcd.off();
  delay(1000);
  lcd.on();
  lcd.pos(0,1);
  lcd.at(2,7,"m:\0");
  for (int b=0; b<101; b+=5) {
    lcd.pos(0,3);
    lcd.at(2,9,millis());
    delay(500);
  }
  lcd.empty();
  lcd.print("Line Feed\0");
  for (int b=0; b<51; b+=5) {
    lcd.lf();
    delay(500);
  }
  lcd.empty();
  lcd.print("Chr Return\0");
  delay(1000);
  for (int b=0; b<51; b+=5) {
    lcd.pos(0,11);
    delay(500);
    lcd.cr();
    delay(500);
  }

  lcd.cursorBlock();
  lcd.empty();
  lcd.print("Block Cursor\0");
  for (int x=1; x<16; x++) {
    lcd.pos(1,x);
    delay(500);
  }
  lcd.empty();
  lcd.pos(1,1);
  lcd.cursorUnderline();
  for (int x=1; x<16; x++) {

    lcd.pos(1,x);
    delay(500);
  }
  lcd.backLightOff();
  lcd.cursorOff();
}

void loop () {
  lcd.at(2,9,millis());
}



