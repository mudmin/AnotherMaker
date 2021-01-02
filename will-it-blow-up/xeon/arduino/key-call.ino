#include <Ethernet.h>
#include <SPI.h>
#include <ArduinoJson.h> //v6.11.5 from Library Manager

const int port = 80;
const char* server = "192.168.95.199";
const char* api = "GET /wibu/key/?key=12345-67890-12345-67890-12345 HTTP/1.0";

EthernetClient client;
void setup() {
  // Initialize Serial port
  Serial.begin(9600);
  Serial.println("Beginning boot process");
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
  // delay(10000);
  // no delay means the arduino hits the API as fast as it possibly can
}

void apiCall(){
  if (!client.connect(server, port)) {
    Serial.println(F("Connection failed"));
    return;
  }

  Serial.println(F("Connected!"));

  // Send HTTP request
  client.println(api);
  client.println(F("Host: test.org"));
  client.println(F("Connection: close"));
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    return;
  }
  //
  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }


  client.stop();
}
