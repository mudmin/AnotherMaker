#include <TM1637Display.h>

// Define the connections pins:
#define CLK 50  
#define DIO 51  

// Initialize the TM1637Display library with the connections pins
TM1637Display display(CLK, DIO);

// Custom segment for displaying "P"
const uint8_t SEG_P = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G;

void setup() {

  Serial.begin(9600);

  // Initialize the display
  display.setBrightness(0x0a); // Set the display brightness (0x00 to 0x0f)
  display.clear();

  // Initialize digital pins D2 to D52 as outputs
  for (int pin = 2; pin <= 52; pin++) {
    pinMode(pin, OUTPUT);
  }
  Serial.println("Booted");
  delay(1000);
}

void displayLEDNumber(int ledNumber) {
  uint8_t data[] = { SEG_P, 0x00, 0x00, 0x00 }; // Start with "P" and blanks
  
  // Fill in the LED number for the 3rd and 4th digits
  if (ledNumber < 10) {
    // Single digit LED number, display it in the 4th digit place
    data[2] = 0x00; // Blank
    data[3] = display.encodeDigit(ledNumber);
  } else {
    // Two-digit LED number, split it across the 3rd and 4th digits
    data[2] = display.encodeDigit(ledNumber / 10);
    data[3] = display.encodeDigit(ledNumber % 10);
  }
  
  display.setSegments(data);
}

void loop() {

  for (int ledNumber = 1, pin = 2; pin <= 13; ledNumber++, pin++) {
    digitalWrite(pin, HIGH);  
    displayLEDNumber(ledNumber); 
    Serial.print("LED ");
    Serial.print(ledNumber);
    Serial.println(" is lit up");
    delay(1000);             
    digitalWrite(pin, LOW);   
  }

  for (int ledNumber = 13, pin = 22; pin <= 50; ledNumber++, pin++) {
    digitalWrite(pin, HIGH);  
    displayLEDNumber(ledNumber); 
    Serial.print("LED ");
    Serial.print(ledNumber);
    Serial.println(" is lit up");
    delay(1000);            
    digitalWrite(pin, LOW);  
  }
}