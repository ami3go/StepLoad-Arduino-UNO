/* Board UNO
 Custom shield on top with: 
 2x oled displays on the same bus 
 3x button
 1x encoder with button
 i2c memory
*/   
#include "src/TimerOne/TimerOne.h"         //https://github.com/PaulStoffregen/TimerOne
#include "src/TimerTwo/TimerTwo.h"         //https://github.com/theAndreas/TimerTwo
#include "src/ClickEncoder/ClickEncoder.h" //https://www.mikrocontroller.net/articles/Drehgeber
#include "src/Fsm/Fsm.h"                   //https://github.com/jonblack/arduino-fsm
#include "src/JC_Button/JC_Button.h"       //https://github.com/JChristensen/JC_Button
#include "src/LED/LED.h"          //
#include <avr/pgmspace.h> // libs for working with flash, writing and reading text constants 

//**********graphical oled stuff******************************************************** 
//Consume too much memory, currenly not used. Better use SSD1306Ascii *******************
//#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
//***************************************************************************************

//********** text oled library ********************************************************** 
//Wire.h also consume additional 8% of flash, so swtiched not AVRI2C 
//#include <Wire.h>              //*** Version for Arduino Wire Lib
//#include "src/Oled/SSD1306Ascii.h"      //*** https://github.com/greiman/SSD1306Ascii
//#include "src/Oled/SSD1306AsciiWire.h"  //*** 

#include "src/Oled/SSD1306Ascii.h"        //**** Version for Arduino Custome I2C Lib
#include "src/Oled/SSD1306AsciiAvrI2c.h"  //**** https://github.com/greiman/SSD1306Ascii
//***************************************************************************************

#include "src/SimpleCLI/SimpleCLI.h" // https://github.com/spacehuhn/SimpleCLI


#include "src/UserApplications/UserApplications.h" // sparated file for large code application 
#include "src/UserApplications/Variables_def.h"    // Constant and varialble collection for the project  


// 0X3C+SA0 - 0x3C or 0x3D  default address of OLED screen. 
#define I2C_ADDRESS1 0x3C //default adderss, left screen
#define I2C_ADDRESS2 0x3D //requre to change resistor possion on OLED befor soldering. Right screen 

// Define proper RST_PIN if required.
#define RST_PIN -1

//SSD1306AsciiWire oled1; // Version for Arduino Wire Lib
//SSD1306AsciiWire oled2; // 

SSD1306AsciiAvrI2c oled1; //Left  screen
SSD1306AsciiAvrI2c oled2; //Right screen  

// debug macros for an easy access to Serial.print function
// 
//#define DEBUG  // enable debug macros uncomment this line 
// use case : insert DEBUG_PRINT(x) in code. 
//

#ifdef DEBUG
 #define DEBUG_PRINT(x)        Serial.print (x)
 #define DEBUG_PRINTDEC(x)     Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)      Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x)
#endif



//***********************************************************
//******** VARIABLES ****************************************
//***********************************************************

ChannelTimerVar  CHAvar = {def_Ncycles, def_ONtime, def_OFFtime, def_CHState, 0, 0};
ChannelTimerVar  TMPvar = {def_Ncycles, def_ONtime, def_OFFtime, def_CHState, 0, 0};
unsigned int volatile  Nstarts=0;



// put string constant into FLASH memory to reduse RAM usage

#define strlenth 55        
//const static char StrNext_flash[strlenth]  PROGMEM = "Next: <Click>  Exit Menu: <Hold> -->     "; 
//const static char StrOn_flash[strlenth]    PROGMEM = "Edit   T_ON :   100  us   -   65  ms     ";  
//const static char StrOff_flash[strlenth]   PROGMEM = "Edit   T_OFF:   100  us   -   65  ms     ";
//const static char StrNrep_flash[strlenth]  PROGMEM = "Edit   N_rep:   1   -   500   times      ";
const static char StrStart_flash[strlenth]   PROGMEM = "START:  <Click>    N Runs:";
const static char StrEdit_flash[strlenth]    PROGMEM = "To Edit  Settings:   <Hold>   ----->   ";

const static char StrNext1_flash[strlenth]  PROGMEM = "To go Next:    Click Encoder  -->     "; 
const static char StrNext2_flash[strlenth]  PROGMEM = "To exit Menu:  Hold Encoder  -->      ";


const static char StrOn1_flash[strlenth]    PROGMEM = "[+] o o   [Edit]:   ON    time              "; 
const static char StrOn2_flash[strlenth]    PROGMEM = "Min val: 100  us      Max val: 65  ms       "; 

const static char StrOff1_flash[strlenth]   PROGMEM = "o [+] o   [Edit]:   OFF    time             ";
const static char StrOff2_flash[strlenth]   PROGMEM = "Min val: 100  us      Max val: 65  ms        ";

const static char StrNrep1_flash[strlenth]  PROGMEM = "o o [+]  [Edit]: number of pulses            ";
const static char StrNrep2_flash[strlenth]  PROGMEM = "Min val: 1            Max val: 500           ";


// RingBuffStruct  Vout = {0 , 0, 0, 0, 0, 0 };
   RingBuffStruct  Vout;


// char StepLoadResStatusStr[10];
// char  PreLoadResStatusStr[10];

// Create CLI Object
SimpleCLI cli;   

