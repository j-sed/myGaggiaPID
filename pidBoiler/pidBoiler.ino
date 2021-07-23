// LIBS
#include <Ucglib.h>
#include <SPI.h>
#include <max6675.h>
#include "Ucglib.h"
#include <TFT.h>
#include "QuickPID.h"
#include <cppQueue.h>
#include "Ticker.h"
#include <MemoryFree.h>

// PIN DEF
// display
#define RESET 8
#define DC 9
#define CSLCD 10
#define LCDON 6
// ssr
#define SSR_PIN 7
// max6675
#define pinSO 3
#define pinCS 4
#define pinSCK 5
// Notes::
// possible to use: int pinCS = 4;
// maybe will be forced to, as it uses less memory

// **** DECLARATION **** //
// Variables
int xPos = 0;
char sensorPrintout[6];
bool SSRStat = true;
int xPosOld = 0;
int graphHeightOld = 104.0;
float tempCorrection = 0.0;
const uint16_t tMaxHardLimit = 112;
String sensorVal;
const uint16_t WindowSize = 1000;
const uint16_t minWindow = 250;
unsigned long windowStartTime;
uint16_t timer = 0;
uint16_t timerDisplay = 0;
uint16_t timeOld = 0;
uint16_t timeNew = 0;
const uint16_t tempDispXPosition = 160-35;
float teplotaC = 104.0;
bool runCheck = true;
float in = 0;
//Define Variables we'll be connecting to
float Setpoint, Input, Output;

//Specify the links and initial tuning parameters
// float Kp = 35, Ki = 5.0, Kd = 20;
float Kp = 45, Ki = 1.0, Kd = 25;
float POn = 1.0;   // proportional on Error to Measurement ratio (0.0-1.0), default = 1.0
float DOn = 0.0;   // derivative on Error to Measurement ratio (0.0-1.0), default = 0.0

// Functions
void checkActivity();
void lcdOnOff();

// **** INITIALIZATION **** //
TFT TFTscreen = TFT(CSLCD, DC, RESET);
MAX6675 termoclanek(pinSCK, pinCS, pinSO);
QuickPID myQuickPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, POn, DOn, QuickPID::DIRECT);
// FIFO queues 
cppQueue tempAvg(sizeof(in), 2, FIFO);	// Instantiate queue
cppQueue resetSampleTime(sizeof(in),2,FIFO);
// Timers
Ticker timer1(checkActivity, 1000000, 0, MICROS_MICROS);
// Ticker timer2(lcdOnOff,      120000000, 0, MICROS_MICROS);

/************************************************
******************** FUNCTIONS ******************
            definitions & declarations 
*************************************************/

