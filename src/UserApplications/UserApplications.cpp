#include "UserApplications.h"
#include "Variables_def.h"

/************************************************************************************
********************* Generator app ************************************************         
************************************************************************************/
void Generator(ChannelTimerVar *var ){
 if(var->CHState == ENABLE){
  if (var->Ncycles_counter == 0){
       var->CHState = DISABLE;
  }
  if (var->counter == var->ONtime){
       digitalWrite(CHA, HIGH);
       }
  if (var->counter == 0){
       digitalWrite(CHA, LOW);
       var->Ncycles_counter--;
       var->counter = var->ONtime + var->OFFtime ; 
      } 
  var->counter--;      
  }
  
};

/************************************************************************************
********************* fill Structure with Default Values app ***********************        
************************************************************************************/
void fillStructDefValues(ChannelTimerVar *var ){
      var->Ncycles = def_Ncycles; 
      var->ONtime  = def_ONtime; 
      var->OFFtime = def_OFFtime;
      var->CHState = ENABLE;
      var->counter = var->ONtime + var->OFFtime ;
      var->Ncycles_counter = var->Ncycles ;
      
};
/************************************************************************************
********************* Copy Structure Variable 2 to Varible 1 app *******************         
************************************************************************************/
void copyVar2toVar1(ChannelTimerVar *var1, ChannelTimerVar *var2){
  var1->Ncycles = var2->Ncycles; //
  var1->ONtime = var2->ONtime; // 
  var1->OFFtime = var2->OFFtime;
  var1->counter = var2->counter ;
  var1->Ncycles_counter= var2->Ncycles_counter ;  
};

/************************************************************************************
***** Increase/Decriase Return value and check it with Min and Max rage app ********         
************************************************************************************/
unsigned int makeValueInRange(unsigned int const CurrentValue, int const varValue, 
                             unsigned int const  minValue, unsigned int const    maxValue){
    //check precondition 
    if (minValue > maxValue ) return 0;
    if (varValue == 0 ) return CurrentValue;
    
    if (varValue < 0){
       unsigned int returnValue;
       
       if(CurrentValue > abs(varValue)) {
           returnValue = CurrentValue + varValue;
           if (returnValue >= minValue) return returnValue;
           else  return minValue;
       }
    
        else {
          return minValue; 
        }
        
       }
       else {  
    // check main conditions  
    if((CurrentValue + varValue) <= minValue ) return minValue;
    else if((CurrentValue + varValue) <= maxValue) return (CurrentValue + varValue); 
    else if((CurrentValue + varValue) > maxValue)  return  maxValue;  
       }
};

unsigned int setValueInRange(unsigned int const CurrentValue, int const newValue, 
                             unsigned int const  minValue, unsigned int const    maxValue){
    //check precondition 
    if (minValue > maxValue ) return 0;
    if (newValue == 0 ) return CurrentValue;
    if (newValue == CurrentValue ) return CurrentValue;
    if (newValue < 0) {
            return CurrentValue;
          }
       else {  
    // check main conditions  
    if((newValue) <= minValue ) return minValue;
    else if((newValue) <= maxValue) return (newValue); 
    else if((newValue) > maxValue)  return  maxValue;  
       }
};

void readStrFromFlash (char *flashStr, char *returnStr){
	 byte k; // counter variable
    int len; //variable for number of characters to read   
    
    len = strlen_P(flashStr);
    for(k=0; k<len; k++ ){
      returnStr[k] = pgm_read_byte_near(flashStr + k); // reading from flash 
         }; 




};




/************************************************************************************
***** Get avarage voltage form ring buffer  *****************************************         
************************************************************************************/
void getAvarageVoltage(RingBuffStruct *Var){
        
      float summ = 0;         // variable for summ of ring buffer
      byte ZeroCounter = 0;   // counter for number of zerros in the buffer     
      
      Var->AvarageValue = 0;  //  
      Var->Updated = 0;       // Avarage value update indicator  
      
      for (int i = 0; i <= (RingBuffIndexMax); i++){  //go through the ring buffer
            if ( Var->Buff[i] == 0) ZeroCounter++;    // search for zeros 
            else summ = Var->Buff[i] + summ;          // sum up ring buffer valaues 
             };
      if (ZeroCounter > (RingBuffIndexMax/2)) {       // if half of the buffes is zero 
          Var->AvarageValue = 0;                      // probably output voltage is zeor too 
        }
      else {
        Var->AvarageValue =  summ/((RingBuffIndexMax+1) - ZeroCounter);  //exclude zeros for summ 
        }
      
};

void writeToRingBuffer(RingBuffStruct *Var, float Voltage){
  
  if ((Var->SampleCounter == 0) && (Var->Enable == 1)){  
  Var->Buff[Var->BuffIndex] = Voltage;    
  Var->Updated = 1;
  Var->SampleCounter = 100;
  Var->BuffIndex++;
  if (Var->BuffIndex > RingBuffIndexMax ) Var->BuffIndex = 0;
   }
  Var->SampleCounter--;
};


void enableRingBuffer (RingBuffStruct *Var){
     Var->Enable = 1;
};
void disableRingBuffer (RingBuffStruct *Var){
     Var->Enable = 0;
}



/**************************************************************************
*************** ADC BLOCK *************************************************
**************************************************************************/

