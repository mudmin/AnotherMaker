//This is in response of how to make these lights have a 1 second interval but 500 ms apart.
//There are a lot of ways to do this, but this is a quick and dirty way.
//basically what I'm doing is starting the left one a half second earlier and then changing the
//interval after the first time it goes through the loop.

int left = 2;
int intervalLeft = 500; //how long to delay in millis
unsigned long previousLeft = 0;
int leftState = LOW;

int right = 3;
int intervalRight = 1000; //how long to delay in millis
unsigned long previousRight = 0;
int rightState = LOW;


void setup() {
  Serial.begin(9600);
  pinMode(left, OUTPUT);
  pinMode(right, OUTPUT);
}

void checkLeft(){
  unsigned long currentMillis = millis();

  if(currentMillis - previousLeft >= intervalLeft){
    //save this reading!
    previousLeft = currentMillis;

    //figure out if you should turn the LED on or off
    if(leftState == LOW){
      leftState = HIGH;
      intervalLeft = 1000; //fix the offset I created
    }else{
      leftState = LOW;
    }
    digitalWrite(left,leftState);
  }
}

void checkRight(){
  unsigned long currentMillis = millis();

  if(currentMillis - previousRight >= intervalRight){
    //save this reading!
    previousRight= currentMillis;

    //figure out if you should turn the LED on or off
    if(rightState == LOW){
      rightState = HIGH;
    }else{
      rightState = LOW;
    }
    digitalWrite(right,rightState);
  }
}


void loop() {
  checkLeft();
  checkRight();
}
