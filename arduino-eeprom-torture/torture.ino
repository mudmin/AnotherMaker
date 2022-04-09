#include <EEPROM.h>

#define HALT while(true){}

// test paramiters
#define TEST_SIZE 256
#define PROMPT_COUNT 100

// global values
uint8_t gRead = 0; // an 8 bit global to to store the read value
uint8_t gState = 0; // an 8 bit global state value to be written and compaired
unsigned long gCounter = 0;

// Prompt test setup
void setup()
{
  Serial.begin(115200);
  Serial.print("Booted EEPROM Size: ");
  Serial.print(EEPROM.length())
  Serial.print(" Test size: ");
  Serial.print(TEST_SIZE);
  serial.print(" Prompt every: ");
  serial.print(PROMPT_COUNT);
  serial.println(" Test cycles");
  delay(500); //for (less) drama 
}

// main loop
void loop () {

  // Flip the state bits
  gState = ~gState;
 
 // Display the test number every PROMPT_COUNT cycles
  if(gCounter % PROMPT_COUNT == 0) {
    Serial.print("Test No. ");
    Serial.println(gCounter + 1);
  }
    //write TEST_LENGTH bytes to the EEPROM
    for (int i = 0; i < TEST_SIZE; i++){
      EEPROM.write(i, gState); //address, value
    }

    //read it back and make sure you're getting the right value
    for (int i = 0; i < TEST_SIZE; i++){
      gRead = EEPROM.read(i); //address
      if(gRead != gState){
        Serial.print("Failed Test No. ");
        Serial.print(gCounter + 1);
        Serial.print("Read Error at: ");
        Serial.print(i);
        Serial.print(" State Value: ");
        Serial.print(gState);
        Serial.print(" Read value: ");
        Serial.print(gRead);
        Serial.print(", Halting");

       // halt
        HALT
      }
    }
    gCounter++;
} // main loop
