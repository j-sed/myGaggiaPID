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
int timeNow;
int teplotaCAvg;
float tempCorrection = 0.0;
int tMaxHardLimit = 112;
String sensorVal;
unsigned int WindowSize = 1000;
unsigned int minWindow = 250;
unsigned long windowStartTime;
int timer = 0;
int timerDisplay = 0;
int timeOld = 0;
int timeNew = 0;
float teplotaC = 0;

//Define Variables we'll be connecting to
float Setpoint, Input, Output;
float tMaxStopHeat = Setpoint;

//Specify the links and initial tuning parameters
float Kp = 35, Ki = 5.0, Kd = 20;
float POn = 1.0;   // proportional on Error to Measurement ratio (0.0-1.0), default = 1.0
float DOn = 0.0;   // derivative on Error to Measurement ratio (0.0-1.0), default = 0.0

QuickPID myQuickPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, POn, DOn, QuickPID::DIRECT);

void updateDisplay(){
  // map(val, val_min,val_max,screen_min,screen_max)
  int graphHeight = map(teplotaC, 85, tMaxHardLimit, TFTscreen.height(), 20);
  int setPointHeight = map(Setpoint, 85, tMaxHardLimit, TFTscreen.height(), 20);
//  Serial.print("graphHeight:");    Serial.print(graphHeight);    Serial.println(",");
  timerDisplay += abs(timeOld - timeNew);
  if (timerDisplay > 500){
  TFTscreen.fillRect(1, setPointHeight, 160, 1, 004225);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.line(xPosOld, graphHeightOld, xPos,graphHeight);
  // TFTscreen.stroke(153, 229, 0);
  // TFTscreen.line(1,setPointHeight,160,setPointHeight);
  xPosOld = xPos;
  graphHeightOld = graphHeight;
  if (xPos >= 160) {

    xPos = 0;
    xPosOld =0;

    TFTscreen.background(0, 0, 0);
    CurrentTempDisplay();
  }

  else {
    
    xPos++;
    
  }
  timerDisplay = 0;
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

void PWMWrite(byte pin){
  
  if (millis() - windowStartTime >= WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
    // Output = WindowSize-Output;
    myQuickPID.Compute();  
  }
  if (
   (Input < (float) 103.00) &&
   ((unsigned int)Output > (millis() - windowStartTime))
   
  ){
    digitalWrite(pin, HIGH);
  } 
  else {
    digitalWrite(pin, LOW); 
  }
}

// void PWM(int Output){
//   if (Output <= 0.0) {
//     Output = 0.0;
//   }  
//   if (Output >= 1000.0) {
//     Output = 1000.0;
//   }
// }

void CurrentTempDisplay(){
  TFTscreen.setTextColor(0xFFFF);
  // nastavení kurzoru na pozici [x, y] = [40, 25]
  TFTscreen.setCursor(0, 0);
  TFTscreen.setTextSize(1);
  // tištění textu
  TFTscreen.print("Soucasna teplota:");
  TFTscreen.setCursor(0, 10);
  TFTscreen.setTextSize(1);
  TFTscreen.setTextColor(004225);
  TFTscreen.print("Idealni teplota:");
  TFTscreen.setCursor(160-35, 10);
  TFTscreen.print(Setpoint);
  TFTscreen.setTextColor(0xFFFF);
}
void tempReading(char * printout){
  int xStart = 160-35;  //60
  int yStart = 0;  //35
  int width = 35;   //35
  int height = 10;  //10
  TFTscreen.fillRect(xStart, yStart, width, height, 0x0000);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text(printout, xStart, yStart);
}

void setup(void) {
  // komunikace přes sériovou linku rychlostí 9600 baud
  Serial.begin(115200);  
  Setpoint = 102;
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
  // change the sampling time for PID to 5s
  myQuickPID.SetSampleTimeUs(5000);
  //tell the PID to range between 0 and the full window size
  myQuickPID.SetOutputLimits(0, WindowSize);
  //turn the PID on
  myQuickPID.SetMode(QuickPID::AUTOMATIC);
}


/********************* MAIN LOOP ***********************/
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
  tempReading(sensorPrintout);
  // convert the reading to a char array
  
  if ((teplotaC < tMaxHardLimit) && (teplotaC > -1))
  {

  CurrentTempDisplay();
  
  /************************************************
   *********************** PID ********************
     turn the output pin on/off based on pid output
   ************************************************/
  PWMWrite(SSR_PIN);
  Serial.print("Setpoint:");  Serial.print(Setpoint);  Serial.print(",");
  Serial.print("MaxTemp:");  Serial.print((int) 105);  Serial.print(",");
  Serial.print("Input:");     Serial.print(Input);     Serial.print(",");
  Serial.print("Output:");    Serial.print(Output/10);    Serial.println(",");
  // Serial.print(Input);
  // Serial.print("\t"); // a space ' ' or  tab '\t' character is printed between the two values.
  // Serial.println(Output);


  delay(200);

  }
  /************************************************
   ************* SECURITY LOOP ********************
     If temp is too high or error reading, then 
   ************************************************/
  else if (teplotaC >= tMaxHardLimit){
  TFTscreen.background(0, 0, 0);
  TFTscreen.fillRect(60, 35, 35, 8, 0x0000);
  TFTscreen.setTextColor(0xFFFF);
  // nastavení kurzoru na pozici [x, y] = [40, 25]
  TFTscreen.setCursor(60, 25);
  TFTscreen.setTextSize(2);
  // tištění textu
  TFTscreen.print("VYSOKA");
  TFTscreen.setCursor(60, 45);
  TFTscreen.setTextSize(2);
  TFTscreen.print("TEPLOTA");
  TFTscreen.setTextSize(1);
  digitalWrite(SSR_PIN, LOW);
  delay(250);
  TFTscreen.fillRect(0, 0, 160, 128, 0x0000);
  Serial.print(Input);
  Serial.print("\t"); // a space ' ' or  tab '\t' character is printed between the two values.
  Serial.println(Output/10);
  }else
  {
  TFTscreen.background(0, 0, 0);
  TFTscreen.setTextColor(0xFFFF);
  // nastavení kurzoru na pozici [x, y] = [40, 25]
  TFTscreen.setCursor(60, 25);
  TFTscreen.setTextSize(2);
  // tištění textu
  TFTscreen.print("CHYBA\nCIDLA");
  TFTscreen.setTextSize(1);
  digitalWrite(SSR_PIN, LOW);
  delay(250);
  TFTscreen.fillRect(0, 0, 160, 128, 0x0000);
  Serial.print(Input);
  Serial.print("\t"); // a space ' ' or  tab '\t' character is printed between the two values.
  Serial.println(Output/10);
  }
  } // END of LOOP
