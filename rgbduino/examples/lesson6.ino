//This example sketch will go through and dim/blink the little LED to the right of pin 6
//on the arduino.

void setup()
{
pinMode(6,OUTPUT);
}
void loop()
{
analogWrite(6,255); //same with HIGH
delay(1000);
analogWrite(6,123);
delay(1000);
analogWrite(6,50);
delay(1000);
analogWrite(6, LOW);//same with 0
delay(1000);
}
