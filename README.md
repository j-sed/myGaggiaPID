# myGaggiaPID
PID regulator using arduino nano for Gaggia classic (2005)

## Pipeline
* displayTest project to test displaying information to user
* PIDTest project to test SSR with thermocouple
* **ACTIVE** testing
* **ACTIVE** case build & integration to the gaggia
### Implemented
* The goal is to reach approximately 95 C. Some sources say, that the temperature drop between boiler and grouphead is approximately 10 degrees.
* The code is setup to aim at 100 C with PID cutoff at 101.0 C. Hard cutoff is set for 112 C to prevent excessive overheating if PID gets mad.
    * Implemented if loop to check if temp is not too high, otherwise turn the SSR off


## Prerequisites
### BOM
* Arduino nano CH340
* SSR-25 DA
* MAX 6675 & thermocouple type K
* TFT LCD 128x160
* sheet metal for display
* wires for high temps.

## PID

Used values found on arduino forum, which work quite well.

PID values:
* Kp = 53
* Ki = 0.5
* Kd = 56

Values for PID taken from: https://forum.arduino.cc/t/problems-with-regulate-a-coffee-machine-boiler-pid-ssr-relais/542488/26

Current output:
![capturePlotter.png](capturePlotter.png)

### AutoTune
Problem with autotune is the aggressivity. I have to learn how to use them properly.
* best values from autotune
    * SOME_OVERSHOOT_PID
        * Tu: 204.27  td: 5.89  Ku: 20.86  Kp: 6.95  Ki: 0.07  Kd: 472.91
        * Tu: 30.01  td: 1.08  Ku: 30.17  Kp: 10.05  Ki: 0.67  Kd: 100.49

    * PESSEN_INTEGRAL_PID
        * Tu: 30.94  td: 0.47  Ku: 28.47  Kp: 19.93  Ki: 1.61  Kd: 92.49

## Setup
### loading CH340
steps which were done to load arduino nano clone
* installed drivers as per: https://github.com/OLIMEX/ch340-dkms
* ls -l /dev/ttyUSB*
* sudo usermod -a -G dialout jsed

For the display using lcd 128x160 with pinout as follows
#### Display
![https://navody.dratek.cz/images/obr_clanky/74_lcd_160x128/74_lcd_160x128_schema.png](https://navody.dratek.cz/images/obr_clanky/74_lcd_160x128/74_lcd_160x128_schema.png)
https://navody.dratek.cz/images/obr_clanky/74_lcd_160x128/74_lcd_160x128_schema.png
#### Thermocouple
and the MAX6675 was connected as shown bellow. 
![](https://navody.dratek.cz/images/obr_clanky/18_termoclanek_driver/18_termoclanek_driver_schema.png)
https://navody.dratek.cz/images/obr_clanky/18_termoclanek_driver/18_termoclanek_driver_schema.png
#### Solid-state relay
The relay was connected with pin D7:(+) and GND:(-). 
### Code
#### Idea
The display will show actual temperature reading and below it will be the graph of the temperature, so that the user knows, when it is best to start brewing coffee.
#### Structure
Wanted features
* switch in the code to turn on/off the SSR
* possibly text saying that its good to start brewing 
* stopwatch - timer, so that the user knows how long the coffee is poured
* time

Sources:
Figures taken from navody.dratek.cz
Parts of code taken from navody.dratek.cz
