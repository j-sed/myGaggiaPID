#include <Ucglib.h>

// LCD ST7735 128x160 SPI

// připojení potřebných knihoven
#include <SPI.h>
#include <max6675.h>
#include "Ucglib.h"
#include <TFT.h>

// nastavení propojovacích pinů
#define RESET 8
#define DC 9
#define CSLCD 10
#define RELE_PIN 7
int pinSO  = 3;
int pinCS  = 4;
int pinSCK = 5;
// inicializace displeje z knihovny
//Ucglib_ST7735_18x128x160_HWSPI ucg(DC, CSLCD, RESET);
TFT TFTscreen = TFT(CSLCD, DC, RESET);
MAX6675 termoclanek(pinSCK, pinCS, pinSO);
int xPos = 0;
char sensorPrintout[6];
void setup(void) {
  // komunikace přes sériovou linku rychlostí 9600 baud
  Serial.begin(9600);
//  pinMode(RELE_PIN, OUTPUT);

  // nastavení průhledného pozadí písma
  //  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  //  // nastavení plného pozadí písma
  //  //ucg.begin(UCG_FONT_MODE_SOLID);
  //  // otočení displeje o 270 stupňů
  //  ucg.setRotate270();
  //  // nastavení fontu,
  //  // ukázka dalších fontů zde:
  //  // https://github.com/olikraus/ucglib/wiki/fontsize
  //  ucg.setFont(ucg_font_helvR12_tr);
  //  // vyčištění obsahu displeje
  //  ucg.clearScreen();
  //  // nastavení barvy na bílou (barva ve složkách RGB,
  //  // červená-zelená-modrá v intenzitě 0-255)
  //  ucg.setColor(255, 255, 255);
  //  // nastavení kurzoru na pozici [x, y] = [40, 25]
  //  ucg.setPrintPos(40, 25);
  //  // tištění textu
  //  ucg.print("Mereni teploty");
  //  // ucg.setPrintPos(10, 50);
  //  // // nastavení barvy na červenou
  //  // ucg.setColor(255, 0, 0);
  //  // ucg.println("dratek.cz!");
  //  ucg.setPrintPos(40, 75);
  //  // nastavení barvy na zelenou
  //  ucg.setColor(0, 255, 0);
  //  ucg.print("Teplota:");
  //  ucg.setPrintPos(40, 125);
  //  // nastavení barvy na šedou
  //  ucg.setColor(127, 127, 127);
  //  ucg.println(" Stupnu Celsia");
  //  // reference všech dostupných příkazů:
  //  // https://github.com/olikraus/ucglib/wiki/reference

  TFTscreen.begin();
  TFTscreen.background(0, 0, 0);
  TFTscreen.setTextColor(0xFFFF);
  // nastavení kurzoru na pozici [x, y] = [40, 25]
  TFTscreen.setCursor(40, 25);
  TFTscreen.setTextSize(1);
  // tištění textu
  TFTscreen.print("Mereni teploty");
}

void loop(void) {
  // načtení aktuální teploty termočlánku
  // do proměnné teplotaC
  float teplotaC = termoclanek.readCelsius();
  //  // nastavení barvy na černou
  //  ucg.setColor(0, 0, 0);
  //  // nakresli vyplněný box danou barvou
  //  // od souřadnic [0, 80] 160 pixelů do délky
  //  // a 20 pixelů na výšku
  //  ucg.drawBox(0, 80, 160, 20);
  //  // nastavení barvy na modrou
  //  ucg.setColor(0, 0, 255); // modrá
  //  ucg.setPrintPos(60, 100);
  //  // vytištění čísla - počet vteřin od
  //  // zapnutí Arduina
  //  ucg.print(teplotaC);
  // pauza po dobu 1 vteřiny
  //  TFTscreen.drawRect(0, 60, 160, 50,0xFFFF);
  String sensorVal = String(teplotaC);
  // convert the reading to a char array
  sensorVal.toCharArray(sensorPrintout, 6);

  TFTscreen.fillRect(35, 50, 90, 40, 0xFFFF);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text(sensorPrintout, 40, 70);
  int graphHeight = map(teplotaC, 0, 160, 0, TFTscreen.height());
  Serial.println(graphHeight);
  TFTscreen.stroke(255, 255, 255);

  TFTscreen.line(xPos, TFTscreen.height() - graphHeight, xPos, TFTscreen.height() - graphHeight + 2);
  if (xPos >= 160) {

    xPos = 0;

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
//  digitalWrite(RELE_PIN, HIGH);
  delay(1000);
//  digitalWrite(RELE_PIN, LOW);
}
