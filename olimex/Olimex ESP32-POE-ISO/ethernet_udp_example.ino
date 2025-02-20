#include <ETH.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

int board_id = 1;
const char* swver = "v1.0.2";

// Ethernet settings (assume network 192.168.<subnet>.<x>)
const byte mySubnet   = 88;
const byte myDeviceIP = 100;
const byte myServerIP = 23;
const int udpPort = 12345;

// Construct full IP addresses
IPAddress localIP(192, 168, mySubnet, myDeviceIP);
IPAddress gateway(192, 168, mySubnet, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
IPAddress udpServerIP(192, 168, mySubnet, myServerIP);

#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12

static bool eth_connected = false;
bool bootUdpSent = false;
unsigned long previousMillis = 0;
static IPAddress lastIP;

WiFiUDP udp;

void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case SYSTEM_EVENT_ETH_START:
            Serial.println("ETH: Driver started");
            ETH.setHostname("esp32-ethernet");
            ETH.config(localIP, gateway, subnetMask, primaryDNS, secondaryDNS);
            break;
            
        case SYSTEM_EVENT_ETH_CONNECTED:
            Serial.println("ETH: Physical layer connected");
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
            break;
            
        case SYSTEM_EVENT_ETH_DISCONNECTED:
            Serial.println("ETH: Physical layer disconnected");
            eth_connected = false;
            break;
            
        case SYSTEM_EVENT_ETH_STOP:
            Serial.println("ETH: Driver stopped");
            eth_connected = false;
            break;
            
        default:
            break;
    }
}

void setup() {
    delay(250);  // Important delay for ethernet stability
    
    Serial.begin(115200);
    Serial.println("\nESP32-POE-ISO Starting Up...");
    
    // Set ethernet power pin high explicitly
    pinMode(ETH_PHY_POWER, OUTPUT);
    digitalWrite(ETH_PHY_POWER, HIGH);
    delay(100);  // Give power pin time to stabilize
    
    WiFi.onEvent(WiFiEvent);
    
    Serial.println("Starting Ethernet...");
    ETH.begin();
    
    // Set up OTA update functionality using board_id as hostname
    char theboard_id[4];
    sprintf(theboard_id, "%d", board_id);
    ArduinoOTA.setHostname(theboard_id);
    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else
                type = "filesystem";
            Serial.println("Start updating " + type);
        })
        .onEnd([]() {
            Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });
    
    ArduinoOTA.begin();
    
    // Start UDP on the designated port
    udp.begin(udpPort);

    Serial.println("Setup Complete");
    Serial.print("Board ID: ");
    Serial.println(board_id);
    Serial.print("Software Version: ");
    Serial.println(swver);

    randomSeed(analogRead(0));
}

void loop() {
    ArduinoOTA.handle();
    
    // Check for IP changes and update connection status
    IPAddress currentIP = ETH.localIP();
    if (currentIP != lastIP) {
        if (currentIP != IPAddress(0, 0, 0, 0)) {
            eth_connected = true;
            Serial.print("Connected with IP: ");
        } else {
            eth_connected = false;
            Serial.print("IP Address changed to: ");
        }
        Serial.println(currentIP.toString());
        lastIP = currentIP;
    }

    // Once Ethernet is connected, send a boot UDP message (only once)
    if (eth_connected && !bootUdpSent) {
        Serial.println("Sending boot UDP message...");
        udp.beginPacket(udpServerIP, udpPort);
        udp.print("Boot message from board ");
        udp.print(board_id);
        udp.print(" swver ");
        udp.print(swver);
        udp.endPacket();
        bootUdpSent = true;
        previousMillis = millis();
        Serial.println("Boot UDP message sent");
    }

    // Every 5 seconds send a UDP message with a random number
    if (eth_connected && bootUdpSent && (millis() - previousMillis >= 5000)) {
        previousMillis = millis();
        long rnd = random(1000, 9999);
        Serial.print("Sending random UDP message: ");
        Serial.println(rnd);
        udp.beginPacket(udpServerIP, udpPort);
        udp.print("Random message: ");
        udp.print(rnd);
        udp.endPacket();
    }
}