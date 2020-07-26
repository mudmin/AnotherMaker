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
