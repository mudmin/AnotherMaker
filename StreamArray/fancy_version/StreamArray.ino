//This sketch is obviously a ridiculously lazy fork of my "Interactive Mode" for the coding array.
// The point is that you can hook up pretty much whatever sensors you want to whatever pins you want.
// The pins are declared down in line 44-ish.
// THE ONLY think we care about is that data is being transmitted to the PC with a Serial.print of
// a variable with an =, so for the touchpad, we are sending touch=true or touch=false. That's it
// Node red will look for these strings and parse that data. Also note that you do not need to have all these
// sensors and buttons to make this work.  The only thing that matters is that you're sending a whatever=whatever
// when something happens. All the magic happens in node red.

//If you don't understand what's going on, just upload this code and look at the serial monitor. It will all
//make sense.


#include <Arduino.h>      //Used Primarily for Platform IO
#include "pitches.h"      //To create sounds and tones from the buzzer
#include <Wire.h>         //Used to communicate with I2C devices like the LCD
#include <Servo.h>        //the little servo motor
Servo myservo;
#include <DHT.h>          //Temperature & Humidity Sensor
#define DHTPIN 12         //Temp & Humidity sensor pin
#define DHTTYPE DHT11     //There are multiple types of DHT Sensors, CA uses DHT11
#define buz 6             //Buzzer pin
DHT dht(DHTPIN, DHTTYPE);
#include <LiquidCrystal_I2C.h>        //Special library for the LCD screen
LiquidCrystal_I2C lcd(0x27, 16, 2);   //The address of the LCD Screen and how many columns/rows it has

///////////////////////////
//this section tells the Arduino what pins the sensors are connected to
int echo = 4;             //Ultrasonic sensor echo pin
int trigger = 5;          //Ultrasonic sensor trigger pin
int touch = 7;            //Touch module
int pushbutton = 8;       //Tactile pushbutton
int red = 9;              //RGB LED Red pin
int green = 10;           //RGB LED Green pin
int blue = 11;            //RGB LED Blue pin
int dhtsensor = 12;       //DHT 11 Temperature & Humidity Sensor
int led = 13;             //The single color led is connected to pin 13
int hall = A2;            //Hall Effect(Magnetic Sensor)
int flame = A5;



//end pin declarations
//////////////////////////

//We are going to create some variables here that will hold data lower in the sketch
//These top ones basically keep track of time to determine how often a sensor should be checked
long mil = 0;
long ledt = 0;
long buzt = 0;
long lcdt = 0;
long dist = 0;
long lit = 0;
long analogt =0;
long mict = 0;

float temp, humi;
unsigned long dura = 0;
float dst = 0;
int leds = 0;
int bar = 0;
int oldbar = 0;         //The previous reading of the sliderbar
int touchwas = false;   //Save whether the touch sensor was last in a touched or not state
int touched = 0;
int pbpushed = false;         //Save whether the pushbutton was last in a pushed or not state
//Microphone (You can adjust sensitivity with the screw)
const int sampleWindow = 125; // sample period milliseconds (125 mS = 8 Hz)
int soundValue = 0;         // store the value of the sound sensor
double threshold = 2.0;         // set clapping sound threshold voltage value
int flamewas = false;

//These variables are used to receive data from the PC/Raspberry Pi
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

//The code in the setup function is run once on boot
void setup() {
  Serial.begin(9600);           //Setup the serial port to begin at 9600bps
  Serial.println("StreamArray Booted");
  //Give the Arduino some basic information on what the pins we declared above actually do
  pinMode(echo, INPUT);         //Ultrasonic sensor echo pin
  pinMode(trigger, OUTPUT);     //Ultrasonic sensor trigger pin
  pinMode(touch, INPUT);        //Touch Module
  pinMode(pushbutton, INPUT);   //Tactile Pushbutton
  pinMode(red, OUTPUT);         //RGB Red LED
  pinMode(green, OUTPUT);       //RGB Green LED
  pinMode(blue, OUTPUT);        //RGB Blue LED
  pinMode(led, OUTPUT);         //Single Color LED


  myservo.attach(3);           //Tell the servo library which pin the servo is connected to
  dht.begin();                 //Start the temperature and humidity sensor

  myservo.write(180 - int(bar / 5.68));

  lcd.init();                  //Startup the LCD
  lcd.backlight();             //Turn on the LCD baclight

  bar = analogRead(A0);        //Read the position of the slider bar once on boot
  oldbar = bar;
  makeIcon();                  //Call the functions that display the boot animation on the LCD Screen
  drawIcon();
}

