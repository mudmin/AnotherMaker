//Demo code for this board https://www.pcbway.com/project/shareproject/Olimex_POE_ISO_Breakout_Board_Mount_cede76cb.html
//If you're interested in getting pcbs, using this link https://www.pcbway.com/setinvite.aspx?inviteid=446230 

//build notes
// L2 LED is Red, L3 is Yellow, L4 is Green
// Footprints are incorrect and the long pin of the led goes towards the square pad
// The footprints next to the leds are for jumpers to enable the leds. You can just short a wire if you always want them to be available.
// for the resistors, I used 330 ohm for red and yellow and probably should have used 220 ohm for green
// diodes are 1N4007
// transistors are BC547GC a 2n2222 may work but your mileage may vary

#include <ETH.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Ethernet settings
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12

// Pin definitions
const int PIN_RED_LED = 2;
const int PIN_YELLOW_LED = 4;
const int PIN_GREEN_LED = 5;
const int PIN_RELAY1 = 13;
const int PIN_RELAY2 = 15;

// Global variables
static bool eth_connected = false;
WebServer server(80);

// Structure to store pin states
struct PinState {
  int pin;
  bool state;
  const char* name;
  const char* type;
};

// Array to store all controlled pins
PinState pins[] = {
  {PIN_RED_LED, false, "Red", "led"},
  {PIN_YELLOW_LED, false, "Yellow", "led"},
  {PIN_GREEN_LED, false, "Green", "led"},
  {PIN_RELAY1, false, "Relay 1", "relay"},
  {PIN_RELAY2, false, "Relay 2", "relay"}
};

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      ETH.setHostname("esp32-gpio");
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

void setup() {
  Serial.begin(115200);
  
  // Initialize all pins
  for (auto &pin : pins) {
    pinMode(pin.pin, OUTPUT);
    digitalWrite(pin.pin, pin.state ? HIGH : LOW);
  }

  // Initialize Ethernet
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  
  // Set static IP
  IPAddress ip(192, 168, 88, 22);
  IPAddress gateway(192, 168, 88, 1);
  IPAddress subnet(255, 255, 255, 0);
  ETH.config(ip, gateway, subnet);

  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/pins", HTTP_GET, handleGetPins);
  server.on("/api/pin", HTTP_POST, handleSetPin);
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Olimex Control Panel</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { 
          font-family: Arial; 
          margin: 0; 
          padding: 20px; 
          background-color: #f0f2f5;
        }
        .container {
          max-width: 800px;
          margin: 0 auto;
        }
        h1 {
          color: #1a1a1a;
          text-align: center;
          margin-bottom: 30px;
        }
        .card {
          background: white;
          border-radius: 10px;
          padding: 20px;
          margin-bottom: 20px;
          box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .card-title {
          color: #444;
          margin: 0 0 20px 0;
          padding-bottom: 10px;
          border-bottom: 1px solid #eee;
        }
        .controls {
          display: grid;
          grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
          gap: 20px;
        }
        .control-item {
          display: flex;
          align-items: center;
          padding: 15px;
          background: #f8f9fa;
          border-radius: 8px;
          position: relative;
        }
        .status-indicator {
          width: 12px;
          height: 12px;
          border-radius: 50%;
          margin-right: 12px;
          transition: all 0.3s ease;
        }
        .status-indicator.led {
          box-shadow: 0 0 10px rgba(0,0,0,0.1);
        }
        .status-indicator.led.on {
          box-shadow: 0 0 15px rgba(255,255,0,0.5);
        }
        .status-indicator.relay {
          border: 2px solid #ccc;
        }
        .status-indicator.on.relay {
          background-color: #28a745;
          border-color: #28a745;
        }
        .status-indicator.off {
          background-color: #808080;
        }
        .status-indicator.led.on[data-color="Red"] {
          background-color: #dc3545;
        }
        .status-indicator.led.on[data-color="Green"] {
          background-color: #28a745;
        }
        .status-indicator.led.on[data-color="Yellow"] {
          background-color: #ffc107;
        }
        .status-indicator.relay.on {
          background-color: #28a745;
        }
        button {
          margin-left: auto;
          padding: 8px 16px;
          border: none;
          border-radius: 4px;
          background: #007bff;
          color: white;
          cursor: pointer;
          transition: background 0.3s ease;
        }
        button:hover {
          background: #0056b3;
        }
        .name {
          font-weight: 500;
          margin-right: 10px;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>ESP32 Control Panel</h1>
        <div class="card">
          <h2 class="card-title">LED Controls</h2>
          <div id="led-controls" class="controls"></div>
        </div>
        <div class="card">
          <h2 class="card-title">Relay Controls</h2>
          <div id="relay-controls" class="controls"></div>
        </div>
      </div>
      <script>
        function updatePins() {
          fetch('/api/pins')
            .then(response => response.json())
            .then(data => {
              const ledControls = document.getElementById('led-controls');
              const relayControls = document.getElementById('relay-controls');
              
              ledControls.innerHTML = '';
              relayControls.innerHTML = '';
              
              data.forEach(pin => {
                const div = document.createElement('div');
                div.className = 'control-item';
                const indicator = document.createElement('div');
                indicator.className = 'status-indicator ' + pin.type + ' ' + (pin.state ? 'on' : 'off');
                indicator.setAttribute('data-color', pin.name);
                
                div.innerHTML = indicator.outerHTML + 
                               '<span class=\"name\">' + pin.name + '</span>' +
                               '<button onclick=\"togglePin(' + pin.pin + ',' + !pin.state + 
                               ')\">Turn ' + (pin.state ? 'Off' : 'On') + '</button>';
                
                if (pin.type === 'led') {
                  ledControls.appendChild(div);
                } else {
                  relayControls.appendChild(div);
                }
              });
            });
        }

        function togglePin(pin, state) {
          fetch('/api/pin', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({pin: pin, state: state})
          }).then(() => updatePins());
        }

        // Update status every 2 seconds
        updatePins();
        setInterval(updatePins, 2000);
      </script>
    </body>
    </html>
  )";
  server.send(200, "text/html", html);
}

void handleGetPins() {
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();
  
  for (auto &pin : pins) {
    JsonObject pinObj = array.createNestedObject();
    pinObj["pin"] = pin.pin;
    pinObj["state"] = pin.state;
    pinObj["name"] = pin.name;
    pinObj["type"] = pin.type;
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSetPin() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
  if (error) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  int pin = doc["pin"];
  bool state = doc["state"];
  
  // Find and update pin state
  for (auto &p : pins) {
    if (p.pin == pin) {
      p.state = state;
      digitalWrite(pin, state ? HIGH : LOW);
      break;
    }
  }
  
  server.send(200, "text/plain", "OK");
}