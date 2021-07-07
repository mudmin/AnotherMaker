//Button Grid Button Test by AnotherMaker
//youtube.com/AnotherMaker
//AnotherMaker.com

//This is for an Arduino mega, but obviously you can change your pin numbers for other boards

void setup() {
  for (int i = 3; i < 54; i++) {
    pinMode(i,INPUT_PULLUP);
  }
  Serial.begin(9600);
  Serial.println("Beginning Button Test");
  Serial.println("Press a button and look for a message.");
}

void loop(){
  for (int i = 3; i < 54; i++) {
    if(digitalRead(i) == LOW){
      Serial.print(i);
      Serial.println(" pressed!");
      delay(300);
    }
  }
}
