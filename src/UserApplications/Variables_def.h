#ifndef VARIABLES_DEF_H
#define VARIABLES_DEF_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif



#define SoftwareVersion 3.0 //current build 
#define UARTSpeed 115200    //

#define ENABLE      0x0F
#define DISABLE     0xF0

#define def_Ncycles  3 
#define def_ONtime   10 //*10us
#define def_OFFtime  10 //*10us
#define def_CHState  DISABLE 

#define ONtimeMAX   6500 //*10us
#define ONtimeMIN   10   //*10us

#define OFFtimeMAX   ONtimeMAX //*10us
#define OFFtimeMIN   ONtimeMIN //*10us

#define NcyclesMAX  500
#define NcyclesMIN  1 

#define ButtonDelayValue 150
/*
 *  Pins configuration 
 */

#define CHA 10  //  stepload mosfet
#define CHB 11  // preload mosfet
//#define CHA 12
//#define CHB 11
#define CHC 9   //LEDs power OK
#define CHD 8   //LEDS resistor OK 

/*  States time constants 
 *  ms
 */
#define HomeScreenShowTime 200 /* ms */
#define DefCongToWaitingStartDelay 100  /* ms */
#define PreLoadDelayBefore 10  /* ms delay after applying preload and before star StepLoad */ 
#define PreLoadDelayAfter  10   /* ms delay after finish StepLoad and release preload  */  

/*  Timer One interrupt period 
 *  Timer1.initialize(def_T1value); Begin using the timer. This function must be called first. "microseconds" is the period of time the timer takes.
 */
#define def_T1value 10 /* microseconds */ 


/*
 * Structure definitions for generator 
 */
typedef struct{
   unsigned int  Ncycles;
   unsigned int  ONtime;
   unsigned int  OFFtime;
            byte CHState;
   unsigned long counter;
   unsigned int  Ncycles_counter;
} ChannelTimerVar;

/*
 * Structure definitions  Ring buffer for Output voltage 
 */
#define RingBuffIndexMax 20 
typedef struct{
   float Buff[RingBuffIndexMax + 1];
   float AvarageValue;
   byte BuffIndex;
   byte Updated;
   byte SampleCounter;
   byte Enable;         
} RingBuffStruct;

/*
 * Structure definitions for  Resistors and LEDs status checker  
 * currenly no in use
 */
typedef struct{
   byte R1CurrentState;
   byte R1Updated;
   String R1text;
   byte R1LastState; 
   byte R2CurrentState;
   byte R2Updated;
   String R2text;
   byte R2LastState;
   byte LEDResCurrentState;
   byte LEDResUpdated;
   byte LEDPwrCurrentState;
   byte LEDPwrUpdated;          
} ResStatusStruct;



/*
 *  Oled constants 
 */
 #define OnTimeRow 2
 #define OffTimeRow 4
 #define NcycRow 6
 #define UpsRow 2
 #define UpsPos 65
 #define PreLoadStatusRow 4
 #define StepLoadStatusRow 5
 #define StrResStatusOK    "Connected"
 //#define StrResStatusNotOK "***NOT***"
 #define StrResStatusNotOK "NOT Detected"  
  
 //#define defFont Callibri10
 #define defFont Iain5x7 //works good


 

/*
 *  Divider ratio constants 
 */

#define Kdiv 3.97


#endif  
