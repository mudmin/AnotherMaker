unsigned long currentMillis = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  currentMillis = millis();
  Serial.print("Current Millis: ");
  Serial.println(currentMillis);
  delay(5000);
}