// Function to create average from two readings
// created to further smooth-out the curve on display
float avgTemp(){
  tempAvg.push(&teplotaC);
  float temp1, temp2;
  tempAvg.peekIdx(&temp1, (uint16_t)0);
  tempAvg.peekIdx(&temp2, (uint16_t)1);
  tempAvg.drop();
  return (temp1+temp2)/2.0;
}
void updateDisplay(){
  // map(val, val_min,val_max,screen_min,screen_max)
  
  
  timerDisplay += abs(timeOld - timeNew);
  if (timerDisplay > (uint16_t) 500){
    
  uint16_t graphHeight = map(avgTemp(), 85, tMaxHardLimit, TFTscreen.height(), 20);
  uint16_t setPointHeight = map(Setpoint, 85, tMaxHardLimit, TFTscreen.height(), 20);

  TFTscreen.fillRect(1, setPointHeight, 160, 1, 004225);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.line(xPosOld, graphHeightOld, xPos,graphHeight);
  // TFTscreen.stroke(153, 229, 0);
  // TFTscreen.line(1,setPointHeight,160,setPointHeight);
  xPosOld = xPos;
  graphHeightOld = graphHeight;
  if (xPos >= (uint16_t) 160) {

    xPos = 0;
    xPosOld =0;
    lcdOnOff();
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
  if (timer > (uint16_t) 200) {
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
    if(command == "temp")
    {
      Serial.println(teplotaC); // send action to Serial Monitor
    }
  }
}

void PWMWrite(byte pin){
  
  if (millis() - windowStartTime >= WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
    myQuickPID.Compute();  
  }
  if (
   (Input < (float) 105.00) &&
   ((uint16_t)Output > (millis() - windowStartTime))
   
  ){
    digitalWrite(pin, HIGH);
  } 
  else {
    digitalWrite(pin, LOW); 
  }
}

void CurrentTempDisplay(){
  TFTscreen.setTextColor(0xFFFF);
  TFTscreen.setCursor(0, 0);
  TFTscreen.setTextSize(1);
  TFTscreen.print(F("Soucasna teplota:"));
  TFTscreen.setCursor(0, 10);
  TFTscreen.setTextSize(1);
  TFTscreen.setTextColor(004225);
  TFTscreen.print(F("Idealni teplota:"));
  TFTscreen.setCursor(tempDispXPosition, 10);
  TFTscreen.print(Setpoint);
  TFTscreen.setTextColor(0xFFFF);
}
void tempReading(char * printout){
  const uint16_t yStart = 0;  //35
  const uint16_t width = 35;   //35
  const uint16_t height = 10;  //10
  TFTscreen.fillRect(tempDispXPosition, yStart, width, height, 0x0000);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text(printout, tempDispXPosition, yStart);
}
// Function to battle against large Integral part from long inactivity
void checkActivity(){
  resetSampleTime.push(&teplotaC);
  float temp1, temp2;
  resetSampleTime.peekIdx(&temp1, (uint16_t)0);
  resetSampleTime.peekIdx(&temp2, (uint16_t)1);
  resetSampleTime.drop();
  if ((temp2-temp1>= (float )2.0) && (teplotaC < (float) 85.0)){
    runCheck = false;
    myQuickPID.SetMode(QuickPID::MANUAL);
    while(TCouple() < Setpoint-10){
      digitalWrite(SSR_PIN,HIGH);
      delay(500);
    }
    myQuickPID.SetMode(QuickPID::AUTOMATIC);
  }


}

void lcdOnOff(){
  digitalWrite(LCDON, LOW);
  delay(100);
  digitalWrite(LCDON, HIGH);
  TFTscreen.begin();
  TFTscreen.setRotation(1);
    // Screen setting
  TFTscreen.background(0, 0, 0);
  TFTscreen.setTextColor(0xFFFF);
}

/********************* SETUP FUNCTION ***********************/
void setup(void) {
  pinMode(LCDON, OUTPUT);
  digitalWrite(LCDON, HIGH);
  Serial.begin(115200);  
  Setpoint = 104;
  tempAvg.push(&Setpoint);
  resetSampleTime.push(&Setpoint);
  timer1.start();
  // timer2.start();

// Switch to turn on the OUTPUT pin for SSR 
  switch(SSRStat) {
  case true:
    pinMode(SSR_PIN, OUTPUT);
    break;
  }
  TFTscreen.begin();
  TFTscreen.setRotation(1);
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
  //  TFTscreen.drawRect(0, 60, 160, 50,0xFFFF);
  readCLI();
  timeOld = timeNew;
  timeNew = millis(); // Don't poll the temperature sensor too quickly
  if (runCheck) timer1.update();
  if (teplotaC > (float) 100.0) runCheck = true;
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
  TFTscreen.setRotation(1);
  // Serial.print("freeMemory()="); Serial.println(freeMemory());
  // timer2.update();
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
  TFTscreen.setCursor(60, 25);
  TFTscreen.setTextSize(2);
    TFTscreen.print(F("VYSOKA"));
  TFTscreen.setCursor(60, 45);
  TFTscreen.setTextSize(2);
  TFTscreen.print(F("TEPLOTA"));
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
  TFTscreen.setCursor(60, 25);
  TFTscreen.setTextSize(2);
  TFTscreen.print(F("CHYBA\nCIDLA"));
  TFTscreen.setTextSize(1);
  digitalWrite(SSR_PIN, LOW);
  delay(250);
  TFTscreen.fillRect(0, 0, 160, 128, 0x0000);
  Serial.print(Input);
  Serial.print("\t"); // a space ' ' or  tab '\t' character is printed between the two values.
  Serial.println(Output/10);
  }
  } // END of LOOP
