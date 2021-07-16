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
int xPosOld = 0;
int graphHeightOld;
int DelayVal = 500;
int timeNow;
int teplotaCAvg;
float tempCorrection = 1.0;
int tMax = 125;
String sensorVal;
unsigned int WindowSize = 1000;
unsigned int minWindow = 250;
unsigned long windowStartTime;


const uint32_t sampleTimeUs = 500000; // 0.5s
//const byte inputPin = 0;
// const byte outputPin = 7;
const int outputMax = WindowSize;
const int outputMin = 0;
bool printOrPlotter = 1;  // on(1) monitor, off(0) plotter

byte outputStep = 100;
byte hysteresis = 1;
float setpoint = 96;       // 1/3 of range for symetrical waveform
int output = WindowSize/3;          // 1/3 of range for symetrical waveform

//Define Variables we'll be connecting to
float Setpoint, Input, Output;

//Specify the links and initial tuning parameters
float Kp = 0, Ki = 0, Kd = 0;
float POn = 0.5;   // proportional on Error to Measurement ratio (0.0-1.0), default = 1.0
float DOn = 0.0;   // derivative on Error to Measurement ratio (0.0-1.0), default = 0.0


bool pidLoop = false;

// QuickPID myQuickPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, POn, DOn, QuickPID::DIRECT);
QuickPID _myPID = QuickPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, POn, DOn, QuickPID::DIRECT);





int timer = 0;
int timeOld = 0;
int timeNew = 0;
float teplotaC = 0;


