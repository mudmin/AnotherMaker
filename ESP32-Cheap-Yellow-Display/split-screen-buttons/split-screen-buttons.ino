// Adapted from 
// https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/tree/main/Examples/Basics/2-TouchTest

//Splits the screen horizontally and adds an arrow to pointing to the left and right.  
//touching each half of the screen will fire off a different function. 

// Make sure to copy the UserSetup.h file into the library as
// per the Github Instructions at 
//https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md. 
//The pins are defined in there.

// ----------------------------
// Standard Libraries
// ----------------------------

#include <SPI.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <XPT2046_Touchscreen.h>
#include <TFT_eSPI.h>

// ----------------------------
// Touch Screen pins
// ----------------------------

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
int pressureThreshold = 500;
int arrowSize = 20;

// the larger the offset, the more the arrows are pushed to the edge of the screen
int arrowOffsetX = 300;
int arrowOffsetY = 60;

// ----------------------------

SPIClass mySpi = SPIClass(HSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

TFT_eSPI tft = TFT_eSPI();

void setup()
{
  Serial.begin(115200);

  // Staright the SPI for the touch screen and init the TS library
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(1);

  // Staright the tft display and set it to black
  tft.init();
  tft.setRotation(1);

  baseScreenBackground();

  int x = tft.width() / 2;
  int y = 100;
  int fontSize = 2;
  tft.drawCentreString("Touch Screen to Start", x, y, fontSize);
}

void baseScreenBackground()
{
  // Clear the screen and draw the red and green halves
  tft.fillScreen(TFT_RED);
  tft.fillRect(0, tft.height() / 2, tft.width(), tft.height() / 2, TFT_BLUE);

  // right arrow (rotated 180 degrees) on the left side
  int rightArrowX = arrowOffsetX + arrowSize / 2;
  int rightArrowY = tft.height() / 2 + arrowOffsetY;
  int rightArrowTopX = rightArrowX - arrowSize / 2;
  int rightArrowTopY = rightArrowY - arrowSize / 2;
  int rightArrowBottomY = rightArrowY + arrowSize / 2;

  tft.fillTriangle(
      rightArrowTopX, rightArrowTopY,
      rightArrowX, rightArrowY,
      rightArrowTopX, rightArrowBottomY,
      TFT_WHITE);

  // Calculate the X-coordinate of the left point of the arrow's base
  int leftArrowX = tft.width() - arrowOffsetX - arrowSize / 2;

  // Calculate the Y-coordinate of the center point of the arrow
  int leftArrowY = tft.height() / 2 - arrowOffsetY;

  // Calculate the X-coordinate of the top point of the arrow
  int leftArrowTopX = leftArrowX + arrowSize / 2;

  // Calculate the Y-coordinate of the top point of the arrow
  int leftArrowTopY = leftArrowY - arrowSize / 2;

  // Calculate the Y-coordinate of the bottom point of the arrow
  int leftArrowBottomY = leftArrowY + arrowSize / 2;

  // Draw the left arrow using the calculated coordinates
  tft.fillTriangle(
      // X-coordinate of the top point of the arrow
      leftArrowTopX,

      // Y-coordinate of the top point of the arrow
      leftArrowTopY,

      // X-coordinate of the left point of the arrow's base
      leftArrowX,

      // Y-coordinate of the center point of the arrow
      leftArrowY,

      // X-coordinate of the top point of the arrow (again, to close the triangle)
      leftArrowTopX,

      // Y-coordinate of the bottom point of the arrow
      leftArrowBottomY,

      // Color of the arrow (in this case, white)
      TFT_WHITE);
}

void printTouchLocation(TS_Point p)
{
  Serial.print("Pressure = ");
  Serial.print(p.z);
  Serial.print(", x = ");
  Serial.print(p.x);
  Serial.print(", y = ");
  Serial.print(p.y);
  Serial.println();

  // leave a dead zone in the middle of the screen.
  if (p.y < 2000)
  {
   topPressed();
  }
  else if (p.y > 2800)
  {
   bottomPressed();
  }
  delay(1000);
  baseScreenBackground();
}

void topPressed(){
  tft.fillScreen(TFT_GREEN);
  tft.setTextColor(TFT_BLACK, TFT_GREEN);
  tft.drawCentreString("Top half touched", tft.width() / 2, tft.height() / 2, 2);
  Serial.println("Top half touched");
}

void bottomPressed(){
  tft.fillScreen(TFT_GREEN);
  tft.setTextColor(TFT_BLACK, TFT_GREEN);
  tft.drawCentreString("Bottom half touched", tft.width() / 2, tft.height() / 2, 2);
  Serial.println("Bottom half touched");
}

void loop()
{
  if (ts.tirqTouched() && ts.touched())
  {
    TS_Point p = ts.getPoint();
    if (p.z >= pressureThreshold)
    {

      printTouchLocation(p);
    }

    delay(100);
  }
}
