//Let's see how many relays I can connect to an Arduino Mega clone before the thing freaks out.
//relays connected to pins 2-9
int min = 2;
int max = 10;
void setup() {
Serial.begin(9600);

for (int i = min; i < max; i++) {
  pinMode(i,OUTPUT);
  digitalWrite(i,LOW);
}
Serial.println("Booted");
delay(2500);
}

void loop() {
 for (int i = min; i < max; i++) {
  digitalWrite(i,HIGH);
}
delay(5000);

 for (int i = min; i < max; i++) {
  digitalWrite(i,LOW);
}
delay(5000);
}
