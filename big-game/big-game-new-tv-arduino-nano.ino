//Another Maker
//AnotherMaker.com
//Youtube.com/AnotherMaker

//The loop is just an example. Do whatever you think
//is fun.  Use it as an experiment to play with randomness
//and maybe convert it to a switch statement.

//You will need the IR Remote Library from the Arduino IDE
#include <IRremote.h>
int rn = 0;
IRsend irsend;

void setup()
{
  Serial.begin(115200);
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