// Commands
Command cmdStart;
Command cmdSet;
Command cmdSetOntime;
Command cmdSetOfftime;
Command cmdSetNrep;
Command cmdGetstatus;
Command cmdGetVload;
Command cmdHelp;

void errorCallback(cmd_error* e); 


/****Rotary Encoder Driver with Acceleration SetUP.***************************
 * Supports: Open, Closed, Pressed, Held, Released, Clicked, DoubleClicked
 ***************************************************************************/
 
ClickEncoder *encoder; 
volatile int16_t EncoderValueLast,  EncoderValueUpdate;
ClickEncoder::Button EncoderButton;

/*******Hardware buttons**************************************************************
 * Hardware buttons
 * Button(pin, dbTime, puEnable, invert);
 * dbTime: Debounce time in milliseconds. Defaults to 25ms if not given. (unsigned long)
 * puEnable: true to enable the microcontroller's internal pull-up resistor, else false. Defaults to true if not given. (bool)
 * invert: false interprets a high logic level to mean the button is pressed, true interprets a low level as pressed. true should be used when a pull-up resistor is employed, false for a pull-down resistor. Defaults to true if not given. (bool)
 * 
 */
Button A_Button(7, 25, false, true);
Button B_Button(6, 25, false, true);
Button C_Button(5, 25, false, true);

/************************************************************* 
*  Hardware LED, located on Resistor BOX board 
*************************************************************/

LED LED_PWR(CHC);
LED LED_RES(CHD);

/********************************************
 * Custome applications declaration
 */

void TimerOneISR();
void WriteToHeadLine (char Str1[strlenth],char Str2[strlenth] );
void WriteToHeadLine2 (char Str11[strlenth],char Str12[strlenth],char Str21[strlenth],char Str22[strlenth]);

void waiting_encoder_button_release();
void ScreenUpdate(char Str1[20], char Str2[20]);
void UpdateValuesDisplOne();
void UpdateValuesDisplTwo();
void ClearHeadLine ();


/************************************************************
 ******** Starte machine applications ***********************
 ************************************************************/
 
/*********State Machine Trasition Signals *******************/

#define GoTo_OnTimeSetUp  0 
#define GoTo_OffTimeSetUp 1 
#define GoTo_NcyclesSetUp 2
#define GoTo_WaitingStart 3
#define GoTo_Running      4

/*********State Machine Applications ************************/
 
void enter_running();
void on_running();
void exit_running();

void enter_int();
void on_int();
void exit_int();

void enter_default_conf();
void on_default_conf();
void exit_default_conf();

void enter_waiting_start();
void on_waiting_start();
void exit_waiting_start();

void enter_setup_ontime();
void on_setup_ontime();
void exit_setup_ontime();

void enter_setup_offtime();
void on_setup_offtime();
void exit_setup_offtime();


void enter_setup_ncycles();
void on_setup_ncycles();
void exit_setup_ncycles();

/*********Finite Start Machine. State list: ************************/
State state_int(&enter_int, &on_int,&exit_int); //initial state
State state_default_conf(&enter_default_conf, &on_default_conf, &exit_default_conf); //fill up default configuration
State state_setup_ontime(&enter_setup_ontime, &on_setup_ontime, &exit_setup_ontime);  
State state_setup_offtime(&enter_setup_offtime, &on_setup_offtime, &exit_setup_offtime);
State state_setup_ncycles(&enter_setup_ncycles, &on_setup_ncycles, &exit_setup_ncycles);
State state_waiting_start(&enter_waiting_start, &on_waiting_start, &exit_waiting_start); //main working state  
State state_running(&enter_running, &on_running, &exit_running); //running generator state 

Fsm fsm(&state_int); // Starting state  

/**************** Timer two interrupt  *****************************
 ******  Used for Servise Application  *****************************
 */
void TimerTwoISR() {
  encoder->service();                         //encoder service command   
  writeToRingBuffer(&Vout, getLoadVoltage()); //measuring Vout block 
};/********************* Timer two interrupt END *********************/

/**************Timer One ISR **************************************
**************  Used for Output Pulses Generation *****************
*******************************************************************/
void TimerOneISR(){
    Generator(&CHAvar);
    if (CHAvar.CHState == DISABLE){
         Timer1.stop(); 
        };
}; /************ end of TimerOneISR ******************/

/****************Arduino setUp app start *********************************************
 ************************************************************************************* 
 *************************************************************************************/
