//////////////////////////////////////////////////
//  AnotherMaker https://youtube.com/AnotherMaker
//with many contributions from Rui Santos
//  Complete project details at https://randomnerdtutorials.com
//////////////////////////////////////////////////



//////////////////////////////////////////////////
// // Comment this section in for WIFI
// // Load Wi-Fi library
// #include <WiFi.h>
// //Replace with your network credentials
// const char* ssid = "YOUR_SSID_HERE";
// const char* password = "YOUR_PASSWORD_HERE";
// //
//////////////////////////////////////////////////


//////////////////////////////////////////////////
//// Comment this section in for ETHERNET
//// This is for the Olimex Ethernet boards. You may
//// need to change a few settings
//// Load Ethernet Library
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12
#include <ETH.h>
static bool eth_connected = false;
uint64_t chipid;
//ESP32 Specific Ethernet function
void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}
//
//////////////////////////////////////////////////

//Although we're using the term wifi a lot, that's just to keep the
//code the same as a standard ESP32

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String relay1State = "off";
String relay2State = "off";

// Assign output variables to GPIO pins
const int relay1 = 32;
const int relay2 = 33;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(9600);
  // Initialize the output variables as outputs
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  // Set outputs to LOW
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);


//////////////////////////////////////////////////
// Comment this section in for WIFI
//   // Connect to Wi-Fi network with SSID and password
//   Serial.print("Connecting to ");
//   Serial.println(ssid);
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//    Serial.println("");
//    Serial.println("Wifi connected.");
// //  Print local IP address
//   Serial.println("IP address: ");
//   Serial.println(WiFi.localIP());
//////////////////////////////////////////////////

//////////////////////////////////////////////////
////Comment this section in for Ethernet
  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.

  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  Serial.println("");
  Serial.println("Ethernet connected.");
//
//////////////////////////////////////////////////

  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /relay1/on") >= 0) {
              Serial.println("RELAY 1 on");
              relay1State = "on";
              digitalWrite(relay1, HIGH);
            } else if (header.indexOf("GET /relay1/off") >= 0) {
              Serial.println("RELAY 1 off");
              relay1State = "off";
              digitalWrite(relay1, LOW);
            } else if (header.indexOf("GET /relay2/on") >= 0) {
              Serial.println("RELAY 2 on");
              relay2State = "on";
              digitalWrite(relay2, HIGH);
            } else if (header.indexOf("GET /relay2/off") >= 0) {
              Serial.println("RELAY 2 off");
              relay2State = "off";
              digitalWrite(relay2, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>Olimex ESP32-EVB</h1>");

            // Display current state, and ON/OFF buttons for RELAY 1
            client.println("<p>RELAY 1 - State " + relay1State + "</p>");
            // If the relay1State is off, it displays the ON button
            if (relay1State=="off") {
              client.println("<p><a href=\"/relay1/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/relay1/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Display current state, and ON/OFF buttons for RELAY 2
            client.println("<p>RELAY 2 - State " + relay2State + "</p>");
            // If the relay2State is off, it displays the ON button
            if (relay2State=="off") {
              client.println("<p><a href=\"/relay2/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/relay2/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
