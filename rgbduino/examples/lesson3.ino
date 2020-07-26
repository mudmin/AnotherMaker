
int LED = 3;
int brightness = 0;
int fadeAmount = 5;
void setup()
{
pinMode(3, OUTPUT);
}
void loop()
{
analogWrite(LED, brightness);
brightness = brightness + fadeAmount;
if (brightness <= 0 || brightness >= 255)
{
fadeAmount = -fadeAmount;
}
delay(30);
}
