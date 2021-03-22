#define TINY_GSM_DEBUG Serial
#define CAYENNE_PRINT Serial
#define TINY_GSM_MODEM_SIM7000


#define USE_GSM  //! Uncomment will use SIM7000 for GSM communication

#ifdef USE_GSM
#include <CayenneMQTTGSM.h>
#else
#include <CayenneMQTTESP32.h>
#endif
#include <Arduino.h>
#include <Wire.h>

#define BATTERY_VIRTUAL_CHANNEL             1
#define SOLAR_VIRTUAL_CHANNEL               2


#define PIN_TX      27
#define PIN_RX      26
#define UART_BAUD   9600
#define PWR_PIN     4
#define BAT_ADC     35
#define SOLAR_ADC   36


HardwareSerial  gsmSerial(1);

#ifdef USE_GSM
// GSM connection info.
char apn[] = "h2g2"; // Access point name. Leave empty if it is not needed.
char gprsLogin[] = ""; // GPRS username. Leave empty if it is not needed.
char gprsPassword[] = ""; // GPRS password. Leave empty if it is not needed.
char pin[] = ""; // SIM pin number. Leave empty if it is not needed.
#else
// WiFi network info.
char ssid[] = "your wifi ssid";
char wifiPassword[] = "your wifi password";
#endif

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "Cayenne username";
char password[] = "Cayenne password";
char clientID[] = "Cayenne clientID";

bool bmpSensorDetected = true;



void setup()
{
    Serial.begin(UART_BAUD);
    gsmSerial.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    pinMode(PWR_PIN, OUTPUT);

    //Launch SIM7000
    digitalWrite(PWR_PIN, HIGH);
    delay(300);
    digitalWrite(PWR_PIN, LOW);




    //Wait for the SIM7000 communication to be normal, and will quit when receiving any byte
    int i = 6;
    delay(200);
    while (i) {
        Serial.println("Send AT");
        gsmSerial.println("AT");
        if (gsmSerial.available()) {
            String r = gsmSerial.readString();
            Serial.println(r);
            break;
        }
        delay(1000);
        i--;
    }

#ifdef USE_GSM
    Cayenne.begin(username, password, clientID, gsmSerial, apn, gprsLogin, gprsPassword, pin);
#else
    Cayenne.begin(username, password, clientID, ssid, wifiPassword);
#endif
}


void loop()
{
    Cayenne.loop();
}




// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
    CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
    //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}

CAYENNE_IN(1)
{
    CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
}


float readBattery(uint8_t pin)
{
    int vref = 1100;
    uint16_t volt = analogRead(pin);
    float battery_voltage = ((float)volt / 4095.0) * 2.0 * 3.3 * (vref);
    return battery_voltage;
}


CAYENNE_OUT(BATTERY_VIRTUAL_CHANNEL)
{
    float mv = readBattery(BAT_ADC);
    Serial.printf("batter : %f\n", mv);
    Cayenne.virtualWrite(BATTERY_VIRTUAL_CHANNEL, mv, TYPE_VOLTAGE, UNIT_MILLIVOLTS);

}


CAYENNE_OUT(SOLAR_VIRTUAL_CHANNEL)
{
    float mv = readBattery(SOLAR_ADC);
    Serial.printf("solar : %f\n", mv);
    Cayenne.virtualWrite(SOLAR_VIRTUAL_CHANNEL, mv, TYPE_VOLTAGE, UNIT_MILLIVOLTS);

}