void setup() {
    A_Button.begin();
    B_Button.begin();
    C_Button.begin();

    LED_PWR.off();
    LED_RES.off();
  
    digitalWrite(CHA, LOW);
    digitalWrite(CHB, LOW);
//  digitalWrite(CHC, LOW); used for LED
//  digitalWrite(CHD, LOW); used for LED
  
    pinMode(CHA, OUTPUT);
    pinMode(CHB, OUTPUT);
//  pinMode(CHC, OUTPUT); used for LED
//  pinMode(CHD, OUTPUT); used for LED
  
    Timer1.initialize(def_T1value); //microseconds
    Timer1.attachInterrupt(TimerOneISR); // 
  
    Timer2.EnableTimerInterrupt(TimerTwoISR, 1000); // microseconds
    Timer2.ResumeTimer();

    encoder = new ClickEncoder(2, 3, 4, 1 , LOW); // encoder setUp  (uint8_t A, uint8_t B, uint8_t BTN = -1)

    oled1.begin(&Adafruit128x64, I2C_ADDRESS1);
    oled2.begin(&Adafruit128x64, I2C_ADDRESS2);

    oled1.setFont(defFont);
    oled2.setFont(defFont);

   Nstarts = 0;
   Vout = {0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
 
  //  Nstarts=65530; // testing LCD view
    Serial.begin(UARTSpeed);

/******* Command Line interface int ********************************/
//Command start; getstatus; getvload; getvload ; 
//Command set, setontime; setofftime, set nrep ;
//Command help;

      cmdStart = cli.addCmd("start"); // Start command settings 
      cmdStart.addPosArg("delay", 0);
      const static char StrStart[] PROGMEM = "Start generating, trigger run button via uart"; 
      cmdStart.setDescription(StrStart);    

      cmdGetstatus = cli.addCmd("getstatus");  // getstatus command settings 
      const static char StrGetStatus[] PROGMEM ="getstatus returns state of variables like On/Off time, load voltage, etc... ";
      cmdGetstatus.setDescription(StrGetStatus);
 
      cmdGetVload = cli.addCmd("getvload");  // getstatus command settings 
      const static char StrGetVload[] PROGMEM ="getvload returns voltage measured on input of stepload board";
      cmdGetVload.setDescription(StrGetVload);

     
      cmdSet = cli.addCmd("set");  //Set commadn settigns 
      cmdSet.addArgument("ontime");
      cmdSet.addArgument("offtime");
      cmdSet.addArgument("nrep");
      const static char StrSet[] PROGMEM = "set the generator values like On/Off time, number pulses ";
      cmdSet.setDescription(StrSet);

      cmdSetOntime = cli.addCmd("setontime");  //Set commadn settigns 
      cmdSetOntime.addPosArg("ontimevalue", 100);
      const static char StrOntime[] PROGMEM = "Set the ON pulse duration time, in us"; 
      cmdSetOntime.setDescription(StrOntime);

      cmdSetOfftime = cli.addCmd("setofftime");  //Set commadn settigns 
      cmdSetOfftime.addPosArg("offtimevalue", 100);
      const static char StrOfftime[] PROGMEM = "Set the OFF pulse duration time, in us"; 
      cmdSetOfftime.setDescription(StrOfftime);

      cmdSetNrep = cli.addCmd("setnrep");  //Set commadn settigns 
      cmdSetNrep.addPosArg("nrepvalue", 3);
      const static char StrNrep[] PROGMEM = "Set the nubmer of pulses to be geenerated on a signle run"; 
      cmdSetOfftime.setDescription(StrNrep);

      cmdHelp = cli.addCmd("help"); //Help command settigns 
      const static char StrHelp[] PROGMEM = "Display help massage, command list and arguments ";
      cmdHelp.setDescription(StrHelp);

      cli.setOnError(errorCallback); // Set error Callback
        
/***********FSM TRANSITIONS  Settings start **************************************************************************/
/*1*/  fsm.add_timed_transition(&state_int,          &state_default_conf,  HomeScreenShowTime,         NULL);  // timed tansition 
/*2*/  fsm.add_timed_transition(&state_default_conf, &state_waiting_start, DefCongToWaitingStartDelay, NULL);  // timed tansition  

/*3*/  fsm.add_transition(&state_waiting_start, &state_setup_ontime,   GoTo_OnTimeSetUp, NULL); // hold encoder button to  change settings 
/*4*/  fsm.add_transition(&state_setup_ontime,  &state_setup_offtime,  GoTo_OffTimeSetUp, NULL); // click to navigate inside menu  
/*5*/  fsm.add_transition(&state_setup_offtime, &state_setup_ncycles,  GoTo_NcyclesSetUp, NULL); // click to navigate inside menu
/*6*/  fsm.add_transition(&state_setup_ncycles, &state_setup_ontime,   GoTo_OnTimeSetUp, NULL); //  click to navigate inside menu

/*7*/  fsm.add_transition(&state_setup_ontime,  &state_waiting_start, GoTo_WaitingStart, NULL); // hold encoder button to exit settings 
/*8*/  fsm.add_transition(&state_setup_offtime, &state_waiting_start, GoTo_WaitingStart, NULL); // hold encoder button to exit settings 
/*9*/  fsm.add_transition(&state_setup_ncycles, &state_waiting_start, GoTo_WaitingStart, NULL); // hold encoder button to exit settings 

/*10*/ fsm.add_transition(&state_waiting_start, &state_running,       GoTo_Running ,     NULL); // click to start cycle steop load 
/*11*/ fsm.add_transition(&state_running,       &state_waiting_start, GoTo_WaitingStart, NULL); // click to start cycle steop load 

/************* FSM TRANSITIONS  Settings END *******************************************/

}; /****************Arduino setUp app END **********************************************/

/*************** Arduino MAIN LOOP *****************************************************/
void loop() {
fsm.run_machine();// just keep state machine alive
  delay(1); 
};/*************** END OF Arduino MAIN LOOP ******************************************/

/************************************************************************************
 *************************** Finite start machine. State list ***********************
 ************************************************************************************/

/************* Begin (STATE running) *************/
void enter_running(){
  Timer2.StopTimer();
  CHAvar.CHState = ENABLE;
  copyVar2toVar1(&TMPvar, &CHAvar);
};

void on_running(){
  digitalWrite(CHB, HIGH);
  delay(PreLoadDelayBefore); // in ms Create delay after connectig preload 
  float N=0;
  N = (((CHAvar.ONtime + CHAvar.OFFtime)*CHAvar.Ncycles)/100);
  if (N<1) N=1;
  Timer1.start(); 
  delay((int) N); // create delay while timer is running and output working
  delay(PreLoadDelayAfter); // in ms
  digitalWrite(CHB, LOW);
  fsm.trigger(GoTo_WaitingStart);  
};

void exit_running(){
  copyVar2toVar1(&CHAvar, &TMPvar);
  Nstarts++;
  Serial.print(F("Run counter:"));
  Serial.print(Nstarts);
  Serial.println(F(" "));
  Timer2.ResumeTimer();
};
/************* END (STATE: running) ****************************************/

/************* Begin (STATE: int) ******************************************/
void enter_int(){
  //olde1 - left display
  oled1.clear();
  oled1.set2X();
  oled1.print(F("Step Load: ")); 
  oled1.setCursor(15,3);
  oled1.print(F("  V2.0  "));
  oled1.setCursor(0,6);
  oled1.set1X();
  oled1.print(F("Serial speed: "));
  oled1.print(UARTSpeed);
  //oled2 - right display
  oled2.set2X();
  oled2.clear();
  oled2.print(F("Visteon"));
  oled2.setCursor(0,3);
  oled2.print(F("March     2020")); 
  oled2.setCursor(0,5);
  oled2.print(F("SW ver:  "));
  oled2.print(SoftwareVersion);
  CLIWeclomeMassage(); // print to serial interface
  
 };
void on_int(){
   // for future use 
};
void exit_int(){
  //exit home screen animation 
   oled1.setCursor(0,7);
   oled1.print(F("Start: "));
   oled1.setCursor(50,7);
  
   for (int i = 0; i <= 10; i++){
    oled1.clearToEOL();
    oled1.print(F("*"));
    delay(HomeScreenShowTime); //value at Variables_def.h
  }
  
   oled1.clear();
   oled2.clear(); 
};
/************* END (STATE int) *********************************************/

/************* Begin (STATE default_conf) **********************************/
void enter_default_conf(){
     fillStructDefValues(&CHAvar);
};

void on_default_conf(){
};

void exit_default_conf(){
};
/************* END (STATE default_conf) ***********************************/

/************* Begin (STATE waiting_start) ********************************/
void enter_waiting_start(){
     
 //   waiting_encoder_button_release(); // to avoid mulltiple enter/exit on hold button 
    
    char Str1_temp[strlenth]; // creating temp string variable for reading from flash
    char Str2_temp[strlenth];

    readStrFromFlash(StrStart_flash,Str1_temp );
    readStrFromFlash(StrEdit_flash, Str2_temp );  

     ScreenUpdate(Str1_temp, Str2_temp);  // pring strings on OLED displays 
     
     oled1.set1X();                  // updated run counter on Oled screen 
     oled1.setCursor(94,0);          // 
     oled1.clearToEOL();             //     
     oled1.setCursor(100,0);         // 
     oled1.print(Nstarts);           // 
     
     CHAvar.counter = CHAvar.ONtime + CHAvar.OFFtime ;
     CHAvar.Ncycles_counter = CHAvar.Ncycles ;
     enableRingBuffer(&Vout);
     
     waiting_encoder_button_release(); // to avoid mulltiple enter/exit on hold button 
};

void on_waiting_start(){

// local variable that traks changes in state and update screen value only in case of value have been changed. 
static ResStatusStruct Res; 
static float   VoutLast;
 
 //************ Begin ************************************************************
 // avaragin voltage, ring buffer 
 // analog read has bugs with rundom zerro measurement 
 // in next section zeors detected, counted and excluded from avaragin
 //******************************************************************************
  if (Vout.Updated == 1){

      getAvarageVoltage(&Vout);

      if (Vout.AvarageValue != VoutLast) //update value on on changes 
      {
        VoutLast = Vout.AvarageValue;       
        oled2.set2X();                           // screen voltage update
        oled2.clearField(UpsPos - 10,UpsRow,5);  // screen voltage update
        oled2.setCursor(UpsPos,UpsRow);          // screen voltage update
        oled2.print(Vout.AvarageValue,3);        // screen voltage update
      }
  }; /*********avaragin voltage, ring buffer, END*********************************/

/*********** Update resistor status Begin ********************************************/
      if ( getPreLoadVoltage() > 0.2 ) Res.R1CurrentState = HIGH; // check if Pre Load resistor is connected 
      else  Res.R1CurrentState=LOW;
      
     if(Res.R1LastState != Res.R1CurrentState ){    // updated screen if state is changed                
         Res.R1LastState = Res.R1CurrentState;
         oled2.set1X();
         oled2.setCursor(65,PreLoadStatusRow);
         oled2.clearToEOL();
         if(Res.R1CurrentState==HIGH) oled2.print(F(StrResStatusOK)); 
         else oled2.print(F(StrResStatusNotOK)); 
      };
      
      if ( getStepLoadVoltage() > 0.2 ) Res.R2CurrentState = HIGH; // check if Step Load resistor is connected 
      else Res.R2CurrentState = LOW;
    
      if(Res.R2LastState != Res.R2CurrentState ){  // updated screen if state is changed     
         Res.R2LastState = Res.R2CurrentState; 
        oled2.set1X();
        oled2.setCursor(65,StepLoadStatusRow);
        oled2.clearToEOL();   
        if(Res.R2CurrentState==HIGH) oled2.print(F(StrResStatusOK)); 
        else oled2.print(F(StrResStatusNotOK)); 
        };
/************ Update resistor status END ***********************************************/

/******** LED indication Begin ******************************************************/
   if ( getLoadVoltage() > 0.5 ) {
    if(LED_PWR.getState() == LOW) LED_PWR.on();  
   }
      else{
        if(LED_PWR.getState() == HIGH)  LED_PWR.off();
      }
   
   if(( getStepLoadVoltage() > 0.2 )&( getPreLoadVoltage() > 0.2 )) {
          if(LED_RES.getState() == LOW) LED_RES.on(); 
          } 
          else {
           if(LED_RES.getState() == HIGH) LED_RES.off();
          }     
/******* LED indication END *********************************************************/
 
/******* Press "Start" button to RUN ************************************************/
  if (A_Button.read()) {               
     fsm.trigger(GoTo_Running); 
  }
/******* Press "B" to reduce value  *************************************************/  
  if (B_Button.read()) {
           CHAvar.OFFtime = makeValueInRange(CHAvar.OFFtime, -10,OFFtimeMIN,OFFtimeMAX);
           CHAvar.ONtime = makeValueInRange (CHAvar.ONtime,  -10,ONtimeMIN, ONtimeMAX);  
           UpdateValuesDisplOne();
           delay(ButtonDelayValue);         
          };
/******* Press "C" to increase value  ***********************************************/                    
  if (C_Button.read()) {
           CHAvar.OFFtime = makeValueInRange(CHAvar.OFFtime, 10,OFFtimeMIN,OFFtimeMAX);
           CHAvar.ONtime =  makeValueInRange(CHAvar.ONtime,  10,ONtimeMIN, ONtimeMAX); 
           UpdateValuesDisplOne();
           delay(ButtonDelayValue);      
          };
/******* Change value with encoder    ***********************************************/  
  EncoderValueUpdate = encoder->getValue();
        
  if (EncoderValueUpdate!= EncoderValueLast){
          CHAvar.Ncycles = makeValueInRange(CHAvar.Ncycles, EncoderValueUpdate,NcyclesMIN,NcyclesMAX);  
          EncoderValueLast = EncoderValueUpdate;
          UpdateValuesDisplOne();
          CHAvar.Ncycles_counter = CHAvar.Ncycles ;
          delay(ButtonDelayValue);         
         } ;       
  
/**********  Encoder button application process *****************************/
  EncoderButton = encoder->getButton();
  if (EncoderButton != ClickEncoder::Open) {
      switch (EncoderButton) {
      
       case ClickEncoder::Held:{
           fsm.trigger(GoTo_OnTimeSetUp); 
           break;
       }       
       case ClickEncoder::Clicked:{
            fsm.trigger(GoTo_Running); 
            break;
           }
       case ClickEncoder::DoubleClicked:{
            fillStructDefValues(&CHAvar);
            UpdateValuesDisplOne();  
            break;
            };    
      
          
        };
    };

/*********** Command Line interface section *********************************/

/****    Check if something available on uart ***********************************/
   if (Serial.available()) {
        String input = Serial.readStringUntil('\n');// Read out string from the serial monitor
        Serial.print(F("# "));// Echo the user input
        Serial.println(input);  
        cli.parse(input); // Parse the user input into the CLI
    }; /*  */

   if (cli.available()) {
      // Read out string from the serial monitor
      Command cmd = cli.getCommand();
      if(cmd == cmdSet) {
        Argument ontimeArg  = cmd.getArgument(F("ontime"));
        Argument offtimeArg = cmd.getArgument(F("offtime"));
        Argument nrepArg    = cmd.getArgument(F("nrep"));

        int  ontime_var  = ontimeArg.getValue().toInt()/10;
        int  offtime_var = offtimeArg.getValue().toInt()/10;
        int  nrep_var    = nrepArg.getValue().toInt();
    
        CHAvar.ONtime  = setValueInRange(CHAvar.ONtime, ontime_var,ONtimeMIN,ONtimeMAX);
        CHAvar.OFFtime = setValueInRange(CHAvar.OFFtime, offtime_var,OFFtimeMIN,OFFtimeMAX);
        CHAvar.Ncycles = setValueInRange(CHAvar.Ncycles, nrep_var,NcyclesMIN,NcyclesMAX);  

        UpdateValuesDisplOne(); 
        
  }
     //Run generator CMD!
     if(cmd == cmdStart) {
      Argument delayArg = cmd.getArgument(F("delay"));
      int  delayval = delayArg.getValue().toInt();
      delayval = setValueInRange(0, delayval ,0, 5000);
      if (delayval!= 0) delay(delayval);
      fsm.trigger(GoTo_Running);  
     };
     //Return variable paramentes 
     if(cmd == cmdGetstatus) printStatusToSerial( &CHAvar, Nstarts, Vout.AvarageValue);

     if(cmd == cmdGetVload) printVloadToSerial(Vout.AvarageValue);
          
     // Return Help massage.   
     if(cmd == cmdHelp) printHelptoSerial();
     
     if(cmd == cmdSetOntime) {
      Argument ontimeArg = cmd.getArgument(F("ontimevalue"));
      int  ontimeval = ontimeArg.getValue().toInt()/10;
      CHAvar.ONtime = setValueInRange(CHAvar.ONtime, ontimeval ,ONtimeMIN, ONtimeMAX);
      UpdateValuesDisplOne(); 
        }
     if(cmd == cmdSetOfftime) {
      Argument offtimeArg = cmd.getArgument(F("offtimevalue"));
      int  offtimeval = offtimeArg.getValue().toInt()/10;
      CHAvar.OFFtime = setValueInRange(CHAvar.OFFtime, offtimeval ,OFFtimeMIN, OFFtimeMAX);
      UpdateValuesDisplOne(); 
     }
      if(cmd == cmdSetNrep) {
      Argument nrepArg = cmd.getArgument(F("nrepvalue"));
      int  nrepval = nrepArg.getValue().toInt();
      CHAvar.Ncycles = setValueInRange(CHAvar.Ncycles, nrepval,NcyclesMIN,NcyclesMAX); 
      UpdateValuesDisplOne(); 
     } 


     
    } /*End of  if (cli.available()) */
/*
 * CLI END section
 */
};

void exit_waiting_start(){
//  DEBUG_PRINTLN("exit_waiting_start");
  disableRingBuffer (&Vout);
};
/************* END (STATE waiting_start) *************/

/************* Begin (STATE setup_ontime) *************/
void enter_setup_ontime(){
    //store string in flash to free up some RAM
    
    char Str11_temp[strlenth];
    char Str12_temp[strlenth];
    char Str21_temp[strlenth];
    char Str22_temp[strlenth];
    readStrFromFlash (StrOn1_flash, Str11_temp);
    readStrFromFlash (StrOn2_flash,Str12_temp);
    
    readStrFromFlash (StrNext1_flash,Str21_temp);
    readStrFromFlash (StrNext2_flash,Str22_temp);
    
    WriteToHeadLine2(Str11_temp,Str12_temp,Str21_temp,Str22_temp);
    
    waiting_encoder_button_release();
    EncoderValueLast = encoder->getValue(); 
    
};
void on_setup_ontime() {
 //   DEBUG_PRINTLN("on_setup_ontime");
        
       EncoderValueUpdate = encoder->getValue();
       
       if (EncoderValueUpdate!= EncoderValueLast){

           if (((abs(5*EncoderValueUpdate))>CHAvar.ONtime)&(EncoderValueUpdate<0)) CHAvar.ONtime = ONtimeMIN;
           else CHAvar.ONtime = makeValueInRange(CHAvar.ONtime, 5*EncoderValueUpdate,ONtimeMIN,ONtimeMAX);    
           EncoderValueLast = EncoderValueUpdate;
           UpdateValuesDisplOne();
           delay(ButtonDelayValue);     
         } ;
  
      EncoderButton = encoder->getButton();
     if (EncoderButton != ClickEncoder::Open) {
     switch (EncoderButton) {
      
       case ClickEncoder::Held:{
           fsm.trigger(GoTo_WaitingStart); 
           break;
           };               
       case ClickEncoder::Clicked:{
            fsm.trigger(GoTo_OffTimeSetUp); 
            break;
           };
 
    };
  }; 
  if (B_Button.read()){

           CHAvar.ONtime = makeValueInRange(CHAvar.ONtime, - 100,ONtimeMIN,ONtimeMAX);   
           UpdateValuesDisplOne();
           delay(ButtonDelayValue);             
           };
  if (C_Button.read()){

           CHAvar.ONtime = makeValueInRange(CHAvar.ONtime, 100,ONtimeMIN,ONtimeMAX);
           UpdateValuesDisplOne();
           delay(ButtonDelayValue);             
          };
  if (A_Button.read()){
           fsm.trigger(GoTo_OffTimeSetUp);
           delay(ButtonDelayValue);             
           };

   
};
void exit_setup_ontime(){
  //  DEBUG_PRINTLN("exit_setup_ontime");
};
/************* END (STATE setup_ontime) **************/

/************* Begin (STATE setup_offtime) ***********/
void enter_setup_offtime(){
  //  DEBUG_PRINTLN("enter_setup_offtime");
  //store string in flash to free up some RAM
    
    char Str11_temp[strlenth];
    char Str12_temp[strlenth];

    readStrFromFlash (StrOff1_flash, Str11_temp);
    readStrFromFlash (StrOff2_flash,Str12_temp);
    

    
    WriteToHeadLine2(Str11_temp,Str12_temp,0,0);

    waiting_encoder_button_release();
    EncoderValueLast = encoder->getValue(); 
};
void on_setup_offtime(){
//    DEBUG_PRINTLN("on_setup_offtime");
      
      EncoderValueUpdate = encoder->getValue();
      if (EncoderValueUpdate!= EncoderValueLast){
           
          if (((abs(5*EncoderValueUpdate))>CHAvar.OFFtime)&(EncoderValueUpdate<0)) CHAvar.OFFtime = OFFtimeMIN;
          else  CHAvar.OFFtime =  CHAvar.OFFtime = makeValueInRange(CHAvar.OFFtime, 5*EncoderValueUpdate,OFFtimeMIN,OFFtimeMAX);
          EncoderValueLast = EncoderValueUpdate;
          UpdateValuesDisplOne();
          delay(ButtonDelayValue);          
         } ;

        EncoderButton = encoder->getButton();
     if (EncoderButton != ClickEncoder::Open) {

     switch (EncoderButton) {
      
       case ClickEncoder::Held:{
           fsm.trigger(GoTo_WaitingStart); 
           break;
            };     
       case ClickEncoder::Clicked:{
            fsm.trigger(GoTo_NcyclesSetUp); 
            break;
           };
            
    };
  };
  if (B_Button.read()){

           CHAvar.OFFtime = makeValueInRange(CHAvar.OFFtime, -100,OFFtimeMIN,OFFtimeMAX);
           UpdateValuesDisplOne();
           delay(ButtonDelayValue);               
           };
  if (C_Button.read()){
           CHAvar.OFFtime = makeValueInRange(CHAvar.OFFtime, 100,OFFtimeMIN,OFFtimeMAX);
           UpdateValuesDisplOne();
           delay(ButtonDelayValue);             
          };
  if (A_Button.read()){
         fsm.trigger(GoTo_NcyclesSetUp);
         delay(ButtonDelayValue);           
           };
  
};
void exit_setup_offtime(){
  //  DEBUG_PRINTLN("exit_setup_offtime");
    
};
/************* END (STATE setup_offtime) *************/

/************* Begin (STATE setup_ncycles) ***********/
void enter_setup_ncycles(){
 //   DEBUG_PRINTLN("enter_setup_ncycles");
    char Str11_temp[strlenth];
    char Str12_temp[strlenth];

    readStrFromFlash (StrNrep1_flash, Str11_temp);
    readStrFromFlash (StrNrep2_flash,Str12_temp);
  
    WriteToHeadLine2(Str11_temp,Str12_temp,0,0);
    
    waiting_encoder_button_release();
    EncoderValueLast = encoder->getValue(); 
};
void on_setup_ncycles(){
 //   DEBUG_PRINTLN("on_setup_ncycles");
 
        EncoderValueUpdate = encoder->getValue();
        
      if (EncoderValueUpdate!= EncoderValueLast){
          CHAvar.Ncycles = makeValueInRange(CHAvar.Ncycles, EncoderValueUpdate,NcyclesMIN,NcyclesMAX);  
          EncoderValueLast = EncoderValueUpdate;
          UpdateValuesDisplOne();
          delay(ButtonDelayValue);       
         
         } ;
        
        EncoderButton = encoder->getButton();
     if (EncoderButton != ClickEncoder::Open) {
     
     switch (EncoderButton) {
      
       case ClickEncoder::Held:{
            fsm.trigger(GoTo_WaitingStart); 
            break;
            };
       case ClickEncoder::Clicked:{
            fsm.trigger(GoTo_OnTimeSetUp); 
            break;
            };
                 
    };
  };
  if (B_Button.read()){
           
           CHAvar.Ncycles = makeValueInRange(CHAvar.Ncycles, -10,NcyclesMIN,NcyclesMAX);  
           UpdateValuesDisplOne();
           delay(ButtonDelayValue);                
           };
  if (C_Button.read()){
            CHAvar.Ncycles = makeValueInRange(CHAvar.Ncycles, 10,NcyclesMIN,NcyclesMAX); 
           UpdateValuesDisplOne();
           delay(ButtonDelayValue);            
          };
  if (A_Button.read()){
          fsm.trigger(GoTo_OnTimeSetUp);
          delay(ButtonDelayValue);            
           };
 
};
void exit_setup_ncycles(){
    DEBUG_PRINTLN("exit_setup_ncycles");
};
/************* END (STATE setup_ncycles) *************/

void waiting_encoder_button_release(){
        unsigned int counter; 
        counter=0;  
        oled2.set1X();
        oled2.setCursor(0,7);
  while ((encoder->getButton() == ClickEncoder::Held)&(counter<= 20)){
        oled2.clearToEOL();
        oled2.print("*");
        delay(50);
        counter++;
  } ;
   oled2.setCursor(0,7);
   oled2.clearToEOL();
};
//void WriteToHeadLine (char Str1[strlenth],char Str2[strlenth] ){
//        if (Str1 != 0){  
//        oled1.set1X();
//        oled1.setCursor(0,0);
//        oled1.clearToEOL();
//        oled1.print(Str1);
//        }
//        
//        if (Str2 != 0){   
//        oled2.set1X();
//        oled2.setCursor(0,0);
//        oled2.clearToEOL();
//        oled2.print(Str2);
//        }
//}

void WriteToHeadLine2 (char Str11[strlenth],char Str12[strlenth],char Str21[strlenth],char Str22[strlenth]  ){
        if (Str11 != 0){  
        oled1.set1X();
        oled1.setCursor(0,0);
        oled1.clearToEOL();
        oled1.print(Str11);
        }
         if (Str12 != 0){  
        oled1.set1X();
        oled1.setCursor(0,1);
        oled1.clearToEOL();
        oled1.print(Str12);
        }        
        
        
        if (Str21 != 0){   
        oled2.set1X();
        oled2.setCursor(0,0);
        oled2.clearToEOL();
        oled2.print(Str21);
        }
        if (Str22 != 0){   
        oled2.set1X();
        oled2.setCursor(0,1);
        oled2.clearToEOL();
        oled2.print(Str22);
        }
}

void ClearHeadLine (){
      
        oled1.set1X();
        oled1.setCursor(0,0);
        oled1.clearToEOL();
        
        oled1.set1X();
        oled1.setCursor(0,1);
        oled1.clearToEOL();
    
        oled2.set1X();
        oled2.setCursor(0,0);
        oled2.clearToEOL();
                  
        oled2.set1X();
        oled2.setCursor(0,1);
        oled2.clearToEOL();
}




void ScreenUpdate(char Str1[strlenth], char Str2[strlenth]){
        ClearHeadLine ();
        WriteToHeadLine2(Str1,0,Str2,0);
        
        oled1.set2X();
        oled1.setCursor(0,OnTimeRow);
        oled1.print(F("T_on:"));
        oled1.setCursor(0,OffTimeRow);
        oled1.print(F("T_of:"));
        oled1.setCursor(0,NcycRow);
        oled1.print(F("N_rep:"));
        

        UpdateValuesDisplOne(); 

        UpdateValuesDisplTwo();
  
};
void UpdateValuesDisplOne(){
         
            oled1.set2X();
            oled1.clearField(55,OnTimeRow,5);
            oled1.setCursor(55,OnTimeRow);
            oled1.print((CHAvar.ONtime)*10);
            
            oled1.set1X();
            oled1.setCursor(115,OnTimeRow+1);   
            oled1.print(F("us"));
            
            if (CHAvar.ONtime==ONtimeMIN){
              oled1.set1X();
              oled1.setCursor(110,OnTimeRow);
              oled1.clearToEOL();
              oled1.print(F("min")); 
            };
            if (CHAvar.ONtime==ONtimeMAX){
              oled1.set1X();
              oled1.setCursor(110,OnTimeRow);
              oled1.clearToEOL();
              oled1.print(F("max")); 
            };
            if ((CHAvar.ONtime!=ONtimeMIN)&(CHAvar.ONtime!=ONtimeMAX)){
              oled1.set1X();
              oled1.setCursor(110,OnTimeRow);
              oled1.clearToEOL();
            };

            

            oled1.set2X();
            oled1.setCursor(0,OffTimeRow);
            oled1.print(F("T_of:"));
            oled1.clearField(55,OffTimeRow,5);
            oled1.setCursor(55,OffTimeRow);
            oled1.print((CHAvar.OFFtime)*10);
            oled1.set1X();
            oled1.setCursor(115,OffTimeRow+1);  
            oled1.print(F("us"));
            
            if (CHAvar.OFFtime==OFFtimeMIN){
              oled1.set1X();
              oled1.setCursor(110,OffTimeRow);
              oled1.clearToEOL();
              oled1.print(F("min")); 
            };
            if (CHAvar.OFFtime==OFFtimeMAX){
              oled1.set1X();
              oled1.setCursor(110,OffTimeRow);
              oled1.clearToEOL();
              oled1.print(F("max")); 
            };
            if ((CHAvar.OFFtime!=OFFtimeMIN)&(CHAvar.OFFtime!=OFFtimeMAX)){
              oled1.set1X();
              oled1.setCursor(110,OffTimeRow);
              oled1.clearToEOL();
            };  

            oled1.set2X();
            oled1.setCursor(65,NcycRow);
            oled1.clearToEOL();
            oled1.print(CHAvar.Ncycles);
            oled1.setCursor(100,NcycRow+1);   
            oled1.set1X();
            oled1.print(F("times"));
            if (CHAvar.Ncycles==NcyclesMIN){
              oled1.set1X();
              oled1.setCursor(100,NcycRow);
              oled1.clearToEOL();
              oled1.print(F("min")); 
            };
            if (CHAvar.Ncycles==NcyclesMAX){
              oled1.set1X();
              oled1.setCursor(100,NcycRow);
              oled1.clearToEOL();
              oled1.print(F("max")); 
            };
            if ((CHAvar.Ncycles!=NcyclesMIN)&(CHAvar.Ncycles!=NcyclesMAX)){
              oled1.set1X();
              oled1.setCursor(100,NcycRow);
              oled1.clearToEOL();
            };  
            
    
   };   
void UpdateValuesDisplTwo(){
        
          oled2.set2X();
          oled2.setCursor(0,UpsRow);
          oled2.print(F("U_ps:"));
          oled2.clearField(UpsPos - 10,UpsRow,5);
          oled2.setCursor(UpsPos,UpsRow);
          oled2.print(Vout.AvarageValue,3);
          
          
          oled2.setCursor(115,UpsRow);   
          oled2.print(F("V"));
          oled2.set1X();
          oled2.setCursor(0,StepLoadStatusRow);
          oled2.print(F("Step-Load res: "));
          

          oled2.setCursor(0,PreLoadStatusRow);
          oled2.print(F("Pre-Load res:"));  
};

/***********************************************************
 * *********************************************************
 * Command line interface section 
 * *********************************************************
 * *********************************************************
 */
 // Callback in case of an error
void errorCallback(cmd_error* e) {
    CommandError cmdError(e); // Create wrapper object

    Serial.print(F("ERROR: "));
    Serial.println(cmdError.toString());

    if (cmdError.hasCommand()) {
        Serial.print(F("Did you mean \""));
        Serial.print(cmdError.getCommand().toString());
        Serial.println(F("\"?"));
    }
};


 
