//Make sure you install the Adafruit Neopixe library, probably from the library manager,
// but it's also available in this repo

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN1 6
#define PIN2 12
 int LED1=8;
 int LED2=9;
 int LED3=10;
 int LED4=11;
 int LED7=2;
 int LED8=3;
 int LED9=4;
 int LED10=5;
 int LED11=6;
 int LED12=7;
 int n;
#define NTD0 -1
#define NTD1 294
#define NTD2 330
#define NTD3 350
#define NTD4 393
#define NTD5 441
#define NTD6 495
#define NTD7 556

#define NTDL1 147
#define NTDL2 165
#define NTDL3 175
#define NTDL4 196
#define NTDL5 221
#define NTDL6 248
#define NTDL7 278

#define NTDH1 589
#define NTDH2 661
#define NTDH3 700
#define NTDH4 786
#define NTDH5 882
#define NTDH6 990
#define NTDH7 112
//列出全部D调的频率
#define WHOLE 1
#define HALF 0.5
#define QUARTER 0.25
#define EIGHTH 0.25
#define SIXTEENTH 0.625
//列出所有节拍
int tune[]=                 //根据简谱列出各频率
{
  NTD3,NTD3,NTD4,NTD5,
  NTD5,NTD4,NTD3,NTD2,
  NTD1,NTD1,NTD2,NTD3,
  NTD3,NTD2,NTD2,
  NTD3,NTD3,NTD4,NTD5,
  NTD5,NTD4,NTD3,NTD2,
  NTD1,NTD1,NTD2,NTD3,
  NTD2,NTD1,NTD1,
  NTD2,NTD2,NTD3,NTD1,
  NTD2,NTD3,NTD4,NTD3,NTD1,
  NTD2,NTD3,NTD4,NTD3,NTD2,
  NTD1,NTD2,NTDL5,NTD0,
  NTD3,NTD3,NTD4,NTD5,
  NTD5,NTD4,NTD3,NTD4,NTD2,
  NTD1,NTD1,NTD2,NTD3,
  NTD2,NTD1,NTD1
};
float durt[]=                   //根据简谱列出各节拍
{
  1,1,1,1,
  1,1,1,1,
  1,1,1,1,
  1+0.5,0.5,1+1,
  1,1,1,1,
  1,1,1,1,
  1,1,1,1,
  1+0.5,0.5,1+1,
  1,1,1,1,
  1,0.5,0.5,1,1,
  1,0.5,0.5,1,1,
  1,1,1,1,
  1,1,1,1,
  1,1,1,0.5,0.5,
  1,1,1,1,
  1+0.5,0.5,1+1,
};
int length;
int tonepin=6;   //得用6号接口
// Parameter 1 = number of pixels in strip1
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(1, PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(1, PIN2, NEO_GRB + NEO_KHZ800);
// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code
for(n=2;n<=13;n++)
{
  pinMode(n, OUTPUT);
}

  strip1.begin();
  strip2.begin();
  strip1.show(); // Initialize all pixels to 'off'
  strip2.show();

   pinMode(tonepin,OUTPUT);
  length=sizeof(tune)/sizeof(tune[0]);   //计算长度
}

void loop() {
  // Some example procedures showing how to display to the pixels:
  colorWipe(strip1.Color(50, 0, 0), 100); // Red
   colorWipe(strip2.Color(255, 0, 0), 100); // Red
  colorWipe(strip1.Color(0, 50, 0), 100); // Green
    colorWipe(strip2.Color(0, 255, 0), 100); // Green
  colorWipe(strip1.Color(0, 0, 50), 100); // Blue
    colorWipe(strip2.Color(0, 0, 255), 100); // Blue
  turn1();//顺序点亮，顺序熄灭
  clean();  //灭掉所有灯
 // turn2();//6灯齐闪
 // clean();//灭掉所有灯
 // turn3();
 // clean();//灭掉所有灯
for(int x=0;x<length;x++)
  {
    tone(tonepin,tune[x]);
    delay(200*durt[x]);   //这里用来根据节拍调节延时，500这个指数可以自己调整，在该音乐中，我发现用500比较合适。
    noTone(tonepin);
  }
  delay(2000);

}
// Fill the dots one after the other with a color


void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip1.numPixels(); i++) {
    strip1.setPixelColor(i, c);
    strip1.show();
    delay(wait);
  }
    for(uint16_t i=0; i<strip2.numPixels(); i++) {
    strip2.setPixelColor(i, c);
    strip2.show();
    delay(wait);
  }
}
void turn1()  //顺序点亮，顺序熄灭
{
  for(n=2;n<=13;n++)
{
    digitalWrite(n,HIGH);
    delay(300);
}
  for(n=2;n<=13;n++)
{
    digitalWrite(n,LOW);
    delay(300);
}
}
void turn2()//6灯齐闪3次
{ for(int x=0;x<=2;x++)
{
  for(n=2;n<=13;n++)
{
    digitalWrite(n,HIGH);
}
delay(300);
  for(n=2;n<=13;n++)
{
    digitalWrite(n,LOW);
}
delay(300);
}
}
void turn3()//两个两个一起闪3次
{
  for(int x=0;x<=2;x++)
  {
  digitalWrite(8,HIGH);
  digitalWrite(9,HIGH);
  for(n=10;n<=11;n++){
    digitalWrite(n,LOW);
  }
  delay(300);
  digitalWrite(10,HIGH);
  digitalWrite(11,HIGH);
  digitalWrite(8,LOW);
  digitalWrite(9,LOW);
  delay(300);
  for(n=2;n<=11;n++){
    digitalWrite(n,LOW);
  }
  delay(300);
}
}
void clean()
{  for(n=2;n<=11;n++)
{
    digitalWrite(n,LOW);
}
delay(300);
}
