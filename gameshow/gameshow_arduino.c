#include <RCSwitch.h>
#include <CuteBuzzerSounds.h>
//Install the RCSwitch and CuteBuzzerSounds libraries from the Arduino Library Manager
int red = 3;
int green = 4;
int yellow = 5;
int blue = 6;
#define BUZZER_PIN 7

int redAvail = 0;
int greenAvail = 0;
int yellowAvail = 0;
int blueAvail = 0;

int state = -1;
//states
// -1 = booted
//  0 = Asking the question, if they buzz in, they're locked out
//  1 = Available people can buzz in


RCSwitch mySwitch = RCSwitch();
unsigned long rec;

void setup() {
  Serial.begin(9600);
  cute.init(BUZZER_PIN);
  Serial.println("booted");
  cute.play(S_CONNECTION);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(blue, OUTPUT);

}


void loop() {

  // //Comment this in if you want to scroll through all the available sounds
  // //And see their names printed on the serial monitor
  // soundSamples();

  // Serial.print(state);
  if (mySwitch.available()) {
    rec = mySwitch.getReceivedValue();

    // //Comment in this section to see ALL 433mhz stuff coming in
    // Serial.print("Received ");
    // Serial.print( mySwitch.getReceivedValue() );
    // Serial.print(" / ");
    // Serial.print( mySwitch.getReceivedBitlength() );
    // Serial.print("bit ");
    // Serial.print("Protocol: ");
    // Serial.println( mySwitch.getReceivedProtocol() );
    // //End Debug Section


//This section of stuff deals with the leader's remote and will work regardless of the "state" of the game
    if(rec == 5509379){ //button a resets everything
      redAvail = 1;
      greenAvail = 1;
      yellowAvail = 1;
      blueAvail = 1;
      digitalWrite(red,HIGH);
      digitalWrite(green,HIGH);
      digitalWrite(yellow,HIGH);
      digitalWrite(blue,HIGH);
      state = 0;
      cute.play(S_MODE3);
      Serial.println("State set to 0. Everything reset.");
    }

    if(rec == 5509388){ //button b - Buttons unlocked!
      state = 1;
      cute.play(S_HAPPY_SHORT);
        Serial.println("State set to 1. Locked and Loaded!");
    }

    if(rec == 5509424){ //button c - Let the remaining people try!!
      //Let's reset the LEDs. If someone has already buzzed in, we want their LED to be off
      //If they haven't, we want it to be on.
      if(redAvail == 1){
        digitalWrite(red,HIGH);
      }else{
        digitalWrite(red,LOW);
      }

      if(yellowAvail == 1){
        digitalWrite(yellow,HIGH);
      }else{
        digitalWrite(yellow,LOW);
      }

      if(blueAvail == 1){
        digitalWrite(blue,HIGH);
      }else{
        digitalWrite(blue,LOW);
      }

      if(greenAvail == 1){
        digitalWrite(green,HIGH);
      }else{
        digitalWrite(green,LOW);
      }
      state = 1;
      cute.play(S_HAPPY_SHORT);
        Serial.println("State set to 1. Available Players can buzz!!!");
    }


// End Leader Remote Section

    if(state == 0){

      //In this state we want to lock out anyone who pushes the button early
      if(rec == 166945){ //red
        cute.play(S_FART1);
        redAvail = 0; //make the button unavailable
        digitalWrite(red, LOW); //turn the led off
        Serial.println("red disabled");
      }

      if(rec == 8743713){ //yellow
        cute.play(S_FART1);
        yellowAvail = 0;
        digitalWrite(yellow, LOW);
        Serial.println("yellow disabled");
      }

      if(rec == 8717953){ //blue
        cute.play(S_FART1);
        blueAvail = 0;
        digitalWrite(blue, LOW);
        Serial.println("blue disabled");
      }

      if(rec == 276817){ //green
        cute.play(S_FART1);
        greenAvail = 0;
        digitalWrite(green, LOW);
        Serial.println("green disabled");
      }
    }

    if(state == 1){
      //In this state we want to lock out anyone who pushes the button early
      if(rec == 166945 && redAvail == 1){ //red
        redAvail = 0; //make the button unavailable
        cute.play(S_BUTTON_PUSHED);
        digitalWrite(red, LOW);
        delay(100);
        digitalWrite(red, HIGH); //turn the led on
        digitalWrite(yellow,LOW);
        digitalWrite(green,LOW);
        digitalWrite(blue,LOW);
        Serial.println("red buzzed in");

        state = 2;
      }

      if(rec == 8743713  && yellowAvail == 1){ //yellow
        yellowAvail = 0;
        cute.play(S_BUTTON_PUSHED);
        digitalWrite(yellow, LOW);
        delay(100);
        digitalWrite(yellow, HIGH);
        digitalWrite(red,LOW);
        digitalWrite(green,LOW);
        digitalWrite(blue,LOW);
        Serial.println("yellow buzzed in");
        state = 2;
      }

      if(rec == 8717953 && blueAvail == 1){ //blue
        blueAvail = 0;
        cute.play(S_BUTTON_PUSHED);
        digitalWrite(blue, LOW);
        delay(100);
        digitalWrite(blue, HIGH);
        digitalWrite(yellow,LOW);
        digitalWrite(green,LOW);
        digitalWrite(red,LOW);
        Serial.println("blue buzzed in");
        state = 2;
      }

      if(rec == 276817  && greenAvail == 1){ //green
        greenAvail = 0;
        cute.play(S_BUTTON_PUSHED);
        digitalWrite(green, LOW);
        delay(100);
        digitalWrite(green, HIGH);
        digitalWrite(yellow,LOW);
        digitalWrite(red,LOW);
        digitalWrite(blue,LOW);
        Serial.println("green buzzed in");
        state = 2;
      }
    }


    }
    mySwitch.resetAvailable();

}

