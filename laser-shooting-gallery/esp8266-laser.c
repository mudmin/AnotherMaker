/*
Do whatever you want with this, but BE CAREFUL.  I'm not responsible. You'll shoot someone's eye out.
This is built on a Wemos D1 Mini
Since the Wemos output pins are 3v, I'm using a transistor to switch the 5v on the board on/off to fire the laser
Nearly any NPN Transistor would work, but I'm using a 2n222
When you're looking at the "flat" face of the transistor...

Left(1) - Emittor
Center(2) - Base
Right(3) - Collector
The wiring goes like this...
1. 5v pin on the wemos to the +(S) side of the laser
2. - Side of the laser to the Collector (right) side of the transistor.
3. Both one wire of the trigger AND the Emittor (left) side of the trasnistor go to Wemos Ground
4. The Base (center) of the transistor goes to pin D2 (AKA GPIO 4) on the Wemos
5. Pin D1 (AKA GPIO 5) goes to the other side of the pushbutton

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#define DEVICE "PewPew"   //This MUST be unique
int laser = 4;            //pin d2
int button = 5;           //pin d1
int fired = 0;            //Debounce and force people to take their finger off the button to fire again
int reading = 0;          //reading the button
int mode = 0;             //for future use
int rateLimit = 750;     //how often can you send a pew pew sound request
int fireDuration = 100;   //how long to activate the laser (in milliseconds)
unsigned long currentMillis = millis();
unsigned long rateLimitOver = 0; //When am I allowed to send another MQTT message requesting a pew
unsigned long canFireAgain  = 0; //When am I allowed to send another laser burst

int id = 59;              //serves as the final ip address and unique identifier
int subnet = 95;          //the second to last set of numbers in your ip addresses 192.168.SUBNET.xxx
int serverip = 148;       //what is the last part of the ip address of your nodered/mqtt server 192.168.SUBNET.SERVERIP
const char* ssid = "YOUR_SSID_HERE";
const char* password = "YOUR_WIFI_PASSWORD_HERE";

//Networking / MQTT Variables that usually don't need to be changed
WiFiClient ethClient;
PubSubClient client(ethClient);
char *cstring;
IPAddress arduino_ip ( 192,  168,   subnet,  id);
IPAddress dns_ip     (  8,   8,   8,   8);
IPAddress gateway_ip ( 192,  168,   subnet,   1);
IPAddress subnet_mask(255, 255, 255,   0);
IPAddress server(192, 168, subnet, serverip);
int request = 0;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(DEVICE)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("shooting/lasercheck", DEVICE);
      // ... and resubscribe
      // client.subscribe("relays/#");


    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 20 seconds");
      // Wait 20 seconds before retrying
      delay(20000);
    }
  }
}



void setup() {
  Serial.begin(57600);
  pinMode(laser, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  client.setServer(server, 1883);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.config(arduino_ip, gateway_ip, subnet_mask);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Booted");
  delay(1500);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
      }
  client.loop();

  currentMillis = millis();  //get the current time of this loop


  reading = digitalRead(button);      //find the state of the button
  if(currentMillis >= canFireAgain){  //check and see if you should turn the laser off and allow refire
  digitalWrite(laser, LOW);
  if(fired == 0){                     //Only if the trigger has been released
      if(reading == LOW){
          Serial.println("firing");
          digitalWrite(laser, HIGH);

          canFireAgain = currentMillis + fireDuration;
          if(currentMillis > rateLimitOver){
            client.publish("shooting/pew", "1");
            rateLimitOver = currentMillis + rateLimit;
          }
          fired = 1;
      }
    }
  }


  if(fired == 1){
    reading = digitalRead(button);
    if(reading == HIGH){
      Serial.println("resetting");
      fired = 0;
    }
  }
}
