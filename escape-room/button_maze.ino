const int numBtns = 30;
int buttons[]  = {22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,11,12};
int pressed;
int math;


void setup(){
  Serial.begin(9600);
  Serial.println("Arduino Has Booted!");
  for (int thisBtn = 0; thisBtn < numBtns; thisBtn++) {
    pinMode(buttons[thisBtn], INPUT_PULLUP);
  }
}

void loop(){
  for (int thisBtn = 0; thisBtn < numBtns; thisBtn++) {
    // turn the pin on:
    pressed = digitalRead(buttons[thisBtn]);
    if(pressed == LOW){
      math = buttons[thisBtn] - 21;
      //override for compatibility reasons
      if(buttons[thisBtn] == 11){
          math = 29;
        }
        if(buttons[thisBtn] == 12){
          math = 30;
        }
        Serial.println(math);
        delay(600);
        }
      }
  }
