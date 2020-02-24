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

  Serial.print("Current Millis: ");
  Serial.println(currentMillis);
  Serial.print("Previous Red State Change:");
  Serial.println(previousRed);
  Serial.print("Difference");
  Serial.print(currentMillis-previousRed);
  Serial.println(" ms");
  delay(800);

  if(currentMillis - previousRed >= intervalRed){
    Serial.println("Difference is greater than interval. Change state!");
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
