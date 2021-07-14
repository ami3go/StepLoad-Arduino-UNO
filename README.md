# Arduino StepLoad generator (Dynamic load or Transient Load)
Small and usefull tool on power supply's designer desk for quick check the stability of power supply. 
It provide fast load change form some preload (min load) to working load. 
Doing this an usfull inofmation on feedback loop could be extracted.
For more information on techincal side please read manual. 

<p align="center">
  <img src="https://github.com/ami3go/StepLoad-Arduino-UNO/blob/push/Pictures/model.png" width="1000" title="hover text">
</p>

<p align="center">
  <img src="https://github.com/ami3go/StepLoad-Arduino-UNO/blob/push/Pictures/DSC_2054.JPG" width="500" title="hover text">
</p>





More information about StepLoad (Dynamic load,Transient Load) you can read here:

https://www.siglenteu.com/application-note/power-supply-design-load-step/

https://www.testworld.com/wp-content/uploads/power-supply-testing.pdf

https://ridleyengineering.com/hardware/ap310-analyzer/ap300-application/step-load-testing.html

https://www.prodigit.com/show/power-supply-test.htm

Simple software for my custom project of StepLoad pulse generator
It could be used as nice tamplete for pulse generator project or etc. 


Main features are: 
- Fenite state machine (GUI/main code)
- Rottary encoder support (click and hold) 
- Command line interface (use onboard USB and serial port, SimpleCLI lib)
- Buttons support 
- Leds support 
- Timer One and Timet Two
- Two OLED displays (same bus different address, light SSD1306AsciiAvrI2c) 
Project have various lib, links to original source are provided in code. 
All libs are local to be able clone and compile project. 


Build (Arduino UNO, AtMega328 ):
- Flash 29794 byte (92%) from 32256 byte
- RAM 716 byte (34%) from 1332 byte