void loop() {
  //The number of milliseconds since the Arduino booted
  mil = millis();

  if(digitalRead(pushbutton) == HIGH){
    if(pbpushed == false){
      Serial.println("pb=true");
      pbpushed = true;
    }else{
      pbpushed = false;
    }
  }

  if(analogRead(flame) > 1019){
    if(flamewas == false){ //only print this if it was previously false
      Serial.println("fire=true");
    }
    flamewas = true;
  }else{
    flamewas = false;
  }

  if(analogRead(hall) < 480){  //look for a magnet near the hall effect sensor if one is found
    Serial.println("magnet=true");
  }
  //Set up some variables that get cleared out every time the code loops over
  float Duration, Distance;
  bar = analogRead(A0);
  // myservo.write(180 - int(bar / 5.68));
  if(bar != oldbar){
    // Serial.println(bar);
    Serial.println("slider=true");
    oldbar = bar;
  }


  //Ultrasonic Sensor Reading
  if (mil > dist) {
    dist = mil + 1000;
    digitalWrite(trigger, LOW);                                  //Make sure the trigger is off
    delayMicroseconds(2);                                        //Wait 2 microseconds
    digitalWrite(trigger, HIGH);                                 //Shoot off a pulse of ultrasound
    delayMicroseconds(10);                                       //Keep shooting for 10 microseconds
    digitalWrite(trigger, LOW);                                  //Stop shooting
    Duration = pulseIn(echo, HIGH);                              //See how long it took to receive the reflected sound
    Distance = ((float)(340 * Duration) / 10000) / 2;            //Calculate the distance based on the time it took to receive the signal

    lcd.setCursor(10, 1);                                        //Print the distance on the second (1) line of the LCD in the 10th position
    if (Distance < 100) {
      lcd.print(Distance, 10);
    } else {
      lcd.print(" ");
      lcd.print(int(Distance));
    }
    lcd.setCursor(14, 1);
    lcd.print("cm");
    if(Distance > 17 && Distance < 21){
      Serial.println("dist=true");                                        //Only send serial distance if in range of trigger
      Serial.println(Distance);
    }
  }


  //Temperature & Humidity
  if (mil > lcdt) {
    lcdt = mil + 5000;                //Read the temperature every 5 seconds (5000 milliseconds)
    temp = dht.readTemperature();     //Read the temperature sensor and assign it to the temp variable
    humi = dht.readHumidity();        //Get the humidity and assign it to the humi variable
    lcd.setCursor(3, 0);              //Move the cursor to the top row (0), 3rd position

    if (temp < 10) {                  //If temp < 10 put a space to make the spacing nice on the screen
      lcd.print(" ");
    }
    lcd.print(int(temp));             //Print the temperature to the screen
    lcd.setCursor(11, 0);             //Move the cursor to the top row, 11th position

    if (humi < 10) {
      lcd.print(" ");                 //Add a space if the humidity is less than 10%
    }
    lcd.print(int(humi));             //Print the humidity to the lcd

    // Serial.print("temp=");              //Send temperature and humidity to serial monitor
    // Serial.println(temp);
    if(humi > 85){
      Serial.println("hum=true");
    }

  }

  //Photocell (measure light)
  if (mil > lit) {
    lit = mil + 2000;                 //Measure light every 2 seconds (2000 milliseconds)
    // Serial.print("photo=");
    lcd.setCursor(0, 1);              //Set the cursor to the left side of the second line (1)
    if (analogRead(A1) > 700) {
      Serial.println("photo=true");
      // Serial.println("Bright");
    }
    // else if (analogRead(A1) > 550) {
    //   Serial.println("Normal");
    // } else {
    //   Serial.println("Dark");
    // }
  }

  if(mil > mict){
    mict = mil + 300;
    unsigned long start= millis();  // start sampling
    unsigned int peakToPeak = 0;    // Amplitude Value Variables
    unsigned int signalMax = 0;
    unsigned int signalMin = 1024;


    // collect data for 125 milliseconds.
    while (millis() - start < sampleWindow)
    {
      soundValue = analogRead(3); // specify analogue pin number 3.
      if (soundValue < 1024)  // Read data up to the ADC maximum (1024=10bit).
      {
        if (soundValue > signalMax)
        {
          signalMax = soundValue;  // Store the maximum sound value in the signalMax variable.
        }
        else if (soundValue < signalMin)
        {
          signalMin = soundValue;  // store the sound minimum in the variable (signalMin).
        }
      }
    }
    peakToPeak = signalMax - signalMin;  // calculate peak-to-peak amplitude
    double volts = (peakToPeak * 5.0) / 1024; // convert the ADC value to a voltage value. Reference Voltage 5V
    //let's increase the bottom a little bit
    if(volts < 1.5){
      volts = volts*3;
    }else{
      volts = volts *2;
    }
    if(volts > 6.6){
      Serial.println("sound=true");
    }
    // Serial.println(volts);
  }
  //RGB led
  //If either the touch sensor is touched scroll through the RGB LED, otherwise, turn it off
  if (digitalRead(touch) == HIGH) {
    if (mil > ledt) {
      if(touchwas == false){    //check whether this is different than last time
        Serial.println("touch=true");
        touchwas = true;
      }

      ledt = mil + 120;
      leds++;
      if (leds == 1) {
        digitalWrite(9, HIGH);
        digitalWrite(11, LOW);
      } else if (leds == 2) {
        digitalWrite(10, HIGH);
        digitalWrite(9, LOW);
      } else {
        digitalWrite(11, HIGH);
        digitalWrite(10, LOW);
      }
      if (leds == 3) {
        leds = 0;
      }
    }
  } else {
    if(touchwas == true){    //check whether this is different than last time
      Serial.println("touch=false");
      touchwas = false;
    }
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
    digitalWrite(11, LOW);
  }


}






