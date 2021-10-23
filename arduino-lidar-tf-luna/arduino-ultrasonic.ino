// ---------------------------------------------------------------- //
// Arduino Ultrasoninc Sensor HC-SR04
// Re-writed by Arbi Abdul Jabbaar
// Using Arduino IDE 1.8.7
// Using HC-SR04 Module
// Tested on 17 September 2019
// ---------------------------------------------------------------- //
#include <LiquidCrystal.h>
#define echoPin 18 // attach pin D18 Arduino to pin Echo of HC-SR04
#define trigPin 19 //attach pin D19 Arduino to pin Trig of HC-SR04
const int pin_RS = 8;
const int pin_EN = 9;
const int pin_d4 = 4;
const int pin_d5 = 5;
const int pin_d6 = 6;
const int pin_d7 = 7;
const int pin_BL = 10;
LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);

// defines variables
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement
char dist[11];

void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  Serial.begin(115200); // // Serial Communication is starting with 9600 of baudrate speed
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("Device");
  lcd.setCursor(0,1);
  lcd.print("Booted");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Distance (cm)");
  lcd.setCursor(0,1);
  lcd.print("0cm");
}
void loop() {
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  sprintf (dist, "%04i", distance);
  lcd.setCursor(0,1);
  lcd.print(dist);

}
