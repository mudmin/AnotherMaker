//https://youtube.com/anothermaker
//ESP32-POE-ISO example with enhanced diagnostics
//Simplified ethernet initialization

int board_id = 1;
const char* swver = "v1.0.6";

//This is the ESP32-POE-ISO from Olimex
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12

#include <ETH.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

static bool eth_connected = false;
unsigned long lastDiagnosticTime = 0;
const unsigned long DIAGNOSTIC_INTERVAL = 5000;

void printDiagnostics() {
    Serial.println("\n--- ESP32-POE-ISO Diagnostics ---");
    Serial.printf("Board ID: %d\n", board_id);
    Serial.printf("Software Version: %s\n", swver);
    Serial.printf("Ethernet Status: %s\n", eth_connected ? "Connected" : "Disconnected");
    Serial.printf("ETH Power Pin: %d (State: %d)\n", ETH_PHY_POWER, digitalRead(ETH_PHY_POWER));
    
    // Always try to print IP info regardless of eth_connected flag
    Serial.println("Attempting to read network info:");
    Serial.printf("MAC Address: %s\n", ETH.macAddress());
    Serial.printf("IP Address: %s\n", ETH.localIP().toString().c_str());
    Serial.printf("Subnet Mask: %s\n", ETH.subnetMask().toString().c_str());
    Serial.printf("Gateway: %s\n", ETH.gatewayIP().toString().c_str());
    
    if (eth_connected) {
        Serial.printf("MAC Address: %s\n", ETH.macAddress());
        Serial.printf("IP Address: %s\n", ETH.localIP().toString().c_str());
        Serial.printf("Subnet Mask: %s\n", ETH.subnetMask().toString().c_str());
        Serial.printf("Gateway: %s\n", ETH.gatewayIP().toString().c_str());
        Serial.printf("Link Speed: %d Mbps\n", ETH.linkSpeed());
        Serial.printf("Duplex Mode: %s\n", ETH.fullDuplex() ? "Full" : "Half");
    } else {
        Serial.println("Ethernet Connection Failed - Check:");
        Serial.println("1. Cable connection & LED activity");
        Serial.println("2. Power supply (PoE or DC)");
        Serial.println("3. ETH_PHY_POWER pin state should be HIGH");
    }
    
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.println("--------------------------------\n");
}

void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case SYSTEM_EVENT_ETH_START:
            Serial.println("ETH: Driver started");
            ETH.setHostname("esp32-ethernet");
            break;
            
        case SYSTEM_EVENT_ETH_CONNECTED:
            Serial.println("ETH: Physical layer connected");
            break;
            
        case SYSTEM_EVENT_ETH_GOT_IP:
            // Let the IP check in loop() handle the connection status
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
            break;
            
        case SYSTEM_EVENT_ETH_STOP:
            Serial.println("ETH: Driver stopped");
            break;
            
        default:
            break;
    }
}

void setup() {
    delay(250);  // Important delay for ethernet stability
    
    // Set ethernet power pin high explicitly
    pinMode(ETH_PHY_POWER, OUTPUT);
    digitalWrite(ETH_PHY_POWER, HIGH);
    
    Serial.begin(115200);
    Serial.println("\nESP32-POE-ISO Starting Up...");
    Serial.printf("Board ID: %d\n", board_id);
    Serial.printf("Software Version: %s\n", swver);
    
    lcd.begin();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Booting...");
    
    WiFi.onEvent(WiFiEvent);
    Serial.println("Starting Ethernet...");
    ETH.begin();
    
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
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("OTA Update");
        })
        .onEnd([]() {
            Serial.println("\nOTA: Update complete");
            lcd.setCursor(0,1);
            lcd.print("Complete!");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            unsigned int percentage = (progress / (total / 100));
            Serial.printf("OTA Progress: %u%%\r", percentage);
            lcd.setCursor(0,1);
            lcd.printf("Progress: %u%%", percentage);
        })
        .onError([](ota_error_t error) {
            Serial.printf("OTA Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });
        
    ArduinoOTA.begin();
    Serial.println("Setup Complete - Waiting for ethernet connection...");
}

static IPAddress lastIP;
void loop() {
    ArduinoOTA.handle();
    
    // Check for IP changes
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
        
        // Update LCD with new status
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.printf("ID:%d %s", board_id, swver);
        lcd.setCursor(0,1);
        if (eth_connected) {
            lcd.print(currentIP.toString());
        } else {
            lcd.print("No IP");
        }
    }
    
    unsigned long currentTime = millis();
    if (currentTime - lastDiagnosticTime >= DIAGNOSTIC_INTERVAL) {
        printDiagnostics();
        lastDiagnosticTime = currentTime;
    }
}