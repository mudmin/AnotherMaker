// https://youtube.com/AnotherMaker
// Stream(ing) Deck simplified demo

//buttons are wired with one side of the button going to the
//digital pin and the other side going to ground
int blue    = 5;  //As in Digital Pin 5
int white   = 6;
int yellow  = 7;

void setup(){
  Serial.begin(9600);
  pinMode(blue, INPUT_PULLUP);
  pinMode(white, INPUT_PULLUP);
  pinMode(yellow, INPUT_PULLUP);
  Serial.println("Arduino has booted");
}

void loop(){
    if(digitalRead(blue) == LOW){
      Serial.println("blue=true");
    }

    if(digitalRead(white) == LOW){
      Serial.println("white=true");
    }

    if(digitalRead(yellow) == LOW){
      Serial.println("yellow=true");
    }
}
