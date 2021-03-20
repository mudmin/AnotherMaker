/*
  Very rough example of how to send your lat/lon to a webhook/api via get request.
  See my other videos at https://youtube.com/anothermaker for other esp32 API Examples or look in the
  api-calls folder of this repository
  FILE: AllFunctions.ino
  AUTHOR: Koby Hale
  PURPOSE: Test functionality
*/

#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

#define TINY_GSM_DEBUG SerialMon
// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

/*
   Tests enabled
*/
// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false


#define TINY_GSM_TEST_GPRS    true
#define TINY_GSM_TEST_GPS     true
#define TINY_GSM_POWERDOWN    true

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[]  = "h2g2";     //SET TO YOUR APN
const char gprsUser[] = "";
const char gprsPass[] = "";

// Server details
const char server[] = "yourdomain_with_nohttp.com"; //set the rest of the api call below
const int  port = 80;

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

#include <SPI.h>
#include <SD.h>
#include <Ticker.h>

#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
TinyGsmClient client(modem);
HttpClient http(client, server, port);
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  60          // Time ESP32 will go to sleep (in seconds)

#define UART_BAUD   9600
#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4

#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13
#define LED_PIN     12


int counter, lastIndex, numberOfPieces = 24;
String pieces[24], input;


void setup() {
  // Set console baud rate
  Serial.begin(9600);
  delay(10);

  // Set LED OFF
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);
  delay(300);
  digitalWrite(PWR_PIN, LOW);

  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SDCard MOUNT FAIL");
  } else {
    uint32_t cardSize = SD.cardSize() / (1024 * 1024);
    String str = "SDCard Size: " + String(cardSize) + "MB";
    Serial.println(str);
  }

  Serial.println("\nWait...");

  delay(10000);

  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  // Set GSM module baud rate
  // TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  // SerialAT.begin(9600);
  delay(6000);
  modem.simUnlock(GSM_PIN);
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  if (!modem.restart()) {
    Serial.println("Failed to restart modem, attempting to continue without restarting");
  }

  String name = modem.getModemName();
  delay(500);
  Serial.println("Modem Name: " + name);

  String modemInfo = modem.getModemInfo();
  delay(500);
  Serial.println("Modem Info: " + modemInfo);

  // Set SIM7000G GPIO4 LOW ,turn off GPS power
  // CMD:AT+SGPIO=0,4,1,0
  // Only in version 20200415 is there a function to control GPS power
  modem.sendAT("+SGPIO=0,4,1,0");


  #if TINY_GSM_USE_GPRS
    // Unlock your SIM card with a PIN if needed
    if ( GSM_PIN && modem.getSimStatus() != 3 ) {
      modem.simUnlock(GSM_PIN);
    }
  #endif
  /*
    2 Automatic
    13 GSM only
    38 LTE only
    51 GSM and LTE only
  * * * */
  String res;
  do {
    res = modem.setNetworkMode(38);
    delay(500);
  } while (res != "OK");

  /*
    1 CAT-M
    2 NB-Iot
    3 CAT-M and NB-IoT
  * * */
  do {
    res = modem.setPreferredMode(1);
    delay(500);
  } while (res != "OK");

}


void apiCall(){
  #if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_XBEE
    // The XBee must run the gprsConnect function BEFORE waiting for network!
    modem.gprsConnect(apn, gprsUser, gprsPass);
  #endif

    SerialMon.print("Waiting for network...");
    if (!modem.waitForNetwork()) {
      SerialMon.println(" fail");
      delay(10000);
      return;
    }
    SerialMon.println(" success");

    if (modem.isNetworkConnected()) {
      SerialMon.println("Network connected");
    }


      SerialMon.print(F("Connecting to "));
      SerialMon.print(apn);
      if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.println(" fail");
        delay(10000);
        return;
      }
      SerialMon.println(" success");

      if (modem.isGprsConnected()) {
        SerialMon.println("GPRS connected");
      }

      modem.sendAT("+SGPIO=0,4,1,1");

      modem.enableGPS();
      float lat,  lon;
      while (1) {
        if (modem.getGPS(&lat, &lon)) {
          Serial.printf("lat:%f lon:%f\n", lat, lon);
          break;
        } else {
          Serial.print("getGPS ");
          Serial.println(millis());
        }
        delay(2000);
      }
      modem.disableGPS();

      // Set SIM7000G GPIO4 LOW ,turn off GPS power
      // CMD:AT+SGPIO=0,4,1,0
      // Only in version 20200415 is there a function to control GPS power
      modem.sendAT("+SGPIO=0,4,1,0");
      Serial.println("\n---End of GPRS TEST---\n");

    SerialMon.print(F("Performing HTTP GET request... "));
    String message = "/therestofyourdomain/api.php?key=sim7000g";
    message = message + "&lat=";
    message = message + lat;
    message = message + "&lon=";
    message = message + lon;
    int err = http.get(message);
    if (err != 0) {
      SerialMon.println(F("failed to connect"));
      delay(10000);
      return;
    }

    int status = http.responseStatusCode();
    SerialMon.print(F("Response status code: "));
    SerialMon.println(status);
    if (!status) {
      delay(10000);
      return;
    }

    SerialMon.println(F("Response Headers:"));
    while (http.headerAvailable()) {
      String headerName = http.readHeaderName();
      String headerValue = http.readHeaderValue();
      SerialMon.println("    " + headerName + " : " + headerValue);
    }

    int length = http.contentLength();
    if (length >= 0) {
      SerialMon.print(F("Content length is: "));
      SerialMon.println(length);
    }
    if (http.isResponseChunked()) {
      SerialMon.println(F("The response is chunked"));
    }

    String body = http.responseBody();
    SerialMon.println(F("Response:"));
    SerialMon.println(body);

    SerialMon.print(F("Body length is: "));
    SerialMon.println(body.length());

    // Shutdown

    http.stop();
    SerialMon.println(F("Server disconnected"));

  // #if TINY_GSM_USE_WIFI
  //     modem.networkDisconnect();
  //     SerialMon.println(F("WiFi disconnected"));
  // #endif
  // #if TINY_GSM_USE_GPRS
  //     modem.gprsDisconnect();
  //     SerialMon.println(F("GPRS disconnected"));
  // #endif
}

void loop() {
  apiCall();
  delay(30000);


}
