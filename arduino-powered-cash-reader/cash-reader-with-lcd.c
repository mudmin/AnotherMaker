#include <LiquidCrystal.h>

const int pin_RS = 8;
const int pin_EN = 9;
const int pin_d4 = 4;
const int pin_d5 = 5;
const int pin_d6 = 6;
const int pin_d7 = 7;
const int pin_BL = 10;
LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);
const int cash = 15; //connect blue wire to pin 15, purple to ground
int pulse;
int dollars = 0;
int counter = 0;


void setup() {
  Serial.begin(115200);
  pinMode(cash, INPUT_PULLUP);
  Serial.println("Cash machine booted");
 lcd.begin(16, 2);
 lcd.setCursor(0,0);
 lcd.print("Booted");
 delay(1000);
}
void loop() {
     lcd.setCursor(0,0);
     lcd.print("Ready!        ");
     pulse = digitalRead(cash);

    if(pulse == 0){
      counter++;
      if(counter > 8000){
       lcd.setCursor(0,0);
       lcd.print("Bill Accepted!");
       dollars = dollars + 1;
       lcd.setCursor(0,1);
       lcd.print("$");
       lcd.print(dollars);
       lcd.print(" total");
       delay(1000);
        counter = 0;
      }
    }

    if(pulse == 1 && counter > 0){
      Serial.println(counter);
      counter = 0;
    }
}
