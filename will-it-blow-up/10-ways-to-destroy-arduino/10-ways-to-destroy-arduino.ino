//10 Ways to Destroy An Arduino
//Don't try this at home
// 1. Put more than 20v into the barrel jack (ideally 9-12v)
// 2. Take more than 40ma from an output pin (ideally 20)
// 3. Take more than 400ma from all the pins
// 4. Shorting two output pins - Write one high and one low
// 5. Shorting an output pin to ground
// 6. Putting much more than 5v on an input pin
// 7. Short the VIN pin to ground
// 8. Apply more than 13v to the reset pin
// 9. Get it wet
// 10. Apply higher voltage to the 3.3v connector pin



void setup(){
  //let's take pins 3-13 and make them input pins
  for (int i = 3; i < 14; i++) {
    pinMode(i,INPUT);
  }

  //let's take all the pins 22-53 and make them outputs.
  for (int i = 22; i < 54; i++) {
    pinMode(i,OUTPUT);

    //now let's take all the EVEN pins and write them high (5v) and the ODD pins and write them low (ground)
    if ( (i % 2) != 0) {
      //odd pins only
      digitalWrite(i, LOW);
    }else{
      digitalWrite(i, HIGH);
    }
  }
}

void loop(){

}
