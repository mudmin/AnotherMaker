/*********
  TTGO SmartWatch-2020 Morse Code Vibrator

  Even if they don't want it, much credit goes to
  https://create.arduino.cc/projecthub/electropeak/how-to-make-a-morse-code-translator-with-arduino-d6ecc8

  and

  the great Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-input-data-html-form/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#define LILYGO_WATCH_2020_V1
#include <LilyGoWatch.h>
TTGOClass *ttgo;
AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "xxxxxxx";
const char* password = "xxxxxxxxxxx";

const char* PARAM_INPUT_1 = "input1";

const int buz = 4;
String code = "";
int len = 0;
char ch;
char new_char;
int unit_delay = 150;
// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Send me a message</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Message (20 chars max): <input type="text" name="input1" maxlength="20">
    <input type="submit" value="Submit">
  </form><br>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void dot()
{
Serial.print(".");

digitalWrite(buz, HIGH);
delay(unit_delay);

digitalWrite(buz, LOW);
delay(unit_delay);
}
void dash()
{
Serial.print("-");

digitalWrite(buz, HIGH);
delay(unit_delay * 3);

digitalWrite(buz, LOW);
delay(unit_delay);
}
void A()
{
dot();
delay(unit_delay);
dash();
delay(unit_delay);
}
void B()
{
dash();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
}
void C()
{
dash();
delay(unit_delay);
dot();
delay(unit_delay);
dash();
delay(unit_delay);
dot();
delay(unit_delay);
}
void D()
{
dash();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
}
void E()
{
dot();
delay(unit_delay);
}
void f()
{
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dash();
delay(unit_delay);
dot();
delay(unit_delay);
}
void G()
{
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dot();
delay(unit_delay);
}
void H()
{
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
}
void I()
{
dot();
delay(unit_delay);
dot();
delay(unit_delay);
}
void J()
{
dot();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
}
void K()
{
dash();
delay(unit_delay);
dot();
delay(unit_delay);
dash();
delay(unit_delay);
}
void L()
{
dot();
delay(unit_delay);
dash();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
}
void M()
{
dash();
delay(unit_delay);
dash();
delay(unit_delay);
}
void N()
{
dash();
delay(unit_delay);
dot();
delay(unit_delay);
}
void O()
{
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
}
void P()
{
dot();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dot();
}
void Q()
{
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dot();
delay(unit_delay);
dash();
delay(unit_delay);
}
void R()
{
dot();
delay(unit_delay);
dash();
delay(unit_delay);
dot();
delay(unit_delay);
}
void S()
{
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
}
void T()
{
dash();
delay(unit_delay);
}
void U()
{
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dash();
delay(unit_delay);
}
void V()
{
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dash();
delay(unit_delay);
}
void W()
{
dot();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
}
void X()
{
dash();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dash();
delay(unit_delay);
}
void Y()
{
dash();
delay(unit_delay);
dot();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
}
void Z()
{
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
}
void one()
{
dot();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
}
void two()
{
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
}
void three()
{
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
}
void four()
{
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dash();
delay(unit_delay);
}
void five()
{
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
}
void six()
{
dash();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
}
void seven()
{
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
}
void eight()
{
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dot();
delay(unit_delay);
dot();
delay(unit_delay);
}
void nine()
{
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dot();
delay(unit_delay);
}
void zero()
{
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
dash();
delay(unit_delay);
}
void morse()
{
if (ch == 'A' || ch == 'a')
{
A();
Serial.print(" ");
}
else if (ch == 'B' || ch == 'b')
{
B();
Serial.print(" ");
}
else if (ch == 'C' || ch == 'c')
{
C();
Serial.print(" ");
}
else if (ch == 'D' || ch == 'd')
{
D();
Serial.print(" ");
}
else if (ch == 'E' || ch == 'e')
{
E();
Serial.print(" ");
}
else if (ch == 'f' || ch == 'f')
{
f();
Serial.print(" ");
}
else if (ch == 'G' || ch == 'g')
{
G();
Serial.print(" ");
}
else if (ch == 'H' || ch == 'h')
{
H();
Serial.print(" ");
}
else if (ch == 'I' || ch == 'i')
{
I();
Serial.print(" ");
}
else if (ch == 'J' || ch == 'j')
{
J();
Serial.print(" ");
}
else if (ch == 'K' || ch == 'k')
{
K();
Serial.print(" ");
}
else if (ch == 'L' || ch == 'l')
{
L();
Serial.print(" ");
}
else if (ch == 'M' || ch == 'm')
{
M();
Serial.print(" ");
}
else if (ch == 'N' || ch == 'n')
{
N();
Serial.print(" ");
}
else if (ch == 'O' || ch == 'o')
{
O();
Serial.print(" ");
}
else if (ch == 'P' || ch == 'p')
{
P();
Serial.print(" ");
}
else if (ch == 'Q' || ch == 'q')
{
Q();
Serial.print(" ");
}
else if (ch == 'R' || ch == 'r')
{
R();
Serial.print(" ");
}
else if (ch == 'S' || ch == 's')
{
S();
Serial.print(" ");
}
else if (ch == 'T' || ch == 't')
{
T();
Serial.print(" ");
}
else if (ch == 'U' || ch == 'u')
{
U();
Serial.print(" ");
}
else if (ch == 'V' || ch == 'v')
{
V();
Serial.print(" ");
}
else if (ch == 'W' || ch == 'w')
{
W();
Serial.print(" ");
}
else if (ch == 'X' || ch == 'x')
{
X();
Serial.print(" ");
}
else if (ch == 'Y' || ch == 'y')
{
Y();
Serial.print(" ");
}
else if (ch == 'Z' || ch == 'z')
{
Z();
Serial.print(" ");
}
else if (ch == '0')
{
zero();
Serial.print(" ");
}
else if (ch == '1')
{
one();
Serial.print(" ");
}
else if (ch == '2')
{
two();
Serial.print(" ");
}
else if (ch == '3')
{
three();
Serial.print(" ");
}
else if (ch == '4')
{
four();
Serial.print(" ");
}
else if (ch == '5')
{
five();
Serial.print(" ");
}
else if (ch == '6')
{
six();
Serial.print(" ");
}
else if (ch == '7')
{
seven();
Serial.print(" ");
}
else if (ch == '8')
{
eight();
Serial.print(" ");
}
else if (ch == '9')
{
nine();
Serial.print(" ");
}
else if(ch == ' ')
{
delay(unit_delay*7);
Serial.print("/ ");
}
else
Serial.println("Unknown symbol!");
}
void String2Morse()
{
len = code.length();
for (int i = 0; i < len; i++)
{
ch = code.charAt(i);
delay(10);
morse();
}
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  pinMode(buz, OUTPUT);
  digitalWrite(buz,LOW);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->tft->fillScreen(TFT_BLACK);
  ttgo->tft->setTextSize(5);
  ttgo->tft->setTextColor(TFT_RED);
  String ip = WiFi.localIP().toString();
  ttgo->tft->drawString(ip, 62, 90);
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;

    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    code = inputMessage;
    request->send(200, "text/html", "Sent: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");

     inputMessage = "";

  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
delay(30);
if(code != ""){
  String2Morse();
  code = "";
  digitalWrite(buz,LOW);
}
}
