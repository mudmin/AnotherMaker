//Let's see what in the world these pins do

void setup(){
  //Let's use the serial monitor.
  Serial.begin(9600);
  //we are going to loop through the pins and make them all outputs
  for (int i = 2; i < 14; i++) {
    pinMode(i,OUTPUT);
    digitalWrite(i,LOW);
  }
  Serial.println("RGBDuino Booted");
}

void loop(){
  for (int i = 2; i < 14; i++) {
    Serial.print("Writing pin ");
    Serial.print(i);
    Serial.println(" high!");
    digitalWrite(i,HIGH);
    Serial.println("Delaying");
    delay(500);
    Serial.println("Turning off");
    digitalWrite(i,LOW);
    Serial.println("Delaying");
    Serial.println("*************************");
    delay(500);
  }
}
