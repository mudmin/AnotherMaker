//This is an example of sending DHT11 temp/humidity data over a simple api for data logging
//The real purpose of the data logging is to detect when the EnergyDuino stops working.

#include <Ethernet.h>
#include <SPI.h>
#include <ArduinoJson.h> //v6.11.5 from Library Manager
#include <dht.h>
dht DHT;
#define DHT11_PIN 7

// //Node-Red API Relay
// const int port = 1880;
// const char* server = "192.168.95.8";

// const char* api = "GET /apiCalls HTTP/1.0";


const int port = 80;
const char* server = "192.168.95.198";
// const char* api = "GET /527/energyduino.php HTTP/1.0";


EthernetClient client;
void setup() {
  // Initialize Serial port
  Serial.begin(9600);
  while (!Serial) continue;

  // Initialize Ethernet library
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  if (!Ethernet.begin(mac)) {
    Serial.println(F("Failed to configure Ethernet"));
    return;
  }
  delay(1000);

  Serial.println(F("Connecting..."));
}

void loop() {
  apiCall();
  delay(10000);
}

void apiCall(){
  if (!client.connect(server, port)) {
    Serial.println(F("Connection failed"));
    return;
  }
  Serial.println(F("Connected!"));

  //my api happens to be in c:/xampp/htdocs/527 on my pc
  String api = "GET /527/energyduino.php?temp=";

  int chk = DHT.read11(DHT11_PIN);
  Serial.print("Temperature = ");
  Serial.println(DHT.temperature);
  Serial.print("Humidity = ");
  Serial.println(DHT.humidity);

  //let's build out that API string for the URL
  api = api + DHT.temperature;
  api = api + "&hum=";
  api = api + DHT.humidity;

  // Send HTTP request
  client.println(api);
  client.println(F("Host: test.org"));
  client.println(F("Connection: close"));
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Response: "));
    Serial.println(status);
    return;
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
