#include "Arduino.h"
#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', 'X', '3'},
  {'4', '5', 'X', '6'},
  {'7', '8', '9', 'X'},
  {'A', '0', 'B', 'X'}
};

int hangup = 2;
int onHook = 0;
int played = 0;

//color  ESP32 Pin / Arduino Pin / Label on Keypad
//orange 14/9/C1
//Purple 27/8/C2
//White 26/7/C3
//Brown 25/6/C3

// Blue 33/5/R1
// Yellow 32/4/R2
// Green 15/3/R3
// Gray 19/2/R4

//arduino uno
// byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
// byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad

// esp32
byte rowPins[ROWS] = {33, 32, 15, 19}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {14, 27, 26, 25}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//#include <SoftwareSerial.h>

/* Bytes are allocated:

   0  Fixed 0x7E Start Command
   1  Fixed 0xFF Version info ??
   2  Fixed 0x06 Data length not including parity
   3  Command code 0x?? number
   4  Acknowledge 01= yes, 00 = no, returns info
   5  Track high byte
   6  Track low byte
   7  Checksum high byte
   8  Checksum low byte
   9  Fixed 0xEF End Command

   Example:
      track number 100 (bytes 5 & 6) = 0x64 byte 6, 0x00 byte 5
      volume is set to max (30) by default, to set to half volume (15)
      send the following commands: 7E FF 06 06 00 00 0F FF D5 EF

   Checksum is calculated:
      -Sum(bytes(1 to 6)) note the minus sign

*/
//SoftwareSerial mp3(10, 11);
HardwareSerial mp3(2); //16,17

#define startByte 0x7E
#define endByte 0xEF
#define versionByte 0xFF
#define dataLength 0x06
#define infoReq 0x01
#define isDebug false

template<typename T>
void debugPrint(T printMe, bool newLine = false) {
  if (isDebug) {
    if (newLine) {
      Serial.println(printMe);
    }
    else {
      Serial.print(printMe);
    }
    Serial.flush();
  }
}

void setup() {

  // NB For comms to MP3 player not USB
  mp3.begin(9600);
  pinMode(hangup, INPUT_PULLUP);

  // Serial Monitor serial
  Serial.begin(9600);


  // Equaliser setting
  sendCommand(0x07, 0, 5);

  // Specify track to play (0 = first track)
  //  sendCommand(0x03, 0, 0);

  // Play
  //  sendCommand(0x0D, 0, 0);
  // Set volume (otherwise full blast!) command code 0x06 followed by high byte / low byte
  sendCommand(0x06, 0, 5);
}

void loop() {

//deal with playing a sound on initial pickup of the phone
  if(played == 0){
    int read = digitalRead(hangup);
    if(read == HIGH){
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 15); //play buzz sound
      played = 1;
  }
}

  if(played == 1){
    int read = digitalRead(hangup);
    if(read == LOW){
      played = 0;
      delay(100);//debounce
    }
  }

//only play sounds with phone off the hook
int loopRead = digitalRead(hangup);
if(loopRead == HIGH){
  char key = keypad.getKey();

  if (key != NO_KEY) {
    if (key == 49) { //1
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 01);
    }
    if (key == 50) { //2
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 02);
    }
    if (key == 51) { //3
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 03);
    }
    if (key == 52) { //4
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 04);
    }
    if (key == 53) { //5
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 05);
    }
    if (key == 54) { //6
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 06);
    }
    if (key == 55) { //7
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 07);
    }
    if (key == 56) { //8
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 10);
    }
    if (key == 57) { //9
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 11);
    }
    if (key == 48) { //0
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 12);
    }
    if (key == 65) { //A
      Serial.println("star");
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 13);
    }
    if (key == 66) { //B in place of *

      Serial.println("pound");
      sendCommand(0x06, 0, 5);
      sendCommand(0x12, 0, 14);
    }
  }
}//end off the hook check

  // Get Serial Monitor input
  if (Serial.available()) {
    byte Command = Serial.parseInt();
    byte Parameter1 = Serial.parseInt();
    byte Parameter2 = Serial.parseInt();

    // Execute that command
    Serial.print("Sending command: ");
    Serial.print(Command);
    Serial.print(",");
    Serial.print(Parameter1);
    Serial.print(",");
    Serial.println(Parameter2);
    sendCommand(Command, Parameter1, Parameter2);
  }

  // Get MP3 player response (always 10 bytes)
  if (mp3.available() >= 10) {
    byte Returned[10];
    for (byte k = 0; k < 10; k++)
      Returned[k] = mp3.read();

    Serial.print("Returned: 0x");
    if (Returned[3] < 16) Serial.print("0");
    Serial.print(Returned[3], HEX);
    Serial.print("(");
    Serial.print(Returned[3], DEC);
    Serial.print("); Parameter: 0x");
    if (Returned[5] < 16) Serial.print("0");
    Serial.print(Returned[5], HEX);
    Serial.print("(");
    Serial.print(Returned[5], DEC);
    Serial.print("), 0x");
    if (Returned[6] < 16) Serial.print("0");
    Serial.print(Returned[6], HEX);
    Serial.print("(");
    Serial.print(Returned[6], DEC);
    Serial.println(")");
  }
}

void sendCommand(byte Command, byte Param1, byte Param2) {

  // Calculate the checksum
  unsigned int checkSum = -(versionByte + dataLength + Command + infoReq + Param1 + Param2);

  // Construct the command line
  byte commandBuffer[10] = { startByte, versionByte, dataLength, Command, infoReq, Param1, Param2,
                             highByte(checkSum), lowByte(checkSum), endByte
                           };

  for (int cnt = 0; cnt < 10; cnt++) {
    mp3.write(commandBuffer[cnt]);
  }

  // Delay needed between successive commands
  delay(30);
}
