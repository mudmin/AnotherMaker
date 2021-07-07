//Button Grid No Ethernet by AnotherMaker
//youtube.com/AnotherMaker
//AnotherMaker.com

//This is for an Arduino mega, but obviously you can change your pin numbers for other boards
//I'm using an ethernet shield and a server for my project, but I'll show you how to make
//a cool secret button grid just using the arduino itself.
//
//Of course, it's up to you to figure out what you want the arduino to do once the code is entered.

String secret;
const int secret_button = 32; //this is the "key" for entering secret codes
int listening = false;
int position = 0;

void setup() {
  for (int i = 3; i < 54; i++) {
    pinMode(i,INPUT_PULLUP);
  }
  Serial.begin(9600);

}

void loop(){
  for (int i = 3; i < 54; i++) {
    if(digitalRead(i) == LOW){
      Serial.print(i);
      Serial.println(" pressed!");

      //if you want certain buttons to do something, you can do an if/switch statement here such as
      if(i == secret_button){ //32
        Serial.println("That was the secret button!");
      }else if(i == secret_button + 1 || i == secret_button -1){
        Serial.println("That was close to the secret button");
      }else{
        Serial.println("That wasn't even close.");
      }


      //let's develop a simple code
      //if you hit the secret button, it's going to start storing next 4 buttons after it.
      //if those buttons match a pattern, then we're going to do something special.
      if(listening){
        secret = secret + i;
        position++;

        //once four buttons have been pressed after the secret button, then send it
        //to the "checkSecret" function below.
        //normally you would program that function to do something fun.
        //I'm just going to Serial.print something
        if(position >= 4){
          checkSecret(secret);
          position = 0;
          listening = false;
        }
      }

      //if the secret button is pressed, it's going to start appending the button numbers
      //to the "secret" variable.  Once four have been pressed, we'll check to see if that's a special
      //code
      if(i == secret_button){
        Serial.println("Listening to the next 4 button presses");
        listening = true;
        secret = ""; //clear the secret
        position = 0; //reset the position
      }

      delay(250); //slight debounce
    }
  }
}

bool checkSecret(String secret){
  //This function checks that string and sees if it's anything special. If it is, it does something
  //you can also add a final else statement to do something as a default.
  Serial.println("Checking Secret");
  Serial.print("I was given ");
  Serial.println(secret);

  if(secret == "22232425"){
    Serial.println("Party on! Buttons 22 through 25 pressed!");
  }else if(secret == "46474849"){
    Serial.println("Last 4 buttons pressed.  That's ");
    Serial.println("(wait for it)");
    delay(1500); //probably don't want this delay
    Serial.println("ImPRESSive");
  }else{
    Serial.println("Wah wah.  Wrong password.");
  }


}
