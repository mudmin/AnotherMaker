// A button gets soldered between Pins A6 and GND on the Arduino Nano
// Holding the button down on boot will switch to mode 1 which allows you to
// manually step through the pins.  Tapping the button in mode 0 will change
// the speed at which the LEDs step through the pins.

#include <TM1637Display.h>

int stepSpeed = 500;
int mode = 0; 
const int buttonPin = A6;
bool buttonPressed = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 68;
unsigned long buttonPressStartTime = 0;
bool buttonHeld = false;
unsigned long lastStepTime = 0;

#define CLK A0
#define DIO A1
TM1637Display display(CLK, DIO);

const uint8_t SEG_P = 0b01110011;
const uint8_t SEG_I = 0b00000100;
const uint8_t SEG_N = 0b01010100;
const uint8_t SEG_DASH = 0b01000000;

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  display.setBrightness(0x0f);
  for (int pin = 12; pin >= 4; pin--) {
    pinMode(pin, OUTPUT);
  }
  int bootreading = analogRead(buttonPin);
  if(bootreading< 10){
    mode = 1;
  }
  Serial.println("9 Pin Cable Tester with Screen by AnotherMaker");

  // Display initial mode on boot
  uint8_t data[4];
  data[0] = SEG_DASH;
  data[1] = SEG_DASH;
  data[2] = SEG_DASH;
  data[3] = display.encodeDigit(mode);
  display.setSegments(data);
  delay(1000);
  updateDisplayAfterModeSwitch();
}

void loop() {
  int reading = analogRead(buttonPin);
  Serial.println(reading);

  if (reading < 10) {
    if (!buttonPressed && (millis() - lastDebounceTime) > debounceDelay) {
      buttonPressed = true;
      buttonPressStartTime = millis();
      lastDebounceTime = millis();
    } 
  } else {
    if (buttonPressed) {
      if (!buttonHeld && (millis() - lastDebounceTime) > debounceDelay) {
        if (mode == 0) {
          adjustStepSpeed();
        } else if (mode == 1) {
          incrementLEDs();
        }
      }
      buttonPressed = false;
      buttonHeld = false;
      lastDebounceTime = millis();
    }
  }

  if (mode == 0) {
    autoStepThroughPins();
  }
}

void adjustStepSpeed() {
  if (stepSpeed == 500) {
    stepSpeed = 1000;
  } else if (stepSpeed == 1000) {
    stepSpeed = 2500;
  } else if (stepSpeed == 2500) {
    stepSpeed = 250;
  } else {
    stepSpeed = 500;
  }
  Serial.print("Step Speed: ");
  Serial.println(stepSpeed);
  display.clear();
  display.showNumberDec(stepSpeed);
  delay(1000); // Blocking delay for speed change
  display.clear(); // Clear the display after showing speed
}

void incrementLEDs() {
  static int currentLED = 12;
  static bool firstPress = true;
  
  if (firstPress) {
    firstPress = false;
    currentLED = 12; // Reset to initial state on first press in mode 1
  } else {
    digitalWrite(currentLED, LOW);
    currentLED--;
    if (currentLED < 4) {
      currentLED = 12;
    }
  }
  
  digitalWrite(currentLED, HIGH);
  int ledNumber = 13 - currentLED;
  uint8_t data[] = {SEG_P, SEG_I, SEG_N, display.encodeDigit(ledNumber)};
  display.setSegments(data);
}

void autoStepThroughPins() {
  static int currentPin = 12;
  unsigned long currentTime = millis();
  
  if (currentTime - lastStepTime >= stepSpeed) {
    digitalWrite(currentPin, LOW); // Turn off the previous LED
    currentPin--;
    if (currentPin < 4) {
      currentPin = 12;
    }
    digitalWrite(currentPin, HIGH); // Turn on the current LED
    int ledNumber = 13 - currentPin;
    uint8_t data[] = {SEG_P, SEG_I, SEG_N, display.encodeDigit(ledNumber)};
    display.setSegments(data);
    lastStepTime = currentTime;
  }
}

void updateDisplayAfterModeSwitch() {
  if (mode == 1) {
    static int currentLED = 12; // Ensure the currentLED value is consistent
    int ledNumber = 13 - currentLED;
    uint8_t data[] = {SEG_P, SEG_I, SEG_N, display.encodeDigit(ledNumber)};
    display.setSegments(data);
  } else {
    display.clear();
  }
}
