#include <Ethernet.h>
#include <SPI.h>
#include <ArduinoJson.h> //v6.11.5 from Library Manager

// //Node-Red API Relay
const int port = 80;
const char* server = "192.168.95.9";
const char* api = "GET /api/date/? HTTP/1.0";


// // PHP API Relay
// //note the 4414 is there because my php file is in c:\xampp\htdocs\4414
// const int port = 80;
// const char* server = "192.168.95.148";
// const char* api = "GET /4414/apirelay.php HTTP/1.0";

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
  // delay(10000);
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

  // Disconnect
  // client.stop();
}
