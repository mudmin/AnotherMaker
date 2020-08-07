//Build Notes by https://youtube.com/AnotherMaker
//as featured in https://youtu.be/dfoS-NAG6pM

//You will need the Adafruit Neopixel library from the arduino library manager installed

//Connect the data in pin of the first led ring to pin 6 of the arduino.
//From then on, go data out of the first led ring to data in to the other.  Send 5v and ground to
//each neopixel.  You may also want to add a smoothing capacitor. See this guide for best practices.
// https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices

//The 433 mhz receiver goes to interrupt 0 => that is pin #2 on nano,uno,mega
//It also gets 5v and ground.

//To determine the codes that your pre-programmed remote is putting out, you may want to
//see this video https://www.youtube.com/watch?v=McYDX7_Tqy0 to see how I grab the codes. 


#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      21
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//setup colors
uint32_t red =  pixels.Color(255, 0, 0);
uint32_t green = pixels.Color(0,255,0);
uint32_t yellow = pixels.Color(255, 100, 0);
uint32_t off = pixels.Color(0, 0, 0);
unsigned long rec;
//define beginning/ending pixel numbers
int greenBegin = 0;
int greenEnd = 6;
int yellowBegin = 7;
int yellowEnd = 13;
int redBegin = 14;
int redEnd = 20;


void setup() {
  Serial.begin(9600);
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  pixels.begin(); // This initializes the NeoPixel library.
}

void redLight(){
  offLight();
  for(int j = redBegin; j < redEnd+1 ; j++){
    pixels.setPixelColor(j, red);
  }
  pixels.show();
}

void yellowLight(){
  offLight();
  for(int j = yellowBegin; j < yellowEnd+1 ; j++){
    pixels.setPixelColor(j, yellow);
  }
  pixels.show();
}

void greenLight(){
  offLight();
  for(int j = greenBegin; j < greenEnd+1 ; j++){
    pixels.setPixelColor(j, green);
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
if (mySwitch.available()) {
  rec = mySwitch.getReceivedValue();
  Serial.print("Received ");
  Serial.print( mySwitch.getReceivedValue() );
  Serial.print(" / ");
  Serial.print( mySwitch.getReceivedBitlength() );
  Serial.print("bit ");
  Serial.print("Protocol: ");
  Serial.println( mySwitch.getReceivedProtocol() );
  if(rec == 5730977){ //button a
  redLight();
  Serial.println("red");
  }

  if(rec == 5730978){ //button b
  yellowLight();
  Serial.println("yellow");
  }

  if(rec == 5730980){ //button c
  greenLight();
  }

  if(rec == 5730984){ //button d
  offLight();
  }
  mySwitch.resetAvailable();
}
}
