// LIBS
#include <Ucglib.h>
#include <SPI.h>
#include <max6675.h>
#include "Ucglib.h"
#include <TFT.h>
#include "QuickPID.h"

// PIN DEF
// display
#define RESET 8
#define DC 9
#define CSLCD 10
// ssr
#define SSR_PIN 7
// max6675
#define pinSO 3
#define pinCS 4
#define pinSCK 5
// Notes::
// possible to use: int pinCS = 4;
// maybe will be forced to, as it uses less memory

// INITIALIZATION
TFT TFTscreen = TFT(CSLCD, DC, RESET);
MAX6675 termoclanek(pinSCK, pinCS, pinSO);

// DECLARATION
int xPos = 0;
char sensorPrintout[6];
bool SSRStat = false;
int xPosOld = 0;
int graphHeightOld;
int DelayVal = 500;


void setup(void) {
  // komunikace přes sériovou linku rychlostí 9600 baud
  Serial.begin(9600);  

// Switch to turn on the OUTPUT pin for SSR 
  switch(SSRStat) {
  case true:
    pinMode(SSR_PIN, OUTPUT);
    break;
}
  TFTscreen.begin();
// fillRect(xStart,yStart,width,height,color)
    // Screen setting
  TFTscreen.background(0, 0, 0);
  TFTscreen.setTextColor(0xFFFF);
}

void loop(void) {
  // načtení aktuální teploty termočlánku
  // do proměnné teplotaC
  float teplotaC = termoclanek.readCelsius()-tempCorrection;
  //  TFTscreen.drawRect(0, 60, 160, 50,0xFFFF);
  String sensorVal = String(teplotaC);
  // convert the reading to a char array
  sensorVal.toCharArray(sensorPrintout, 6);
  if ((teplotaC < tMax) && (teplotaC > -1))
  {
  TFTscreen.fillRect(60, 35, 35, 8, 0x0000);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text(sensorPrintout, 60, 35);
  // nastavení kurzoru na pozici [x, y] = [40, 25]
  TFTscreen.setCursor(40, 25);
  TFTscreen.setTextSize(1);
  // tištění textu
  TFTscreen.print("Mereni teploty");

 
  // map(val, val_min,val_max,screen_min,screen_max)
  int graphHeight = map(teplotaC, 0, 100, TFTscreen.height(), 0);
//  Serial.print("graphHeight:");    Serial.print(graphHeight);    Serial.println(",");
  TFTscreen.stroke(255, 255, 255);
  
  TFTscreen.line(xPosOld, graphHeightOld, xPos,graphHeight);
  xPosOld = xPos;
  graphHeightOld = graphHeight;
  if (xPos >= 128) {

    xPos = 0;
    xPosOld =0;

    TFTscreen.background(0, 0, 0);
    TFTscreen.setTextColor(0xFFFF);
    // nastavení kurzoru na pozici [x, y] = [40, 25]
    TFTscreen.setCursor(40, 25);
    TFTscreen.setTextSize(1);
    // tištění textu
    TFTscreen.print("Mereni teploty");
  }

  else {

    xPos++;
    
  }

  // SECURITY LOOP - If temp is too high or error reading, then 
  else if (teplotaC >= tMax){
  TFTscreen.begin();
  TFTscreen.background(0, 0, 0);
  TFTscreen.fillRect(60, 35, 35, 8, 0x0000);
  TFTscreen.setTextColor(0xFFFF);
  // nastavení kurzoru na pozici [x, y] = [40, 25]
  TFTscreen.setCursor(60, 25);
  TFTscreen.setTextSize(2);
  // tištění textu
  TFTscreen.print("VYSOKA\nTEPLOTA");
  digitalWrite(SSR_PIN, LOW);
  delay(DelayVal);
  TFTscreen.fillRect(0, 0, 160, 128, 0x0000);
  
  }else
  {
  TFTscreen.begin();
  TFTscreen.background(0, 0, 0);
  TFTscreen.setTextColor(0xFFFF);
  // nastavení kurzoru na pozici [x, y] = [40, 25]
  TFTscreen.setCursor(60, 25);
  TFTscreen.setTextSize(2);
  // tištění textu
  TFTscreen.print("CHYBA\nCIDLA");
  digitalWrite(SSR_PIN, LOW);
  delay(DelayVal);
  TFTscreen.fillRect(0, 0, 160, 128, 0x0000);
  }
  } // END of LOOP
