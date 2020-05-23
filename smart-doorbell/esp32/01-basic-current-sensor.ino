// https://youtube.com/AnotherMaker
// https://github.com/mudmin/AnotherMaker
const int doorbell = 34;            //current sensor connected to pin 34 and ground
int senseDoorbell = 0;              //variable to hold doorbell sensor reading
int debounce = 1000;                //only allow one DingDong per second
unsigned long currentMillis = 0;    //how many milliseconds since the Arduino booted
unsigned long prevRing = 0;         //The last time the doorbell rang

void setup() {
  Serial.begin(9600);
  pinMode(doorbell, INPUT);
  Serial.println("DoorBellDuino Booted");
}

void loop() {
  //get the time since the arduino booted
  currentMillis = millis();
  //only check the doorbell if it hasn't been hit in the last second
  if(currentMillis - prevRing >= debounce){
    senseDoorbell = analogRead(doorbell);  //read the doorbell sensor
    if(senseDoorbell > 50){                //mine read between 0 and 7 with no current and 200 with it.  50 seemed to be safe.
      Serial.println("DingDong");
      prevRing = currentMillis;            //engage debounce mode!
    }
  }
}
