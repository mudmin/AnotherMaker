//youtube.com/AnotherMaker
//You may want to consider using my universal receiver to get the codes for your remote.
//https://github.com/mudmin/AnotherMaker/blob/master/IR-RF/master-receiver-ir-rf-video.c
//video - https://www.youtube.com/watch?v=McYDX7_Tqy0

//You need an IR sending LED with the long leg (anode) to pin 3 of an Uno/Nano or Pin 9 of a Mega
//Connect a 200 to 330 ohm resistor to the short leg of the LED and send the other side of the resistor to ground

  int khz = 38; // 38kHz carrier frequency for the NEC protocol
  //Volume Up 1
  unsigned int irUp[] = {4550,4400, 650,500, 600,500, 600,1600, 650,1600, 600,500, 600,1650, 600,500, 600,500, 650,500, 600,500, 600,1600, 650,1600, 600,500, 600,1650, 600,500, 600,500, 600,1650, 600,1600, 600,1600, 650,450, 650,1600, 650,450, 650,450, 650,500, 650,450, 650,450, 650,500, 600,1600, 650,450, 650,1600, 650,1550, 650,1600, 650};
  //Volume Down 1
  unsigned int irDn[] = {4550,4400, 650,500, 600,500, 600,1600, 650,1600, 600,500, 600,1650, 600,500, 600,500, 600,550, 600,500, 600,1600, 600,1650, 600,500, 600,1650, 600,500, 600,500, 600,500, 650,1600, 600,1600, 650,500, 550,1650, 600,500, 650,500, 600,500, 600,1600, 600,550, 550,550, 600,1600, 650,500, 550,1650, 600,1650, 600,1600, 600};  // SAMSUNG 34346897};


#include <IRremote.h>

IRsend irsend;

void setup()
{
  Serial.begin(9600);
  Serial.println("Booted");
}

void loop() {
  Serial.println("Send it");
  irsend.sendRaw(irUp, sizeof(irUp) / sizeof(irUp[0]), khz);
  delay(500);
  irsend.sendRaw(irDn, sizeof(irDn) / sizeof(irDn[0]), khz);
  delay(5000);
}