//This function makes the little icons that display on the LCD during the temperature/humidity readings
void makeIcon() {
  const static uint8_t icon_temp0[] PROGMEM = {
    0x4, 0xa, 0xa, 0xe, 0xe, 0x1f, 0x1f, 0xe
  };
  const static uint8_t icon_temp1[] PROGMEM = {
    0x8, 0x14, 0x8, 0x3, 0x4, 0x4, 0x3, 0
  };
  const static uint8_t icon_temp2[] PROGMEM = {
    0xc, 0x12, 0x12, 0xc, 0, 0, 0, 0
  };
  const static uint8_t icon_humi[] PROGMEM = {
    0x4, 0x4, 0xa, 0xa, 0x11, 0x11, 0x11, 0xe
  };
  const static uint8_t icon_sonar[] PROGMEM = {
    0x2, 0x9, 0x5, 0x15, 0x15, 0x5, 0x9, 0x2
  };

  uint8_t temp[8];
  memcpy_P(temp, icon_temp0, 8);
  lcd.createChar(0, temp);

  memcpy_P(temp, icon_temp1, 8);
  lcd.createChar(1, temp);

  memcpy_P(temp, icon_temp2, 8);
  lcd.createChar(2, temp);

  memcpy_P(temp, icon_humi, 8);
  lcd.createChar(3, temp);

  memcpy_P(temp, icon_sonar, 8);
  lcd.createChar(4, temp);
}

//This puts the icons on the right place on the screen
void drawIcon() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.write(0);
  //  lcd.setCursor(5, 0);
  //  lcd.write(1);
  lcd.setCursor(5, 0);
  lcd.write(2);
  lcd.write('C');
  lcd.setCursor(9, 0);
  lcd.write(3);
  lcd.setCursor(13, 0);
  lcd.write('%');
  lcd.setCursor(9, 1);
  lcd.write(4);
  lcd.setCursor(14, 1);
  lcd.print("cm");
}
