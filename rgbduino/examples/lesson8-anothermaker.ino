//Let's see what we can do with these RGB LEDs
//Note that these LEDs are connected individually so you can't use them as if the 2 leds are an led "strip"

//include AdaFruit's RGB Library
#include <Adafruit_NeoPixel.h>

//These are the arduino pins that contain those 2 RGB leds..
//We are only going to use the second one in this example
#define RGB1 12
#define RGB2 13

//Now we have to set them up!
// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(1, RGB2, NEO_GRB + NEO_KHZ800);

void setup(){
  //Let's use the serial monitor.
  Serial.begin(9600);
  //we are going to loop through the pins and make them all outputs
  for (int i = 2; i < 14; i++) {
    pinMode(i,OUTPUT);
    digitalWrite(i,LOW);
  }

  strip2.begin();
  strip2.show(); // Initialize all pixels to 'off'
  Serial.println("RGBDuino Booted");
}

void loop(){
  // Some example procedures showing how to display to the pixels:
  //We will use these "manual" examples to mess with RGB1
  Serial.println("ColorWiping!");
  colorWipe(strip2.Color(255, 0, 0), 50); // Red
  colorWipe(strip2.Color(0, 255, 0), 50); // Green
  colorWipe(strip2.Color(0, 0, 255), 50); // Blue
  delay(1000);
  // Send a theater pixel chase in...
  Serial.println("TheaterChasing!");
  theaterChase(strip2.Color(127, 127, 127), 50); // White
  theaterChase(strip2.Color(127,   0,   0), 50); // Red
  theaterChase(strip2.Color(  0,   0, 127), 50); // Blue
  delay(1000);

  //Let's use the functions defined below to mess with RGB2
  Serial.println("Rainbow!");
  rainbow(20);
  delay(1000);

  Serial.println("RainbowCycle!");
  rainbowCycle(20);
  delay(1000);

  Serial.println("TheaterChaseRainbow");
  theaterChaseRainbow(50);
  delay(1000);
}

// These functions will be run on RGB2
// These examples are generally expecting more than 1 or 2 LEDs, but we'll play anyway.
// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip2.numPixels(); i++) {
      strip2.setPixelColor(i, c);
      strip2.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip2.numPixels(); i++) {
      strip2.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip2.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip2.numPixels(); i++) {
      strip2.setPixelColor(i, Wheel(((i * 256 / strip2.numPixels()) + j) & 255));
    }
    strip2.show();
    delay(wait);
  }
}
//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip2.numPixels(); i=i+3) {
        strip2.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip2.show();

      delay(wait);

      for (int i=0; i < strip2.numPixels(); i=i+3) {
        strip2.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip2.numPixels(); i=i+3) {
          strip2.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip2.show();

        delay(wait);

        for (int i=0; i < strip2.numPixels(); i=i+3) {
          strip2.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip2.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip2.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip2.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
