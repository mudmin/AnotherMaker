////////////////////////////
//Network Stuff is commented out because you really don't need that unless you're doing a connected escape room.
// #include <Ethernet.h>
// byte mac[]    = {  0xDE, 0xED, 0xBA, 0xED, 0xDE, 0xED };
// IPAddress ip(192, 168, 95, 205);  //of your arduino
// IPAddress server(192, 168, 95, 223);
// EthernetClient client;
////////////////////////////
int connected = 0;

// LED pin definitions
#define LED_GREEN  40
#define LED_BLUE   41
#define LED_PURPLE 42
#define LED_RED    43

#define LED_CORRECT 48
#define LED_WRONG   49

// Button pin definitions
#define BUTTON_PURPLE 24
#define BUTTON_GREEN  22
#define BUTTON_RED    23
#define BUTTON_BLUE   25


// Buzzer definitions
#define BUZZER  26
#define RED_TONE 220
#define GREEN_TONE 262
#define BLUE_TONE 330
#define PURPLE_TONE 392
#define TONE_DURATION 250



// Game Variables
int start = 27;
int readyLed = 28;
int resetRelay = 7;
int go = 0;
int GAME_SPEED = 400;
int GAME_STATUS = 0;
int const GAME_MAX_SEQUENCE = 10;
int GAME_SEQUENCE[GAME_MAX_SEQUENCE];
int GAME_STEP = 0;
int READ_STEP = 0;
int HDA_MAX = 7; //This is the number of steps to win!!

void setup(){
  randomSeed(analogRead(0));

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_PURPLE, OUTPUT);
  pinMode(LED_CORRECT, OUTPUT);
  pinMode(LED_WRONG, OUTPUT);
  pinMode(resetRelay, OUTPUT);
  digitalWrite(resetRelay, LOW);
  pinMode(readyLed, OUTPUT);
  digitalWrite(readyLed, LOW);
  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_GREEN, INPUT_PULLUP);
  pinMode(BUTTON_BLUE, INPUT_PULLUP);
  pinMode(BUTTON_PURPLE, INPUT_PULLUP);
  pinMode(start, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  Serial.begin(9600);
  // Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
  Serial.println("Booted");
}

void loop(){
  // //if you're not connected to the server, connect!
  // if (!client.connect(server, 80)){
  //   Serial.println("connecing");
  //   client.connect(server, 80);
  // }

  if(go == 0){
    digitalWrite(readyLed, HIGH);
    int check = digitalRead(start);
    if(check == LOW){
      go = 1;
      digitalWrite(readyLed, LOW);
      Serial.println("Start button pushed!");
      resetGame();
    }
  }

  if(go == 1){
  switch(GAME_STATUS){
    case 0:
      resetGame();
      break;
    case 1:
      playSequence();
      break;
    case 2:
      readSequence();
      break;
    case 3:
      gameOver();
      go = 0;
      break;
  }
}
}

void playSequence(){
  // play a step of our sequence
  for(int i=0; i<=GAME_STEP; i++){
    Serial.print("Set LED");
    Serial.println(GAME_SEQUENCE[i]);
    delay(GAME_SPEED*2);
    setLED(GAME_SEQUENCE[i]);
    playTone(GAME_SEQUENCE[i]);
    delay(GAME_SPEED);
    clearLEDs();

  }
  // Go to next step: reading buttons
  GAME_STATUS = 2;
}

void readSequence(){
  // read our buttons
  int button_value = readButtons();

  if(button_value > 0){
    // a button has been pressed
    if(button_value == GAME_SEQUENCE[READ_STEP]){
      // correct value!
      setLED(button_value);
      playTone(button_value);
      digitalWrite(LED_CORRECT, HIGH);
      delay(GAME_SPEED);
      clearLEDs();
      digitalWrite(LED_CORRECT, LOW);

      // Lets speed it up!
      GAME_SPEED = GAME_SPEED-7;

      Serial.println("Correct!");

      if(READ_STEP == GAME_STEP){
        // reset read step
        READ_STEP = 0;
        // Go to next game step
        GAME_STEP++;
        // Go to playback sequence mode of our game
        GAME_STATUS = 1;
        Serial.println("Go To Next Step");
        Serial.println(GAME_STEP);
        if(GAME_STEP >= HDA_MAX){ //did they get enough to win
          Serial.println("Winner!!!");
          // client.print("GET /hda/pick/simon.php?msg=done");
          // client.println(" HTTP/1.1");
          // client.println("Host: 192.168.95.223");
          // client.println("Connection: close");
          // client.println();

          delay(5000);
        }
        // Light all LEDs to indicate next sequence
        setLEDs(true,true,true,true);
        delay(GAME_SPEED);
        setLEDs(false,false,false,false);


      }else{
        READ_STEP++;
      }

      delay(10);

    }else{
      // wrong value!
      // Go to game over mode
      GAME_STATUS = 3;
      Serial.println("Game Over!");
      Serial.println("Fail!!!");
      // client.print("GET /hda/pick/simon.php?msg=fail");
      // client.println(" HTTP/1.1");
      // client.println("Host: 192.168.95.223");
      // client.println("Connection: close");
      // client.println();
        digitalWrite(resetRelay, LOW);
        delay(100);
        digitalWrite(resetRelay, HIGH);
    }
  }

  delay(10);

}

