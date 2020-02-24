int red = 2;
int intervalRed = 1000; //how long to delay in millis
unsigned long previousRed = 0;
int redState = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(red, OUTPUT);
}

void loop() {
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
