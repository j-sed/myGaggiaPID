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
bool SSRStat = true;
int xPosOld = 0;
int graphHeightOld;
int DelayVal = 500;
int timeNow;
int teplotaCAvg;
float tempCorrection = 1.5;
int tMax = 35;

//Define Variables we'll be connecting to
float Setpoint, Input, Output;

//Specify the links and initial tuning parameters
float Kp = 10, Ki = 50, Kd = 30;
float POn = 1.0;   // proportional on Error to Measurement ratio (0.0-1.0), default = 1.0
float DOn = 0.0;   // derivative on Error to Measurement ratio (0.0-1.0), default = 0.0

QuickPID myQuickPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, POn, DOn, QuickPID::DIRECT);
//QuickPID _myPID = QuickPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, POn, DOn, QuickPID::DIRECT);

unsigned int WindowSize = 1000;
unsigned int minWindow = 250;
unsigned long windowStartTime;


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


// PID
  // Select one, reference: https://github.com/Dlloydev/QuickPID/wiki
  //_myPID.AutoTune(tuningMethod::ZIEGLER_NICHOLS_PI);
//  _myPID.AutoTune(tuningMethod::ZIEGLER_NICHOLS_PID);
  //_myPID.AutoTune(tuningMethod::TYREUS_LUYBEN_PI);
  //_myPID.AutoTune(tuningMethod::TYREUS_LUYBEN_PID);
  //_myPID.AutoTune(tuningMethod::CIANCONE_MARLIN_PI);
  //_myPID.AutoTune(tuningMethod::CIANCONE_MARLIN_PID);
  //_myPID.AutoTune(tuningMethod::AMIGOF_PID);
  //_myPID.AutoTune(tuningMethod::PESSEN_INTEGRAL_PID);
  //_myPID.AutoTune(tuningMethod::SOME_OVERSHOOT_PID);
  //_myPID.AutoTune(tuningMethod::NO_OVERSHOOT_PID);
//initialize the variables we're linked to
  windowStartTime = millis();
  Setpoint = 30;
  //tell the PID to range between 0 and the full window size
  myQuickPID.SetOutputLimits(0, WindowSize);

  //turn the PID on
  myQuickPID.SetMode(QuickPID::AUTOMATIC);
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


  
////// PID/////

  
  /************************************************
     turn the output pin on/off based on pid output
   ************************************************/
  Input = teplotaC;
  if (millis() - windowStartTime >= WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
    myQuickPID.Compute();
  }
  if (((unsigned int)Output > minWindow) && ((unsigned int)Output < (millis() - windowStartTime))) digitalWrite(SSR_PIN, HIGH);
  else digitalWrite(SSR_PIN, LOW);
  
//  Serial.print("Input:");    Serial.print(Input);    Serial.println(",");
//  Serial.print("Output:");    Serial.print(Output);    Serial.println(",");
  Serial.print(Input);
  Serial.print("\t"); // a space ' ' or  tab '\t' character is printed between the two values.
  Serial.println(Output);
  
  delay(DelayVal);
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
