// NOTE:
// GPIO
// 6-11 are used in the flash process and shouldn't be used.
// 34-39 are input only

//Special thanks to these 3 Youtubers who kept me sane when I had several weird issues on this project
//Simple Electronics - https://www.youtube.com/channel/UC3yasGCIt1rmKnZ8PukocSw
//Gadget Reboot - https://www.youtube.com/channel/UCwiKHTegfDe33K5wnmyULog
//Pile of Stuff - https://www.youtube.com/user/pileofstuff


#include "WiFi.h"
#include <PubSubClient.h>

//Initial configuration
#define DEVICE "cowboy" //This MUST be unique
char* mp3 = "cowboy1.mp3|cowboy2.mp3"; //The mp3 when activated then when hit
int id = 61;       //serves as the final ip address and unique identifier
int subnet = 95;   //the second to last set of numbers in your ip addresses 192.168.SUBNET.xxx
int serverip = 148;       //what is the last part of the ip address of your nodered/mqtt server 192.168.SUBNET.SERVERIP
const char* ssid = "YOUR_SSID_HERE";
const char* password = "YOUR_WIFI_PASSWORD_HERE";

//IO Variables
int powerLed = 25;    //green
int shootMeLed = 26;  //yellow
int hitLed = 27;      //red
int sensor = 14;      //laser sensor

//State Variables
int active = 0;                 //By default, should not accept input
int reading = 0;                //Store your sensor readinging
char me[10];                    //my id to be used in mqtt messages
char mp3me[32];                 //my mp3 request for setup
char avme[32];                  //activation request topic

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


//GamePlay functions
void activate(unsigned long duration){
  unsigned long currentMillis = millis();
  Serial.println("Looking for input");
  //it's not obvious, but this next line will NOT keep looping. Only the stuff in the while loop will.
  unsigned long stopAt = currentMillis + duration;

  while(active == 1){
  currentMillis = millis();
  if (currentMillis <= stopAt) {
    //tell the world you're active
    digitalWrite(shootMeLed, HIGH);

    //read the sensor
    reading = digitalRead(sensor);
    if(reading == HIGH){
        iveBeenHit();
    }
  }else{
    //time is up!
    deactivate();
    client.publish("shooting/miss", "1");
    miss();
  }
  }
}

void deactivate(){
  //stop looking for input and go dark
  digitalWrite(shootMeLed, LOW);
  active = 0;
  Serial.println("Deactivating sensor");
}

void iveBeenHit(){
  deactivate();
  client.publish("shooting/hit", me);
  Serial.println("I have been hit!!!");
}

void miss(){
  //doing nothing right now
}

void callback(char* topic, byte* payload, unsigned int length) {
  for (int i = 0; i < length; i++) {
    payload[length] = '\0';
    cstring = (char *) payload;
  }

  request = atof(cstring);
  Serial.println("msg rec");
  if (strstr(topic, "whodere") != NULL) {
    request = atof(cstring);
    Serial.println("The request is...");
    Serial.println(request);
    if(request == 1){ //tell the server you're there so you can be added to the target list
      // sprintf(msg,"%d",id);
      Serial.println("WhoDere Request Received....responding.");
      client.publish("shooting/response", mp3me);
    }

  }
  if (strstr(topic, "activate") != NULL) {
    request = atof(cstring);
    Serial.println("An activation request has been sent out");
    Serial.println("I have been requested to activate for ");
    unsigned long dur = request;
    Serial.print(dur);
    Serial.println(" milliseconds");
    active = 1;
    activate(dur);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(DEVICE)) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("shooting/whodere/#");
      client.subscribe(avme);
      // client.subscribe(subscription.c_str());
      // client.subscribe(reset.c_str());

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 20 seconds");
      // Wait 20 seconds before retrying
      delay(20000);
    }
  }
}

void setup()
{

  sprintf(me,"%d",id);
  sprintf(avme,"shooting/activate%d/#",id);
  sprintf(mp3me,"%d|%s",id,mp3);
  Serial.begin(57600);
  Serial.print("My id is ");
  Serial.println(id);
  Serial.print("My activation topic is ");
  Serial.println(avme);
  //establish mqtt connections
  client.setServer(server, 1883);
  client.setCallback(callback);

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
  // Allow the hardware to sort itself out
  pinMode(powerLed,OUTPUT);
  pinMode(shootMeLed,OUTPUT);
  pinMode(hitLed,OUTPUT);
  pinMode(sensor,INPUT);
  delay(1500);
}


void loop()
{
  if (!client.connected()) {
    reconnect();
      }
  client.loop();
  digitalWrite(powerLed,HIGH);
}
