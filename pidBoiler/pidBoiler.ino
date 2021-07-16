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
float tempCorrection = 0.0;
int tMax = 112;
String sensorVal;


//Define Variables we'll be connecting to
float Setpoint, Input, Output;

//Specify the links and initial tuning parameters
float Kp = 53, Ki = 0.5, Kd = 56;
float POn = 0.5;   // proportional on Error to Measurement ratio (0.0-1.0), default = 1.0
float DOn = 0.0;   // derivative on Error to Measurement ratio (0.0-1.0), default = 0.0

QuickPID myQuickPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, POn, DOn, QuickPID::DIRECT);
//QuickPID _myPID = QuickPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, POn, DOn, QuickPID::DIRECT);




unsigned int WindowSize = 1500;
unsigned int minWindow = 250;
unsigned long windowStartTime;
int timer = 0;
int timeOld = 0;
int timeNew = 0;
float teplotaC = 0;



void updateDisplay(){
  int graphHeight = map(teplotaC, 85, tMax, TFTscreen.height(), 20);
  int setPointHeight = map(Setpoint, 85, tMax, TFTscreen.height(), 20);
//  Serial.print("graphHeight:");    Serial.print(graphHeight);    Serial.println(",");
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.fillRect(1, setPointHeight, 160, 1, 0xFF00);
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
}

float TCouple(){
  timer += abs(timeOld - timeNew);
  // Run low-frequency updates
  // delay(200);
  if (timer > 200) {
    teplotaC = termoclanek.readCelsius() - tempCorrection;
    updateDisplay();
    timer = 0;
  }
  return teplotaC;
}
void readCLI(){
  if(Serial.available()) // if there is data comming
  {
    String command = Serial.readStringUntil('\n'); // read string until newline character meet 
    if(command == "TEMP")
    {
      Serial.println(teplotaC); // send action to Serial Monitor
    }
  }
}

void PWMWrite(byte pin, int Output){
  if (millis() - windowStartTime >= WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
    myQuickPID.Compute();
    Output = WindowSize-Output;
  }
  if (((unsigned int)Output > minWindow) && ((unsigned int)Output < (millis() - windowStartTime)) && (Input < 101.0)) digitalWrite(pin, HIGH);
  else digitalWrite(pin, LOW); 
}
void PWM(int Output){
  if (Output <= 0.0) {
    Output = 0.0;
  }  
  if (Output >= 1000.0) {
    Output = 1000.0;
  }
}

void setup(void) {
  // komunikace přes sériovou linku rychlostí 9600 baud
  Serial.begin(115200);  
  Setpoint = 100;
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

//initialize the variables we're linked to
  windowStartTime = millis();
  //tell the PID to range between 0 and the full window size
  myQuickPID.SetOutputLimits(0, WindowSize);

  //turn the PID on
  myQuickPID.SetMode(QuickPID::AUTOMATIC);
}

void loop(void) {
  // načtení aktuální teploty termočlánku
  // do proměnné teplotaC
  //  TFTscreen.drawRect(0, 60, 160, 50,0xFFFF);
  readCLI();
  timeOld = timeNew;
  timeNew = millis(); // Don't poll the temperature sensor too quickly
  // Run low-frequency updates
  Input = TCouple();

  sensorVal = String(teplotaC);
  sensorVal.toCharArray(sensorPrintout, 6);
  // convert the reading to a char array
  
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
  


  
////// PID/////

  
  /************************************************
     turn the output pin on/off based on pid output
   ************************************************/
  PWMWrite(SSR_PIN,Output);
  Serial.print("Setpoint:");  Serial.print(Setpoint);  Serial.print(",");
  Serial.print("MaxTemp:");  Serial.print((int) 105);  Serial.print(",");
  Serial.print("Input:");     Serial.print(Input);     Serial.print(",");
  Serial.print("Output:");    Serial.print(Output/10);    Serial.println(",");
  // Serial.print(Input);
  // Serial.print("\t"); // a space ' ' or  tab '\t' character is printed between the two values.
  // Serial.println(Output);


  delay(200);

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
  TFTscreen.print("VYSOKA");
  TFTscreen.setCursor(60, 40);
  TFTscreen.setTextSize(2);
  TFTscreen.print("TEPLOTA");
  digitalWrite(SSR_PIN, LOW);
  delay(DelayVal);
  TFTscreen.fillRect(0, 0, 160, 128, 0x0000);
  Serial.print(Input);
  Serial.print("\t"); // a space ' ' or  tab '\t' character is printed between the two values.
  Serial.println(Output/10);
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
  Serial.print(Input);
  Serial.print("\t"); // a space ' ' or  tab '\t' character is printed between the two values.
  Serial.println(Output/10);
  }
  } // END of LOOP
