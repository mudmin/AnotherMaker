//if you are not familiar with programming the arduino, 
//you can find the software and instructions here:
// https://www.arduino.cc/en/Guide/ArduinoNano


// be sure to include the TM1637 library in the Arduino IDE
#include <TM1637Display.h>

// Define the connections pins for TM1637 display
#define CLK A0
#define DIO A1

TM1637Display display(CLK, DIO);

// Custom segment definitions for "P", "I", and "N" on the first 3 digits
const uint8_t SEG_P = 0b01110011;
const uint8_t SEG_I = 0b00000100; 
const uint8_t SEG_N = 0b01010100;

void setup() {
  // Initialize serial communication at 9600 baud
  Serial.begin(9600);
  
  // Initialize the TM1637 display
  display.setBrightness(0x0f); // Set the brightness to maximum
  
  // Set pins 12 through 4 as output
  for (int pin = 12; pin >= 4; pin--) {
    pinMode(pin, OUTPUT);
  }
  
  // Print a welcome message
  Serial.println("9 Pin Cable Tester with Screen by AnotherMaker");
}

void loop() {
  // Loop through pins 12 through 4
  for (int pin = 12; pin >= 4; pin--) {
    // Turn on the current LED
    digitalWrite(pin, HIGH);
    // Determine which LED is lit up
    int ledNumber = 13 - pin;
    // Print which LED is lit up
    Serial.print("LED ");
    Serial.print(ledNumber);
    Serial.println(" is lit up");

    // Display "PIN" followed by the LED number on TM1637 display
    uint8_t data[] = {SEG_P, SEG_I, SEG_N, display.encodeDigit(ledNumber)};
    display.setSegments(data);
    
    // Wait for 0.5 seconds
    delay(500);

    // Turn off the current LED
    digitalWrite(pin, LOW);
  }
}
