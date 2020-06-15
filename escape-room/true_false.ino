int switch1 = 22;
int switch2 = 23;
int switch3 = 24;
int switch4 = 25;
int switch5 = 26;
int switch6 = 27;
// int switch7 = 28;
int submit = 29;


void setup() {
  Serial.begin(9600);
  pinMode(switch1,INPUT_PULLUP);
  pinMode(switch2,INPUT_PULLUP);
  pinMode(switch3,INPUT_PULLUP);
  pinMode(switch4,INPUT_PULLUP);
  pinMode(switch5,INPUT_PULLUP);
  pinMode(switch6,INPUT_PULLUP);
  // pinMode(switch7,INPUT_PULLUP);
  pinMode(submit ,INPUT_PULLUP);
}

void loop() {
  int pushed = digitalRead(submit);
  if(pushed == LOW){
    Serial.println("Submit Button Pushed!");
    int s1 = digitalRead(switch1);
      Serial.print("S1=");
      Serial.println(s1);
    int s2 = digitalRead(switch2);
    Serial.print("S2=");
    Serial.println(s2);
    int s3 = digitalRead(switch3);
    Serial.print("S3=");
    Serial.println(s3);
    int s4 = digitalRead(switch4);
    Serial.print("S4=");
    Serial.println(s4);
    int s5 = digitalRead(switch5);
    Serial.print("S5=");
    Serial.println(s5);
    int s6 = digitalRead(switch6);
    Serial.print("S6=");
    Serial.println(s6);
    // int s7 = digitalRead(switch7);
    // Serial.print("S7=");
    // Serial.println(s7);
    delay(5000);
  }
}
