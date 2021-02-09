#include <math.h>
#include <Wire.h>
#include <dht11.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include "DS3231.h"
#include <OneWire.h>
#include <IRremote.h>
IRsend irsend;
RTClib RTC;
dht11 DHT;


const int photoCell = A0;
const int microphone = A1;
const int temt = A2;
const int joyX = A3;
const int joyY = A4;
const int thermistor = A5; //analog temp
const int rotary = A6;
const int moist = A7;
const int lm = A8;

const int knock = 3;
const int joyBut = 4;
const int piranha = 5;
const int photoInt = 6;
const int actBuzz = 7;
const int greenLed = 8;
const int blueLed = 2; //can't use 9 because of IRsend

const int redLed = 10;
const int yellowButton = 11;
const int hallSensor = 12;
const int whiteLed = 13;


/*     Arduino Rotary Encoder Tutorial
*
*  by Dejan Nedelkovski, www.HowToMechatronics.com
*
*/
const int outputA = 49;
const int outputB = 48;
const int DHT11_PIN = 47;
const int passBuzz = 46;
const int pir = 45;
const int capTouch = 44;
const int echoPin = 43;
const int trigPin = 42;
const int DS18S20_Pin = 41;
const int hugeLed = 40;
const int relay = 39;
const int vibe = 38;
const int line = 37;
const int tilt = 36;
const int RECV_PIN = 35;
const int flame = 34;
const int obst = 33;


OneWire ds(DS18S20_Pin);
int relayState = 0;
int counter = 0;
int aState;
int aLastState;
unsigned char vibeState = 0;

int maximumRange = 200; // Maximum range needed
int minimumRange = 0; // Minimum range needed
long duration, distance; // Duration used to calculate distance

int val = 0; //store random int values


//stuff for Analog Temp sensor
double Thermister(int RawADC) {  //Function to perform the fancy math of the Steinhart-Hart equation
  double Temp;
  Temp = log(((10240000/RawADC) - 10000));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
  Temp = Temp - 273.15;              // Convert Kelvin to Celsius
  Temp = (Temp * 9.0)/ 5.0 + 32.0; // Celsius to Fahrenheit - comment out this line if you need Celsius

}


//345 accelerometer_data
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);


float AccelMinX = 0;
float AccelMaxX = 0;
float AccelMinY = 0;
float AccelMaxY = 0;
float AccelMinZ = 0;
float AccelMaxZ = 0;

void setup(){
  Serial.begin(9600);

  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    delay(1000);
  }
  pinMode(joyBut,INPUT);
  pinMode(piranha, OUTPUT);
  pinMode(photoInt, INPUT);
  pinMode(actBuzz, OUTPUT);
  pinMode(passBuzz, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(yellowButton, INPUT);
  pinMode(hallSensor, INPUT);
  pinMode(whiteLed, OUTPUT);
  pinMode(pir, INPUT);
  pinMode(capTouch, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(hugeLed, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(vibe, INPUT);
  pinMode(line, INPUT);
  pinMode(tilt, INPUT);
  pinMode(RECV_PIN, INPUT);
  pinMode(flame, INPUT);
  pinMode(obst, INPUT);
  Wire.begin();
  aLastState = digitalRead(outputA);
}

void loop(){
  digitalWrite(whiteLed, LOW);
  digitalWrite(piranha, LOW);
  digitalWrite(hugeLed, LOW);
  digitalWrite(actBuzz, LOW);
  digitalWrite(passBuzz, LOW);

  lightLevel();
  hallDetect();
  checkButton();
  rainbow();
  photoInter();
  soundLevel();
  checkVibe();
  temtLight();
  checkJoy();
  analogTemp();
  checkRoto();
  rotaryEncoder();
  axdl();
  moisture();
  lmTemp();
  checkKnock();
  readDht();
  rtcNow();
  readPir();
  touchSensor();
  echoRange();
  getTemp();
  checkGas();
  relayFlip();
  isLine();
  isTilt();
  irsend.sendRC5(0x0, 8);
  isIR();
  isFlame();
  obstCheck();

  Serial.println("*********************");
  digitalWrite(whiteLed, HIGH);
  digitalWrite(piranha, HIGH);
  delay(5000);
  digitalWrite(hugeLed, HIGH);
  digitalWrite(actBuzz, HIGH);
  digitalWrite(passBuzz, HIGH);
}
//So, uh yeah. Let's get started...
// void whiteLED(){
//   digitalWrite(white,HIGH);
//   delay(1000);
//   digitalWrite(white,LOW);
//   delay(1000);
// }

void lightLevel(){
  val = analogRead(photoCell);
  Serial.print("light: ");
  Serial.println(val, DEC);
}

void hallDetect(){
  val = digitalRead(hallSensor);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    Serial.println("No Magnet");
  } else {
    Serial.println("Magnet");
  }
}

void checkButton(){
  val = digitalRead(yellowButton);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    Serial.println("Not Pushed");
  } else {
    Serial.println("Pushed");
  }
}

