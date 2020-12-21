//Let's see how many relays I can connect to an Arduino Mega clone before the thing freaks out.
//relays connected to pins 2-9
int min = 0;
int max = 54;
static const uint8_t analog_pins[] = {A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15};
int analogMax = 16;


void setup() {
// Serial.begin(9600);

for (int i = min; i < max; i++) {
  pinMode(i,OUTPUT);
  digitalWrite(i,LOW);
}

for (int i = 0; i < analogMax; i++) {
  pinMode(analog_pins[i],OUTPUT);
  digitalWrite(analog_pins[i],LOW);
}

// Serial.println("Booted");
delay(2500);
}

void loop() {
 for (int i = min; i < max; i++) {
  digitalWrite(i,HIGH);
}
for (int i = 0; i < analogMax; i++) {
  digitalWrite(analog_pins[i],HIGH);
}
delay(5000);

 for (int i = min; i < max; i++) {
  digitalWrite(i,LOW);
}
for (int i = 0; i < analogMax; i++) {
  digitalWrite(analog_pins[i],LOW);
}
delay(5000);
}
