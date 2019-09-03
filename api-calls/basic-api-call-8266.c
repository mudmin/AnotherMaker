//https://electrosome.com/calling-api-esp8266/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> //v6.11.5 from Library Manager

const char* ssid = "YOURssid";
const char* password = "yourPASSWORD";

// Use arduinojson.org/v6/assistant to compute the capacity.
DynamicJsonDocument doc(1024);

const char* api = "http://192.168.95.8:1880/apiCalls"; //Node-Red
// const char* api = "http://192.168.95.148:80/4414/apirelay.php"; //PHP

void setup()
{
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting...");
  }
}

void loop()
{
  apiCall();
  delay(10000);
}
void apiCall(){
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
    Serial.println("Trying API");
    HTTPClient http;    //Declare object of class HTTPClient

    http.begin(api);      //Specify request destination
    http.addHeader("Content-Type", "text/plain");  //Specify content-type header
    int httpCode = http.GET(); //if you want GET
    // int httpCode = http.POST("{\"key\":\"O7KWV-VXKF4-NU5D5-2C2D9-DFEF8\"}");   //If you want POST
    String payload = http.getString();
    Serial.println(payload);
    deserializeJson(doc, payload);              //Get the response payload
    // Extract values
    Serial.println(F("Response:"));
    Serial.print("The weather outside is currently: ");
    Serial.println(doc["weather"].as<char*>());
    Serial.print("The temperature is: ");
    Serial.print(doc["temp"].as<float>(),2);
    Serial.println(" F");
    Serial.print("The humidity is ");
    Serial.print(doc["humidity"].as<int>());
    Serial.println("%");

    http.end();  //Close connection

  }else{

    Serial.println("Error in WiFi connection");

  }
}
