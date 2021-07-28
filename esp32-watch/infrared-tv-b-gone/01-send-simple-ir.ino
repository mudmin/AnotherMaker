#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <PubSubClient.h>

#define DEVICE "0x3E8"
unsigned long phaser = 0x3E8;
const uint16_t kIrLed = 13;
IRsend irsend(kIrLed);  

void setup() {
  irsend.begin();
#if ESP8266
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
#else  // ESP8266
  Serial.begin(115200, SERIAL_8N1);
#endif  // ESP8266
}

void loop() {
  Serial.println("NEC");
  irsend.sendNEC(phaser); //1000
  delay(1000);

  Serial.println("NEC");
  irsend.sendNEC(0x270F); //9999
  delay(5000);

}
