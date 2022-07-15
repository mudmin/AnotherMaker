//https://youtube.com/anothermaker
//very basic Olimex ESP32-POE-ISO example sketch with OTA update, ethernet and 1602 LCD

//give this board a unique int id for OTA
int board_id = 1;

//I also like having a software version that prints to the screen
const char* swver = "v1.0.2";

//This is the ESP32-POE-ISO from Olimex
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12

#include <ETH.h>
#include <LiquidCrystal_I2C.h> // Library for LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows


#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:

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


void setup(){
  //don't remove this delay, it helps fix the ethernet
  delay(250);
  Serial.begin(115200);
  lcd.begin(); //initialize the lcd
  lcd.backlight(); //open the backlight
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Booting...");
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  char theboard_id[4];
  sprintf(theboard_id, "%d", board_id);
  ArduinoOTA.setHostname(theboard_id);
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
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

  Serial.println("Booted");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ESP32# ");
  lcd.print(board_id);
  lcd.setCursor(0,1);
  lcd.print(swver);

}


void loop(){
  ArduinoOTA.handle();
}
