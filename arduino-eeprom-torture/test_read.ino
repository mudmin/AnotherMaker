//This just reads the rom one time to see if you got what appears to be valid data.
//if you cut the power mid cycle on a torture test, you should get a combo of ones and 0s.
//Otherwise, you will get all the same.
//Just checking to make sure my results seem valid
#include <EEPROM.h>
//for loop
int a = 1;
int b = 1;
int read = 0; // a place to store our read



void setup(){
  Serial.begin(9600);
}

void loop(){
  while(a == b){
    for (int i = 0; i < 255; i++){
      read = EEPROM.read(i); //address
      Serial.println(read);
    }
    b = 0;
  }
}
