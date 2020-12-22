//Stayin Alive is 104 BPM
//60,000 / 104 BPM means that we need a "beat" every 577ms -ish

#include <LiquidCrystal.h>
int interval = 500; //how long to delay in millis
unsigned long previous = 0;
int state = LOW;
int counter = 0; //I'm sure I'll use this for something
int led = 48;

//LCD pin to Arduino
const int pin_RS = 8;
const int pin_EN = 9;
const int pin_d4 = 4;
const int pin_d5 = 5;
const int pin_d6 = 6;
const int pin_d7 = 7;
const int pin_BL = 10;
LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);
void setup() {
 lcd.begin(16, 2);
 lcd.setCursor(0,0);
 lcd.print("Device");
 lcd.setCursor(0,1);
 lcd.print("Booted");
 delay(2000); //Suspense!
 lcd.clear();
 digitalWrite(led,HIGH);
}

void loop() {
//I want to count the number of times through the 577ms loop so I can keep these two in as close to sync as possible

  unsigned long currentMillis = millis();

  if(currentMillis - previous >= interval){
    //save this reading!
    previous = currentMillis;
    counter++;
    if(counter == 1){
      lcd.setCursor(0,0);
      lcd.print("Well");
    }else if(counter == 2){
      lcd.setCursor(0,0);
      lcd.print("Well you");
    }else if(counter == 3){
      lcd.setCursor(0,0);
      lcd.print("Well you can");
    }else if(counter == 4){
      lcd.setCursor(0,1);
      lcd.print("tell");
    }else if(counter == 5){
      lcd.setCursor(0,1);
      lcd.print("tell by");
    }else if(counter == 6){
      lcd.setCursor(0,1);
      lcd.print("tell by the");
    }

    if(counter >= 12){
      counter = 0;
      lcd.clear();
    }
}
}