void rainbow(){
  for(val=255; val>0; val--)
  {analogWrite(redLed, val);
    analogWrite(blueLed, 255-val);
    analogWrite(greenLed, 128-val);
  }
  for(val=0; val<255; val++)
  {analogWrite(redLed, val);
    analogWrite(blueLed, 255-val);
    analogWrite(greenLed, 128-val);
  }
  digitalWrite(redLed, HIGH);
  digitalWrite(blueLed, HIGH);
  digitalWrite(greenLed, HIGH);
}

void photoInter(){
  val = digitalRead(photoInt);
  if (val == HIGH) {
    Serial.println("Beam Broken");
  }else{
    Serial.println("Beam Solid");
  }
}

void soundLevel(){
  val=analogRead(microphone);
  Serial.print("SoundLvl: ");
  Serial.println(val,DEC);
}

void temtLight(){
  val = analogRead(temt);
  Serial.print("TEMT Light: ");
  Serial.println(val);
}

void checkJoy(){
  int x,y,z;
  x=analogRead(joyX);
  y=analogRead(joyY);
  z=digitalRead(joyBut);
  Serial.print("X: ");
  Serial.print(x ,DEC);
  Serial.print(",");
  Serial.print("Y: ");
  Serial.print(y ,DEC);
  Serial.print(",");
  if(z == HIGH){
    Serial.println(" Button Pushed");
  }else{
    Serial.println(" Button NOT Pushed");
  }
}

void analogTemp(){
  Serial.print(Thermister(analogRead(thermistor)));
  Serial.println("c");
}

void checkRoto(){
  int rot=analogRead(rotary);
  Serial.print("Rotary Value: ");
  Serial.println(val,DEC);
}

void rotaryEncoder(){
  aState = digitalRead(outputA); // Reads the "current" state of the outputA
  // If the previous and the current state of the outputA are different, that means a Pulse has occured
  if (aState != aLastState){
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(outputB) != aState) {
      counter ++;
    } else {
      counter --;
    }
    Serial.print("Position: ");
    Serial.println(counter);
  }
  aLastState = aState; // Updates the previous state of the outputA with the current state
}

void axdl(){
  AccelMinX = 0;
  AccelMaxX = 0;
  AccelMinY = 0;
  AccelMaxY = 0;
  AccelMinZ = 0;
  AccelMaxZ = 0;
  sensors_event_t accelEvent;
  accel.getEvent(&accelEvent);

  if (accelEvent.acceleration.x < AccelMinX) AccelMinX = accelEvent.acceleration.x;
  if (accelEvent.acceleration.x > AccelMaxX) AccelMaxX = accelEvent.acceleration.x;

  if (accelEvent.acceleration.y < AccelMinY) AccelMinY = accelEvent.acceleration.y;
  if (accelEvent.acceleration.y > AccelMaxY) AccelMaxY = accelEvent.acceleration.y;

  if (accelEvent.acceleration.z < AccelMinZ) AccelMinZ = accelEvent.acceleration.z;
  if (accelEvent.acceleration.z > AccelMaxZ) AccelMaxZ = accelEvent.acceleration.z;

  Serial.print("Accel Minimums: "); Serial.print(AccelMinX); Serial.print("  ");Serial.print(AccelMinY); Serial.print("  "); Serial.print(AccelMinZ); Serial.println();
  Serial.print("Accel Maximums: "); Serial.print(AccelMaxX); Serial.print("  ");Serial.print(AccelMaxY); Serial.print("  "); Serial.print(AccelMaxZ); Serial.println();

}