void updateDisplay() {
  int graphHeight = map(teplotaC, 0, tMax, TFTscreen.height(), 40);
  //  Serial.print("graphHeight:");    Serial.print(graphHeight);    Serial.println(",");
  TFTscreen.stroke(255, 255, 255);

  TFTscreen.line(xPosOld, graphHeightOld, xPos, graphHeight);
  xPosOld = xPos;
  graphHeightOld = graphHeight;
  if (xPos >= 128) {

    xPos = 0;
    xPosOld = 0;

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

float avg(int inputVal) {
  static int arrDat[16];
  static int pos;
  static long sum;
  pos++;
  if (pos >= 16) pos = 0;
  sum = sum - arrDat[pos] + inputVal;
  arrDat[pos] = inputVal;
  return (float)sum / 16.0;
}

float TCouple(){
  timer += abs(timeOld - timeNew);
  // Run low-frequency updates
  delay(100);
  if (timer > 200) {
    teplotaC = termoclanek.readCelsius() - tempCorrection;
    updateDisplay();
    timer = 0;
  }
  return teplotaC;
}

void PWMWrite(byte pin, int Output){
  // Serial.println("PWM function");
  // Serial.println(Output);
  // Serial.println(millis() - windowStartTime);
  if (millis() - windowStartTime >= WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  if (((unsigned int)Output > minWindow) && ((unsigned int)Output < (millis() - windowStartTime))) digitalWrite(pin, HIGH);
  else digitalWrite(pin, LOW); 
}
void PWMWriteAndCompute(byte pin, int Output){
  if (millis() - windowStartTime >= WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
    _myPID.Compute();
  }
  if (((unsigned int)Output > minWindow) && ((unsigned int)Output < (millis() - windowStartTime))) digitalWrite(pin, HIGH);
  else digitalWrite(pin, LOW); 
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

void setup(void) {
  // komunikace přes sériovou linku rychlostí 9600 baud
  Serial.begin(115200);
  pinMode(SSR_PIN, OUTPUT);
  Serial.println();
  if (constrain(output, outputMin, outputMax - outputStep - 5) < output) {
    Serial.println(F("AutoTune test exceeds outMax limit. Check output, hysteresis and outputStep values"));
    while (1);
  }
  // Switch to turn on the OUTPUT pin for SSR
  TFTscreen.begin();
  // fillRect(xStart,yStart,width,height,color)
  // Screen setting
  TFTscreen.background(0, 0, 0);
  TFTscreen.setTextColor(0xFFFF);


  // PID
  // Select one, reference: https://github.com/Dlloydev/QuickPID/wiki
  //_myPID.AutoTune(tuningMethod::ZIEGLER_NICHOLS_PI);
  //_myPID.AutoTune(tuningMethod::ZIEGLER_NICHOLS_PID);
  //_myPID.AutoTune(tuningMethod::TYREUS_LUYBEN_PI);
  //_myPID.AutoTune(tuningMethod::TYREUS_LUYBEN_PID);
  //_myPID.AutoTune(tuningMethod::CIANCONE_MARLIN_PI);
  //_myPID.AutoTune(tuningMethod::CIANCONE_MARLIN_PID);
  //_myPID.AutoTune(tuningMethod::AMIGOF_PID);
  // _myPID.AutoTune(tuningMethod::PESSEN_INTEGRAL_PID);
  // _myPID.AutoTune(tuningMethod::SOME_OVERSHOOT_PID);
  _myPID.AutoTune(tuningMethod::NO_OVERSHOOT_PID);
  //initialize the variables we're linked to
  windowStartTime = millis();
  //tell the PID to range between 0 and the full window size
  // myQuickPID.SetOutputLimits(0, WindowSize);

  //turn the PID on
  // myQuickPID.SetMode(QuickPID::AUTOMATIC);
  _myPID.autoTune->autoTuneConfig(outputStep, hysteresis, setpoint, output, QuickPID::DIRECT, printOrPlotter, sampleTimeUs);
}

void loop(void) {
  // načtení aktuální teploty termočlánku
  // do proměnné teplotaC
  //  TFTscreen.drawRect(0, 60, 160, 50,0xFFFF);

  timeOld = timeNew;
  timeNew = millis(); // Don't poll the temperature sensor too quickly
  // timer += abs(timeOld - timeNew);
  // Run low-frequency updates
  // if (timer > 200) {
  //   teplotaC = termoclanek.readCelsius() - tempCorrection;
  //   updateDisplay();
  //   timer = 0;
  // }
  // sensorVal = String(teplotaC);
  // sensorVal.toCharArray(sensorPrintout, 6);
  // convert the reading to a char array

  readCLI();
  ////// PID/////
  if (_myPID.autoTune) // Avoid dereferencing nullptr after _myPID.clearAutoTune()
  {
    switch (_myPID.autoTune->autoTuneLoop()) {
      case _myPID.autoTune->AUTOTUNE:
        Input = TCouple();
        // Serial.println(Input);
        // analogWrite(outputPin, Output);
        PWMWrite(SSR_PIN,Output);
        readCLI();
        break;

      case _myPID.autoTune->TUNINGS:
        _myPID.autoTune->setAutoTuneConstants(&Kp, &Ki, &Kd); // set new tunings
        _myPID.SetMode(QuickPID::AUTOMATIC); // setup PID
        _myPID.SetSampleTimeUs(sampleTimeUs);
        _myPID.SetTunings(Kp, Ki, Kd, POn, DOn); // apply new tunings to PID
        Setpoint = 100;
        Serial.println("tunings loop");
        readCLI();
        break;

      case _myPID.autoTune->CLR:
        if (!pidLoop) {
          _myPID.clearAutoTune(); // releases memory used by AutoTune object
          pidLoop = true;
          readCLI();
        }
        readCLI();
        break;
    }
  }
  if (pidLoop) {
    if (printOrPlotter == 0) { // plotter
      Serial.print("Setpoint:");  Serial.print(Setpoint);  Serial.print(",");
      Serial.print("Input:");     Serial.print(Input);     Serial.print(",");
      Serial.print("Output:");    Serial.print(Output);    Serial.println(",");
    }
    //    Input = _myPID.analogReadFast(inputPin);
    Input = TCouple();
    PWMWriteAndCompute(SSR_PIN,Output);
    readCLI();
  }
} // END of LOOP
