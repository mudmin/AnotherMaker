//Let's see how many relays I can connect to an Arduino Mega clone before the thing freaks out.
//relays connected to pins 2-9 for 8 relays and so forth
int min = 2;
int max = 26;
void setup() {
Serial.begin(9600);

for (int i = min; i < max; i++) {
  pinMode(i,OUTPUT);
  digitalWrite(i,HIGH);
}
Serial.println("Booted");
delay(2500);
}

void loop() {
  Serial.println("************ New Loop ************");
  for (int i = min; i <max; i++) {
    Serial.println("Messing with Relay ");
    Serial.println(i);
    digitalWrite(i,LOW);
    delay(200);
    digitalWrite(i,HIGH);
    delay(200);
    digitalWrite(i,LOW);
    delay(200);
    digitalWrite(i,HIGH);
    delay(200);
  }
  for (int i = min; i <max; i++) {
    digitalWrite(i,LOW);
  }
  delay(2000);
  for (int i = min; i <max; i++) {
    digitalWrite(i,HIGH);
  }
  delay(2000);
}
