int red = 2;
int intervalRed = 1000; //how long to delay in millis
unsigned long previousRed = 0;
int redState = LOW;

int blue = 3;
int intervalBlue = 2500; //how long to delay in millis
unsigned long previousBlue = 0;
int blueState = LOW;

int green = 4;
int intervalGreen = 5000; //how long to delay in millis
unsigned long previousGreen = 0;
int greenState = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(red, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(green, OUTPUT);
}

void checkRed(){
  unsigned long currentMillis = millis();

  if(currentMillis - previousRed >= intervalRed){
    //save this reading!
    previousRed = currentMillis;

    //figure out if you should turn the LED on or off
    if(redState == LOW){
      redState = HIGH;
    }else{
      redState = LOW;
    }
    digitalWrite(red,redState);
  }
}

void checkGreen(){
  unsigned long currentMillis = millis();

  if(currentMillis - previousGreen >= intervalGreen){
    //save this reading!
    previousGreen= currentMillis;

    //figure out if you should turn the LED on or off
    if(greenState == LOW){
      greenState = HIGH;
    }else{
      greenState = LOW;
    }
    digitalWrite(green,greenState);
  }
}

void checkBlue(){
  unsigned long currentMillis = millis();

  if(currentMillis - previousBlue >= intervalBlue){
    //save this reading!
    previousBlue= currentMillis;

    //figure out if you should turn the LED on or off
    if(blueState == LOW){
      blueState = HIGH;
    }else{
      blueState = LOW;
    }
    digitalWrite(blue,blueState);
  }
}


void loop() {
  checkRed();
  checkGreen();
  checkBlue();
}
