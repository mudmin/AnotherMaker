// https://youtube.com/AnotherMaker
// https://github.com/mudmin/AnotherMaker
//
//You will need the PubSubClient library

#include <Ethernet.h>
#include <PubSubClient.h>
#define DEVICE "DoorBellDuino"

const int doorbell = A15;           //current sensor connected to analog15 and ground
int senseDoorbell = 0;              //variable to hold doorbell sensor reading
int debounce = 5000;                //only allow one DingDong per 5 seconds
unsigned long currentMillis = 0;    //how many milliseconds since the Arduino booted
unsigned long prevRing = 0;         //The last time the doorbell rang
int request = 0;                    //A place to store incoming MQTT Requests for future use
char *cstring;                      //For parsing MQTT requests

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xFE };  //A random mac address generated for this arduino
IPAddress ip(192, 168, 95, 239);                          //The ip address that I'm assigning to the arduino
IPAddress server(192, 168, 95, 8);                        //The ip address of my local mqtt server

EthernetClient ethClient;
PubSubClient client(ethClient);

//reconnect to MQTT client and subscribe to the proper topic(s)
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(DEVICE)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // ... and resubscribe to look for future messages
      client.subscribe("doorbellIn");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//this function is here for future use when the arduino is receiving messages from the server
void callback(char* topic, byte* payload, unsigned int length) {
  for (int i=0;i<length;i++) {
    payload[length] = '\0';

    cstring = (char *) payload;
  }
    Serial.print("The topic received is...");
    Serial.println(topic);
    Serial.print("The message received is...");
    Serial.println(cstring);
}

void setup() {

   pinMode(4,OUTPUT);
   digitalWrite(4,HIGH);  // disable SD card if one in the slot

  Serial.begin(9600);
  pinMode(doorbell, INPUT);
  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
  Serial.println("DoorBellDuino Booted");
}



void loop() {
  if(!client.connected()) {
    reconnect();
  }
    client.loop(); //keep the MQTT dream alive

    //get the time since the arduino booted
    currentMillis = millis();
    //only check the doorbell if it hasn't been hit in the last 5 seconds
    if(currentMillis - prevRing >= debounce){
      senseDoorbell = analogRead(doorbell);  //read the doorbell sensor
      if(senseDoorbell > 50){                //mine read between 0 and 7 with no current and 200 with it.  50 seemed to be safe.
        client.publish("doorBell", "DingDong"); //Send the message over MQTT
        Serial.println("DingDong");
        prevRing = currentMillis;            //engage debounce mode!
      }
    }
}
