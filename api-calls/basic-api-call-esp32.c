#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h> // https://github.com/espressif/arduino-esp32/tree/master/libraries/HTTPClient
#include <ArduinoJson.h> //v6.11.5 from Library Manager

const char* ssid = "yourSSID";
const char* password = "YOURpassword";

// Use arduinojson.org/v6/assistant to compute the capacity.
DynamicJsonDocument doc(1024);

const char* api = "http://192.168.95.9:80/api/key/?key=12345-67890-12345-67890-12345"; //Node-Red
// const char* api = "http://192.168.95.148:80/4414/apirelay.php"; //PHP

void setup()
{
Serial.begin(9600);
Serial.println("Booted");
delay(10);

// We start by connecting to a WiFi network

Serial.println();
Serial.println();
Serial.print("Connecting to ");
Serial.println(ssid);

WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
}

Serial.println("");
Serial.println("WiFi connected");
Serial.println("IP address: ");
Serial.println(WiFi.localIP());
}

void loop()
{
  apiCall();
}
void apiCall(){
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
    Serial.println("Trying API");
    HTTPClient http;    //Declare object of class HTTPClient

    http.begin(api);      //Specify request destination
    http.addHeader("Content-Type", "text/plain");  //Specify content-type header
    int httpCode = http.GET(); //if you want GET
    // int httpCode = http.POST("{\"key\":\"O7KWV-VXKF4-NU5D5-2C2D9-DFEF8\"}");   //If you want POST
    http.end();  //Close connection

  }else{

    Serial.println("Error in WiFi connection");

  }
}
