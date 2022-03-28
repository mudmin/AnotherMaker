#include <EEPROM.h>
//for loop
int a = 1;
int b = 1;

int read = 0; // a place to store our read
int step = 0; // either 0 or 1
unsigned long counter = 0;


void setup()
{
 Serial.begin(9600);
 Serial.println("Booted");
 delay(5000); //for drama
}

void loop(){
  while(a == b){ //until it fails

    //speed things up by cutting doing 1/128th of the writing
    for (int i = 0; i < 2; i++){
      EEPROM.write(i, step); //address, value
    }

    //read it back and make sure you're getting the right value
    for (int i = 0; i < 2; i++){
      read = EEPROM.read(i); //address
      if(read != step){
        Serial.println("FAIIIIIIIIIIIIIIIIIIIIIL");
        b = 0; //kill the loop
      }
    }
    if(step == 0){
      step = 1;
    }else{
      step = 0;
    }

    counter++;
    Serial.print("Round ");
    Serial.print(counter);
    Serial.print(" successful - Step ");
    Serial.println(step);


    // //do a test run of just a few times time
    //   if(counter >= 7){
    //   b = 8;
    // }

  }


}
