#include <WiFiClient.h>
#include <PubSubClient.h>

#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12

int request = 0;
char *cstring;

int relay1 = 32;
int relay2 = 33;

#include <ETH.h>

// CHANGE THESE SETTINGS FOR YOUR APPLICATION
const char* mqttServerIp = "192.168.1.123"; // IP address of the MQTT broker
const short mqttServerPort = 1883; // IP port of the MQTT broker
const char* mqttClientName = "ESP32-POE";
const char* mqttUsername = NULL;
const char* mqttPassword = NULL;

// Initializations of network clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);
static bool eth_connected = false;
uint64_t chipid;

//ESP32 Specific Ethernet
//Yes, I know it says wifi.  Trust me.
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

void callback(char* topic, byte* payload, unsigned int length) {
  for (int i=0;i<length;i++) {
    payload[length] = '\0';
    cstring = (char *) payload;
  }

  request = atof(cstring);
  Serial.println(request);
  Serial.println(topic);


    // This snippet sets all the setpoints at once for the whole arduino
    if (strstr(topic, "setall") != NULL) {

    }

  if (strstr(topic, "request") != NULL) {
    request = atof(cstring);
    Serial.println("The request is...");
    Serial.println(request);
    if (strstr(topic, "1on") != NULL) {
    Serial.println("Turning relay 1 on");
    digitalWrite(relay1, LOW);
    }
    if (strstr(topic, "1off") != NULL) {
    Serial.println("Turning relay 1 off");
    digitalWrite(relay1, HIGH);
    }
    if (strstr(topic, "2on") != NULL) {
    Serial.println("Turning relay 2 on");
    digitalWrite(relay2, LOW);
    }
    if (strstr(topic, "2off") != NULL) {
    Serial.println("Turning relay 2 off");
    digitalWrite(relay2, HIGH);
    }

}
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(mqttClientName)) {
    //if (mqttClient.connect(mqttClientName, mqttUsername, mqttPassword) { // if credentials is nedded
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("random/test","hello world");
      // ... and resubscribe
      mqttClient.subscribe("relays/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,HIGH);

  #define BUTTON_PRESSED()  (!digitalRead (34))

  mqttClient.setServer(mqttServerIp, mqttServerPort);
  mqttClient.setCallback(callback);

  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.

  WiFi.onEvent(WiFiEvent);
  ETH.begin();
}

void loop()
{
 //  if (BUTTON_PRESSED())
 // {
 //   digitalWrite(relay1,LOW);
 //   delay(250);
 //   digitalWrite(relay1,HIGH);
 // }
  // check if ethernet is connected
  if (eth_connected) {
    // now take care of MQTT client...
    if (!mqttClient.connected()) {
      reconnect();
    } else {
      mqttClient.loop();

    }
  }
}
