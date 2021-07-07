//Button Grid With Ethernet by AnotherMaker
//youtube.com/AnotherMaker
//AnotherMaker.com

//This is for an Arduino mega, but obviously you can change your pin numbers for other boards

//Rather than constantly reprogramming the Arduino to add new features, I'm going to have the
//Ethernet shield hit a simple API on my computer that can play sound effects and process the
//secret. If you don't have a shield or don't want to do it this way, please use example 02.
#include <Ethernet.h>
#include <SPI.h>

//In this simple example, I have a server running xampp at 192.168.1.9
//There is a folder called /missioncontrol/api in my htdocs folder
//with an api file called index.php
//Note: In the server file there is a decently documented index.php with some sample sound files.
//It has instructions for win/mac/linux
// Here's a video demo of the mp3 player https://www.youtube.com/watch?v=PJ2Ag5xLjIU
const char* server = "192.168.1.9";
const int port = 80;
// const char* api = "GET /missioncontrol/api HTTP/1.0";
String api = "GET /missioncontrol/api/";
String api_http = "HTTP/1.0";
String query = "";
String secret;
EthernetClient client;

const int secret_button = 32; //this is the "key" for entering secret codes
int listening = false;
int position = 0;


void setup() {
  for (int i = 19; i < 50; i++) {
    pinMode(i,INPUT_PULLUP);
  }
  Serial.begin(9600);

  // Initialize Ethernet library
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  if (!Ethernet.begin(mac)) {
    Serial.println(F("Failed to configure Ethernet"));
    return;
  }
  delay(1500);

}

void loop(){
  for (int i = 19; i < 50; i++) {
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
  if (!client.connect(server, port)) {
    Serial.println(F("Connection failed"));
    return;
  }

  Serial.println(F("Connected!"));

  // Send HTTP request by building out the url the easy way.
  query = api + "?secret=";
  query = query + secret;
  query = query + api_http;
  Serial.println(query);
  client.println(query);
  client.println(F("Host: DoesNotMatter.org"));
  client.println(F("Connection: close"));
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }else{
    Serial.print(F("Success. Response: "));
    Serial.println(status);
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }

  // Disconnect
  client.stop();
}