float getVoltage(uint8_t pin) {
  float voltage = 0;
  voltage = (analogRead(pin)*0.0048828)*Kdiv;
  return   voltage;    
};

float getLoadVoltage(){
      return getVoltage(A0);
};
float getPreLoadVoltage(){
      return getVoltage(A2);
};
float getStepLoadVoltage(){
      return getVoltage(A1);
};
int getBoardID(){
      return analogRead(A3);
};

/* ********* Command Lien USER application *****************************************/
void CLIWeclomeMassage(void){
  Serial.println(F("Visteon, Hardware Team, Sofia, March 2020 "));  
  Serial.print(F("Step Load: v2, software version: "));
  Serial.print(SoftwareVersion);
  Serial.println(F(" "));
  Serial.println(F("Welcome to Stepload Commadn Line Interface (CLI): "));
  Serial.println(F("Type 'help' to get supported command list "));
};

//void printStatusToSerial(ChannelTimerVar const *var, int const Nruns, float const Vload){
//      Serial.println(" ");   
//      Serial.print(F("ONtime = "));
//      Serial.print(var->ONtime*10);
//      Serial.print(F("us"));
//       
//      Serial.print(F("  OFFtime = ")); 
//      Serial.print(var->OFFtime*10);
//      Serial.print(F("us"));
//      
//      Serial.print(F("  Ncycles = "));
//      Serial.print(var->Ncycles);
//      
//      Serial.print(F("  Nruns = "));
//      Serial.print(Nruns);
//
//      Serial.print("  Vload = ");
//      Serial.print(Vload,3);
//      Serial.print("V");
//      Serial.println(" "); 
//};
void printStatusToSerial(ChannelTimerVar const *var, unsigned int const Nruns, float const Vload){
      Serial.println(F(" "));   
      Serial.print(F("ONtime[us]= "));
      Serial.print(var->ONtime*10);
      //Serial.print(F("us"));
       
      Serial.print(F("; OFFtime[us]= ")); 
      Serial.print(var->OFFtime*10);
     // Serial.print(F("us"));
      
      Serial.print(F("; Ncycles = "));
      Serial.print(var->Ncycles);
      
      Serial.print(F("; Nruns = "));
      Serial.print(Nruns);

      Serial.print(F("; Vload[V]= "));
      Serial.print(Vload,3);
      //Serial.print("V");
      Serial.println(F(" ")); 
};

void printVloadToSerial(float const Vload){
      Serial.println(F(" "));   
      Serial.print(F(" Vload [V]= "));
      Serial.print(Vload,3);
      Serial.println(F(" ")); 
};


void printHelptoSerial (void){
  Serial.println(F("|----------------Supported command list:-------------------------------|"));
  Serial.println(F("| 1 | start | Start generating, trigger run button via uart            |"));
  Serial.println(F("| * | type: start  - 'start immideatly'                                |"));
  Serial.println(F("| * | type: start <delay_value=0-5000ms>; example: start 100           |"));
  Serial.println(F("| E:| example: start 100                                               |"));
  Serial.println(F("|----------------------------------------------------------------------|"));
  Serial.println(F("| 2 | set   | Setup pulse generator parameters ON time, Off time, Nrep |"));
  Serial.println(F("| * | type: set -ontime <val us> -offtime <val us> -nrep <val>         |"));
  Serial.println(F("| * | On/Off time value in range 100 us - 6500 us, Nrep in range 1-500 |"));
  Serial.println(F("| E:| example: set -ontime 250 -offtime 300 -nrep 5                    |"));
  Serial.println(F("|----------------------------------------------------------------------|"));
  Serial.println(F("| 3 | set ON time | Set the ON pulse duration time, in us              |"));
  Serial.println(F("| * | type: setontime <val us>; example: setontime 250 [100-6500 us]   |"));
  Serial.println(F("| E:| example: setontime 250 [On time value in range 100 us - 6500 us] |"));
  Serial.println(F("|----------------------------------------------------------------------|"));
  Serial.println(F("| 4 | set OFF time | Set the OFF pulse duration time, in us            |"));
  Serial.println(F("| * | type: setofftime <val us>; example: setofftime 500 [100-6500 us] |"));
  Serial.println(F("| E:| example: setofftime 500[OFF time value in range 100 us - 6500 us]|"));
  Serial.println(F("|----------------------------------------------------------------------|"));
  Serial.println(F("| 5 | set Nrep | Set nubmer of pulses to be geenerated on a signle run |"));
  Serial.println(F("| * | type: setnrep <val times>; example: setnrep 20  [1-500]          |"));
  Serial.println(F("| E:| example: setnrep 20 [Nrep value in range 1 - 500  ]              |"));
  Serial.println(F("|----------------------------------------------------------------------|"));
  Serial.println(F("| 6 | status|Returns current generator setting and load voltage        |"));
  Serial.println(F("| E:| type: getstatus                                                  |"));
  Serial.println(F("|----------------------------------------------------------------------|"));
  Serial.println(F("| 7 | Vload |Returns measured voltage on load connector                |"));
  Serial.println(F("| E:| type: getvload                                                   |"));
  Serial.println(F("|----------------------------------------------------------------------|"));
  Serial.println(F("| 8 | help  | Display this massage                                     |"));
  Serial.println(F("| E:| type: help                                                       |"));
  Serial.println(F("|----------------------------------------------------------------------|"));
};
 



                             
