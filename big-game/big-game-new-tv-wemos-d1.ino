//Another Maker
//AnotherMaker.com
//Youtube.com/AnotherMaker

//You will need the IRremoteESP8266 Library from the Arduino IDE
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

//Pin Definitions
const uint16_t kIrLed = 4;  // ESP8266 GPIO pin D2
const int connected = LED_BUILTIN; //D4,GPIO2
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.
int rn = 0;

void setup() {
irsend.begin();
Serial.begin(115200);
pinMode(connected,OUTPUT);
digitalWrite(connected,HIGH);
Serial.println("Your KEY to a new TV (c) 2021 AnotherMaker");
}

void loop() {
  rn = random(0,6);
  if(rn == 0){
    irsend.sendNEC(0x20DFC03F, 32);
    Serial.println("volume down");
  }else if(rn == 1){
    irsend.sendNEC(0x20DF40BF, 32);
    Serial.println("volume up");
  }else if(rn == 2){
    irsend.sendNEC(0x20DF00FF, 32);
    Serial.println("channel up");
  }else if(rn == 3){
    irsend.sendNEC(0xFFFFFFFF, 32);
    Serial.println("channel down");
  }else if(rn == 4){
    irsend.sendNEC(0x20DF906F, 32);
    Serial.println("mute");
  }else if(rn == 5){
    irsend.sendNEC(0x20DFF40B, 32);
    Serial.println("input");
  }
  delay(random(1000,3000));
}