void resetGame(){
  // reset steps
  READ_STEP = 0;
  GAME_STEP = 0;

  // create random sequence
  for(int i=0; i<GAME_MAX_SEQUENCE; i++){
    GAME_SEQUENCE[i] = random(4) + 1;
  }

  // Go to next game state; show led sequence
  GAME_STATUS = 1;
}

void gameOver(){
  // Red RGB
  digitalWrite(LED_WRONG, HIGH);
  // Play Pwa Pwa Pwa
    tone(BUZZER, 98, TONE_DURATION);
    delay(TONE_DURATION);
    tone(BUZZER, 93, TONE_DURATION);
    delay(TONE_DURATION);
    tone(BUZZER, 87, TONE_DURATION);
    delay(TONE_DURATION);
    tone(BUZZER, 98, TONE_DURATION);
    delay(TONE_DURATION);
    tone(BUZZER, 93, TONE_DURATION);
    delay(TONE_DURATION);
    tone(BUZZER, 87, TONE_DURATION);
    delay(TONE_DURATION);  tone(BUZZER, 98, TONE_DURATION);
    delay(TONE_DURATION);
    tone(BUZZER, 93, TONE_DURATION);
    delay(TONE_DURATION);
    tone(BUZZER, 87, TONE_DURATION);
    delay(TONE_DURATION);  tone(BUZZER, 98, TONE_DURATION);
    delay(TONE_DURATION);
    tone(BUZZER, 93, TONE_DURATION);
    delay(TONE_DURATION);
    tone(BUZZER, 87, TONE_DURATION);
    delay(TONE_DURATION);
}

// HELPER FUNCTIONS
void setLED(int id){
  switch(id){
    case 0:
      setLEDs(false,false,false,false);
      break;
    case 1:
      setLEDs(true,false,false,false);
      break;
    case 2:
      setLEDs(false,true,false,false);
      break;
    case 3:
      setLEDs(false,false,true,false);
      break;
    case 4:
      setLEDs(false,false,false,true);
      break;
  }
}

void playTone(int id){
  switch(id){
    case 0:
      noTone(BUZZER);
      break;
    case 1:
      tone(BUZZER, RED_TONE, TONE_DURATION);
      break;
    case 2:
      tone(BUZZER, GREEN_TONE, TONE_DURATION);
      break;
    case 3:
      tone(BUZZER, BLUE_TONE, TONE_DURATION);
      break;
    case 4:
      tone(BUZZER, PURPLE_TONE, TONE_DURATION);
      break;
  }
}

void setLEDs(boolean red, boolean green, boolean blue, int PURPLE ){
  if (red) digitalWrite(LED_RED, HIGH);
  else digitalWrite(LED_RED, LOW);
  if (green) digitalWrite(LED_GREEN, HIGH);
  else digitalWrite(LED_GREEN, LOW);
  if (blue) digitalWrite(LED_BLUE, HIGH);
  else digitalWrite(LED_BLUE, LOW);
  if (PURPLE) digitalWrite(LED_PURPLE, HIGH);
  else digitalWrite(LED_PURPLE, LOW);
}

void clearLEDs(){
  setLEDs(false,false,false,false);
}

int readButtons(void){
  if (digitalRead(BUTTON_RED) == 0){Serial.println("1"); return 1;}
  else if (digitalRead(BUTTON_GREEN) == 0) {Serial.println("2"); return 2;}
  else if (digitalRead(BUTTON_BLUE) == 0) {Serial.println("3"); return 3; }
  else if (digitalRead(BUTTON_PURPLE) == 0) {Serial.println("4"); return 4; }

  return 0;
}