void moisture(){
  val=analogRead(moist);
  Serial.print("Moisture: ");
  Serial.println(val,DEC);
}

void lmTemp(){
  val = analogRead(lm);
  float mv = ( val/1024.0)*5000;
  float cel = mv/10;
  float farh = (cel*9)/5 + 32;

  Serial.print("LM35: ");
  Serial.print(cel);
  Serial.print("*C");
  Serial.println();
}

void checkKnock(){
  val=digitalRead(knock);
  if(val==HIGH){
    Serial.println("Shock Detected");
  }else{
    Serial.println("No Shock Detected");
  }
}

void readDht(){
  val = DHT.read(DHT11_PIN);
  Serial.print("DHT: ");
  Serial.print(DHT.humidity,1);
  Serial.print("%,\t");
  Serial.print(DHT.temperature,1);
  Serial.println(" deg C");
}

void rtcNow(){
  DateTime now = RTC.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}

void readPir(){
  val = digitalRead(pir);
  if(val == 1){
    Serial.println("Somebody is in this area!");
  }else{
    Serial.println("No motion!");
  }
}

void touchSensor(){
  if(digitalRead(capTouch)==HIGH) {
    Serial.println("Capacitive Touched");
  }else{
    Serial.println("Not touched");
  }
}

void echoRange(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration/58.2;
  if (distance >= maximumRange || distance <= minimumRange){
    Serial.println("Out of Range");
  }else {
    Serial.print(distance);
    Serial.println(" cm");
  }
}

void getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
    //no more sensors on chain, reset search
    ds.reset_search();
    return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
    Serial.print("Device is not recognized");
    return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);

  ds.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  Serial.print("DS18b20: ");
  Serial.print(TemperatureSum);
  Serial.println(" deg C");
}

void checkGas(){
  val = analogRead(10);//gas sensor
  Serial.print("Gas: ");
  Serial.println(val,DEC);

  val = analogRead(9);//alcohol sensor
  Serial.print("Alc: ");
  Serial.println(val,DEC);
}

void relayFlip(){
  if(relayState == 0){
    digitalWrite(relay,HIGH);
    relayState = 1;
    Serial.println("Setting Relay High");
  }else{
    digitalWrite(relay,LOW);
    relayState = 0;
    Serial.println("Setting Relay Low");
  }
}

void checkVibe(){
  val = digitalRead(vibe);
  // Serial.println(val);
  if(val != 0){
    Serial.println("Vibration detected");
  }else{
    Serial.println("Vibration NOT detected");
  }
}

void isLine(){
  val = digitalRead(line);
  if(val != 0){
    Serial.println("No Line!");
  }else{
    Serial.println("Line!!!!!!!!!!!!!!!!!");
  }
}

void isTilt(){
  val = digitalRead(tilt);
  if(val != 0){
    Serial.println("Steady");
  }else{
    Serial.println("TILT!");
  }
}

void isIR(){
  val = digitalRead(RECV_PIN);
  if(val == 0){
    Serial.println("IR Detected");
  }else{
    Serial.println("No IR Detected");
  }
}

void isFlame(){
  val = digitalRead(flame);
  if (val == HIGH) {
    Serial.println("Flame NOT Detected");
  }
  else {
    Serial.println("Flame Detected");
  }
}

void obstCheck(){
  val = digitalRead(obst);
  if (val == HIGH) {
    Serial.println("Object NOT Detected");
  }
  else {
    Serial.println("Object Detected");
  }
}

//Sketch uses 13512 bytes (5%) of program storage space. Maximum is 253952 bytes.
//Global variables use 1405 bytes (17%) of dynamic memory, leaving 6787 bytes for local variables. Maximum is 8192 bytes.