void soundSamples(){
  //Sound examples
  //Comment this section in to hear all the sounds and see the names on the serial monitor
  Serial.println("HAPPY");
    cute.play(S_HAPPY);
  delay(500);
  Serial.println("CONNECTION");
    cute.play(S_CONNECTION);
  delay(500);
  Serial.println("DISCONNECTION");
    cute.play(S_DISCONNECTION);
  delay(500);
  Serial.println("BUTTON_PUSHED");
    cute.play(S_BUTTON_PUSHED);
  delay(500);
  Serial.println("MODE1");
    cute.play(S_MODE1);
  delay(500);
  Serial.println("MODE2");
    cute.play(S_MODE2);
  delay(500);
  Serial.println("MODE3");
    cute.play(S_MODE3);
  delay(500);
  Serial.println("SURPRISE");
    cute.play(S_SURPRISE);
  delay(500);
  Serial.println("OHOOH");
    cute.play(S_OHOOH);
  delay(500);
  Serial.println("OHOOH2");
    cute.play(S_OHOOH2);
  delay(500);
  Serial.println("CUDDLY");
    cute.play(S_CUDDLY);
  delay(500);
  Serial.println("SLEEPING");
    cute.play(S_SLEEPING);
  delay(500);
  Serial.println("HAPPY");
    cute.play(S_HAPPY);
  delay(500);
  Serial.println("SUPER_HAPPY");
    cute.play(S_SUPER_HAPPY);
  delay(500);
  Serial.println("HAPPY_SHORT");
    cute.play(S_HAPPY_SHORT);
  delay(500);
  Serial.println("SAD");
    cute.play(S_SAD);
  delay(500);
  Serial.println("CONFUSED");
    cute.play(S_CONFUSED);
  delay(500);
  Serial.println("FART1");
    cute.play(S_FART1);
  delay(500);
  Serial.println("FART2");
    cute.play(S_FART2);
  delay(500);
  Serial.println("FART3");
    cute.play(S_FART3);
  delay(500);
}
