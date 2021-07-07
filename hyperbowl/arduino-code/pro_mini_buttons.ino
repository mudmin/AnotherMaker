// AnotherMaker.com HyperBowl Arduino Pro Micro code
// This really only requires two buttons
// left is connected to pin D2 and ground
// right is connected to pin D3 and ground
#include "Keyboard.h"

const int left = 2; //left button
const int right = 3; //right button


void setup() {
  pinMode(left,INPUT_PULLUP);
  pinMode(right,INPUT_PULLUP);
  Keyboard.begin();
}

void loop() {
  if(digitalRead(left) == LOW && digitalRead(right) == LOW){
    Keyboard.write(0xB0); //enter
    delay(150);
  }else if(digitalRead(left) == LOW){
    Keyboard.write(0xD8); //left arrow
    delay(150);
  }else if(digitalRead(right) == LOW){
    Keyboard.write(0xD7); //right arrow
    delay(150);
  }
  delay(100);
}
