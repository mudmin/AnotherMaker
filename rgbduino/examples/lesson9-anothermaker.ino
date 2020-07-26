//Let's use both RGB LEDs at once
//Note that these LEDs are connected individually so you can't use them as if the 2 leds are an led "strip"

//include AdaFruit's RGB Library
#include <Adafruit_NeoPixel.h>

//These are the arduino pins that contain those 2 RGB leds..

#define RGB1 12
#define RGB2 13



//Now we have to set them up!
// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(1, RGB1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(1, RGB2, NEO_GRB + NEO_KHZ800);
//Let's setup a few colors
uint32_t red      = strip2.Color(255, 0, 0);
uint32_t green    = strip2.Color(0, 255, 0);
uint32_t blue     = strip2.Color(0, 0, 255);
uint32_t magenta  = strip2.Color(255, 0, 255);
uint32_t off      = strip2.Color(0, 0, 0);
void setup(){
  //Let's use the serial monitor.
  Serial.begin(9600);
  //we are going to loop through the pins and make them all outputs
  for (int i = 2; i < 14; i++) {
    pinMode(i,OUTPUT);
    digitalWrite(i,LOW);
  }

  strip1.begin();
  strip1.show(); // Initialize all pixels to 'off'
  strip2.begin();
  strip2.show(); // Initialize all pixels to 'off'
  Serial.println("RGBDuino Booted");
}

void loop(){
  // Some example procedures showing how to display to the pixels:
  //We will define our own colors to power LED1
  //Notice how levels lower than 255 make that color less bright
  Serial.println("LED1!");
  strip1.setPixelColor(0, 60,0,0);
  strip1.show();
  delay(500);
  strip1.setPixelColor(0, 0,60,0);
  strip1.show();
  delay(500);
  strip1.setPixelColor(0, 0,0,60);
  strip1.show();
  delay(500);
  strip1.setPixelColor(0, 60,0,60);
  strip1.show();
  delay(500);
  strip1.setPixelColor(0, 0,0,0);
  strip1.show();
  delay(500);


  //We will use the colors defined at the top of the sketch to power LED2
  Serial.println("LED2!");
  strip2.setPixelColor(0, red);
  strip2.show();
  delay(500);
  strip2.setPixelColor(0, green);
  strip2.show();
  delay(500);
  strip2.setPixelColor(0, blue);
  strip2.show();
  delay(500);
  strip2.setPixelColor(0, magenta);
  strip2.show();
  delay(500);
  strip2.setPixelColor(0, off);
  strip2.show();
  delay(500);




}
