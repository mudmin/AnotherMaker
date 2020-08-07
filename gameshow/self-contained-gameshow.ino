//Buttons are connected between pin and ground on pins as defined below.
//Neopixel is connected to pin 6. 

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif


#define PIN            6
const int redButton = 3;
const int greenButton = 4;
const int orangeButton = 5;
const int blueButton = 8;
int redSelected = 0;
int greenSelected = 0;
int orangeSelected = 0;
int blueSelected = 0;

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      7
#include <RCSwitch.h>
int state = 0;
//states
// 0 = idle
// 1 = get ready
// 2 = go!
// 3 = number chosen

RCSwitch mySwitch = RCSwitch();
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//setup colors
//setup colors
uint32_t red =  pixels.Color(255, 0, 0);
uint32_t green = pixels.Color(0,255,0);
uint32_t blue = pixels.Color(0,0,255);
uint32_t orange = pixels.Color(247, 80, 0);
uint32_t purple = pixels.Color(150, 0, 150);
uint32_t white = pixels.Color(50, 50, 50);
uint32_t off = pixels.Color(0, 0, 0);
unsigned long rec;


void setup() {
  Serial.begin(9600);
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  pixels.begin(); // This initializes the NeoPixel library.
  pinMode(redButton, INPUT_PULLUP);
  pinMode(greenButton, INPUT_PULLUP);
  pinMode(blueButton, INPUT_PULLUP);
  pinMode(orangeButton, INPUT_PULLUP);
  offLight();
}

void whiteLight(){
  offLight();
  for(int j = 0; j < NUMPIXELS+1 ; j++){
    pixels.setPixelColor(j, white);
  }
  pixels.show();
}

void redLight(){
  offLight();
  for(int j = 0; j < NUMPIXELS+1 ; j++){
    pixels.setPixelColor(j, red);
  }
  pixels.show();
}

void orangeLight(){
  offLight();
  for(int j = 0; j < NUMPIXELS+1 ; j++){
    pixels.setPixelColor(j, orange);
  }
  pixels.show();
}

void greenLight(){
  offLight();
  for(int j = 0; j < NUMPIXELS+1 ; j++){
    pixels.setPixelColor(j, green);
  }
  pixels.show();
}

void blueLight(){
  offLight();
  for(int j = 0; j < NUMPIXELS+1 ; j++){
    pixels.setPixelColor(j, blue);
  }
  pixels.show();
}

void purpleLight(){
  offLight();
  for(int j = 0; j < NUMPIXELS+1 ; j++){
    pixels.setPixelColor(j, purple);
  }
  pixels.show();
}

void offLight(){
  for(int j = 0; j < NUMPIXELS+1 ; j++){
    pixels.setPixelColor(j, off);
  }
  pixels.show();
}

void loop() {
  int origState = state;
  // Serial.print(state);
  if (mySwitch.available()) {
    rec = mySwitch.getReceivedValue();
    // Serial.print("Received ");
    // Serial.print( mySwitch.getReceivedValue() );
    // Serial.print(" / ");
    // Serial.print( mySwitch.getReceivedBitlength() );
    // Serial.print("bit ");
    // Serial.print("Protocol: ");
    // Serial.println( mySwitch.getReceivedProtocol() );
    if(rec == 9332196){ //button a
      redSelected = 0;
      greenSelected = 0;
      orangeSelected = 0;
      blueSelected = 0;
      state = 0;
      if(state != origState){
        offLight();
        Serial.println("state 0 - doing nothing");
      }
    }

    if(rec == 9332193){ //button b
      redSelected = 0;
      greenSelected = 0;
      orangeSelected = 0;
      blueSelected = 0;
      state = 1;
      if(state != origState){
        whiteLight();
        Serial.println("state 1 - get ready");
      }
    }

    if(rec == 9332200){ //button c
      state = 2;
      redSelected = 0;
      greenSelected = 0;
      orangeSelected = 0;
      blueSelected = 0;
      if(state != origState){
        purpleLight();
        Serial.println("state 2 - waiting for button");
      }
    }


    if(rec == 9332194){ //button d
      purpleLight();
      state = 2;
      if(state != origState){
        Serial.println("state 2 - waiting for button that has not been pressed.");
      }
    }
    mySwitch.resetAvailable();

}

if(state == 2){
  int readRed = digitalRead(redButton);
  // Serial.print('red -');Serial.println(readRed);
  if(readRed == LOW && redSelected == 0){
    Serial.println("low");
    redLight();
    redSelected = 1;
    state = 3;
  }
  int readGreen = digitalRead(greenButton);
  // Serial.print('green -');Serial.println(readGreen);
  if(readGreen == LOW && greenSelected == 0){
  Serial.println("low");
    greenLight();
    greenSelected = 1;
    state = 3;

  }
  int readOrange = digitalRead(orangeButton);
  // Serial.print('orange -');Serial.println(readOrange);
  if(readOrange == LOW && orangeSelected == 0){
      Serial.println("low");
    orangeLight();
    orangeSelected = 1;
    state = 3;

  }
  int readBlue = digitalRead(blueButton);
  // Serial.print('blue -');Serial.println(readBlue);
  if(readBlue == LOW && blueSelected == 0){
      Serial.println("low");
    blueLight();
    blueSelected = 1;
    state = 3;
  }
}
}
