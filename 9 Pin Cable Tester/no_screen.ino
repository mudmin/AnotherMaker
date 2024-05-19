//if you are not familiar with programming the arduino, 
//you can find the software and instructions here:
// https://www.arduino.cc/en/Guide/ArduinoNano

void setup()
{
    // Initialize the serial port
    Serial.begin(9600);

    // Set pins 12 through 4 as output
    for (int pin = 12; pin >= 4; pin--)
    {
        pinMode(pin, OUTPUT);
    }

    // Print a welcome message
    Serial.println("9 Pin Cable Tester by AnotherMaker");
}

void loop()
{

    for (int pin = 12; pin >= 4; pin--)
    {
        // Turn on the current LED
        digitalWrite(pin, HIGH);
        // Print which LED is lit up

        Serial.print("LED ");
        Serial.print(13 - pin);
        Serial.println(" is lit up");

        // Wait for 0.5 seconds
        delay(500);

        // Turn off the current LED
        digitalWrite(pin, LOW);
    }
}
