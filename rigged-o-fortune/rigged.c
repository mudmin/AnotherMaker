#include <CheapStepper.h>
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

unsigned long currentTime = 0; //current time since the arduino has booted
unsigned long stopChecking = 0; //when to stop looking for second button
int ground = 9; //using pin D9 as another ground
int hall = 8; //hall effect sensor
int button = 7; //physical start button
CheapStepper stepper (3,4,5,6);
boolean spinning = false;
boolean moveWheel = false;
boolean moveClockwise = true;
boolean riggedTimeSet = false;
boolean rigged = false;
int phase = 0;
unsigned int rando = 0; //random number for steps

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  stepper.setRpm(14);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2 on nano,uno,mega
  //  mySwitch.enableReceive(1);  // Receiver on interrupt 1 => that is pin #3 on nano,uno,mega
  pinMode(9,OUTPUT);
  digitalWrite(9, LOW); //turn D9 into ground
  pinMode(hall,INPUT);
  pinMode(button,INPUT_PULLUP);

  Serial.println("Rigged-o-Fortune Booted");
}



void checkForButton(){ //look for a button press to get this party started
  if(digitalRead(button) == LOW){
    Serial.println("Spin Button Press Detected");
    moveWheel = true;
  }
}

void initialSpin(){ //do at least one full rotation and use this time to look for the rigged button press
  Serial.println("InitialSpin Function Called");
  phase = 1;
  stepper.newMove(moveClockwise, 8192);
}

void finalTurnRigged(){
  while(phase == 2){
    stepper.run();
      stepper.newMove(moveClockwise, 8192);
      Serial.println(digitalRead(hall));
      if(digitalRead(hall) == HIGH){
        Serial.println("Rigged Stop");
        phase = 3;
        stepper.stop();
        resetWheel();
        return false;
      }
  }
}


void finalTurnRando(){
   rando = random(0,4095);
   Serial.println("Spin NOT rigged");
   Serial.print("Moving ");
   Serial.println(rando);
   stepper.newMove(moveClockwise, rando);

   while(phase == 2){
      stepper.run();
      int stepsLeft = stepper.getStepsLeft();
      if(stepsLeft < 1){
        phase = 3;
        stepper.stop();
        resetWheel();
        return false;
      }
    }
  }


void resetWheel(){
  digitalWrite(3,LOW);
  digitalWrite(4,LOW);
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);
  phase=0;
  rigged = false;
  riggedTimeSet = false;
  moveWheel = false;
  spinning = false;
}

void spin(){
  if(spinning == false){
    initialSpin(); //start the wheel on its first rotation
    spinning = true;
  }

  if(riggedTimeSet == false){
    stopChecking = currentTime + 2500;  //When to stop looking for rigged button
    Serial.println("Beginning Rigged Check");
    Serial.print("Current Time: ");
    Serial.println(currentTime);
    Serial.print("Stop Time: ");
    Serial.println(stopChecking);

    riggedTimeSet = true;
  }

  if(currentTime <= stopChecking){
    if (mySwitch.available()) {
    unsigned long rec = mySwitch.getReceivedValue();
    if(rec == 724385){
      rigged = true;
    }
    mySwitch.resetAvailable();
  }
  }
  int stepsLeft = stepper.getStepsLeft();

  if(phase == 1 && stepsLeft == 0){
    phase = 2;
    Serial.println("Moving to Phase 2");
  }
  if(currentTime >= stopChecking && phase == 2 && stepsLeft == 0){
    Serial.println("Rigged Check Complete - Starting Final Turn");
    if(rigged == true){
      finalTurnRigged();
    }else{
      finalTurnRando();
    }
  }

}


void loop() {
  currentTime = millis();
  stepper.run();
  int stepsLeft = stepper.getStepsLeft();
  if(moveWheel == false){
    checkForButton();
  }else{
    spin();
  }
    if (mySwitch.available()) {
        if(currentTime <= stopChecking){
    unsigned long rec = mySwitch.getReceivedValue();
    if(rec == 724385){
      Serial.println("RIGGED keypress!!!");
      rigged = true;
    }
    mySwitch.resetAvailable();

  }
  }

}
