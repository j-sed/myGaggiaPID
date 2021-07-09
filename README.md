# myGaggiaPID
PID regulator using arduino nano for Gaggia classic (2005)
## Pipeline
* displayTest project to test displaying information to user
* **ACTIVE** PIDTest project to test SSR with thermocouple
* testing
* case build & integration to the gaggia
### Implemented
* Implemented if loop to check if temp is not too high, otherwise turn the SSR off

## Prerequisites
### BOM
* Arduino nano CH340
* SSR-25 DA
* MAX 6675 & thermocouple type K
* TFT LCD 128x160
**In the future**
* sheet metal for display
* wires for high temps.
### loading CH340
steps which were done to load arduino nano clone
* installed drivers as per: https://github.com/OLIMEX/ch340-dkms
* ls -l /dev/ttyUSB*
* sudo usermod -a -G dialout jsed
## Setup
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
### PID
PID values:
* Kp = 10
* Ki = 50
* Kd = 30
Values for PID taken from: https://www.reddit.com/r/Coffee/comments/20jre7/how_to_pid_a_gaggia_classic_the_right_way/

Sources:
Figures taken from navody.dratek.cz
Parts of code taken from navody.dratek.cz
