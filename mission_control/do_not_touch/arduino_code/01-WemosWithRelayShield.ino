//12v Squirter by AnotherMaker
//youtube.com/AnotherMaker
//AnotherMaker.com

//This is a wemos d1 mini ( https://amzn.to/2U96z9u / https://www.banggood.com/custlink/K3DyemEzao )
//connected to a relay shield (https://amzn.to/2TdGwO8 / https://www.banggood.com/custlink/KmvhBvEUjg)
//set of both https://amzn.to/2TgCIf6

//The relay shield uses pin D1
//The relay is connected to 12v and a windshield washer pump ( https://amzn.to/3i5RnSC / https://www.banggood.com/custlink/KGGEeDRkBg )
//There is also an arcade button (https://amzn.to/3ejtoOy / https://www.banggood.com/custlink/KmvEg3EzOw ) connected to pin D2

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const int duration = 750; //how many milliseconds do you want to squirter to squirt
const int relay = D1;
const int button = D2;
const char* ssid = "YOUR SSID HERE ";
const char* password = "YOUR PASSWORD HERE";
IPAddress local_IP(192, 168, 95, 206); //What IP do you want this ESP to be on?
IPAddress gateway(192, 168, 95, 1); //What is your gateway (router) ip
IPAddress subnet(255, 255, 0, 0);

ESP8266WebServer server(80);

char relais;
char state;

void handleRoot() {
  server.send(200, "text/plain", "Ready");
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (int i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);

}


void setup(void){

  Serial.begin(9600);
  if (!WiFi.config(local_IP, gateway, subnet)) {
  Serial.println("Static IP Failed");
  }

  WiFi.begin(ssid, password);

  pinMode(button,INPUT_PULLUP);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

server.on("/squirt", [](){
    Serial.println("Webserver causing a squirt");
    server.send(200, "text/plain", "Success");
    digitalWrite(relay, HIGH);
    delay(duration);
    digitalWrite(relay, LOW);
});


  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  if(digitalRead(button) == LOW){
    Serial.println("Button causing a squirt");
    digitalWrite(relay, HIGH);
    delay(duration);
    digitalWrite(relay, LOW);
  }
  server.handleClient();
}
