
/*Code for Wifi three triac 2 amps board
The Board has threewo Triacs two are on/off capable and one is dimmable 

This code is for Atmega328p 
Firmware Version: 0.1
Hardware Version: 0.1

Code Edited By :Naren N Nayak
Date: 05/01/2018
Last Edited By:
Date: 

*/ 


#include <TimerOne.h>

//Relay no.
#define NON_DIMMABLE_TRIAC_ONE 8 //Gpio 8 
#define NON_DIMMABLE_TRIAC_TWO 9 //Gpio 9            
#define DIMMABLE_TRIAC 10 //Gpio 10



//manual switch
#define SWITCH_INPIN1 A0 //switch 1 
#define SWITCH_INPIN2 A1 //switch 2
#define SWITCH_INPIN3 A2 //switch 3  //
/* ZCD */

#define ZCD_INT 0  //Arduino GPIO2 
#define Dimmer_width 115

/*Serial Data variables*/
String serialReceived;
String serialReceived1;
String serialReceived2;
String Dimmer_value_temp;
String Dimmer_value;
String regulator_value_temp;
/*POT Variable */
String regulator_value;

/*ZCD Variables */
int freqStep = 75;//75*5 as prescalar is 16 for 80MHZ
volatile int dim_value = 0;
int dimming = 115;
volatile boolean zero_cross = 0;
volatile int int_regulator=0;
volatile int int_regulator_temp;
volatile int i=1;
/*Flags for Dimmer virtual switch concept */
volatile boolean dimmer_value_changed =false; 
volatile boolean regulator_value_changed =false;


void setup() {

  Serial.begin(115200);
  Serial.println("WiFi-2A-Dimmer");
  pinMode(NON_DIMMABLE_TRIAC_ONE, OUTPUT); //relay1 output
  pinMode(NON_DIMMABLE_TRIAC_TWO, OUTPUT); //relay1 output
  pinMode(DIMMABLE_TRIAC, OUTPUT); //Dimmer output
  
 

  pinMode(SWITCH_INPIN1, INPUT); //manual switch 1 input
  pinMode(SWITCH_INPIN2, INPUT); //manual switch 1 input
  

  attachInterrupt(ZCD_INT, zero_cross_detect, CHANGE);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(dim_check, freqStep);
  
  
}

/*ZCD Interrupt Function*/
void zero_cross_detect() 
{
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  dim_value = 0;
  digitalWrite(DIMMABLE_TRIAC, LOW);      // turn off TRIAC (and AC)

}

/*Timer Interrupt Function used to trigger the triac for Dimming*/
void dim_check() 
{
  /*For Dimmer */
  if (zero_cross == true) 
  {
    if (dim_value >= dimming) 
    {
      digitalWrite(DIMMABLE_TRIAC, HIGH); // turn on Triac 
      dim_value = 0; // reset time step counter
      zero_cross = false; //reset zero cross detection
    }
    else 
    {
      dim_value++; // increment time step counter
    }
  }
}


void loop() 
{

/* Multi by 2 so that 2.5V level gets to 5V 10K 10K divider 
   Div by 11 so that value doesnt exceed 99 asdimmer range is 0-99
   Div and mul by 10 gives better variation in Pot value */
     
   
   int_regulator_temp= ((round(((((analogRead(SWITCH_INPIN3))*2))/11)/10))*10); 
   int_regulator+=int_regulator_temp;
   i++;
   
   if(i>=100)
   {
    int_regulator=(round((int_regulator/100)*10)/10);
    regulator_value_temp="Dimmer:"+String((int_regulator));
    i=0;
    //Serial.println("Regulator value to "+regulator_value);
   }
   
/*############### Flag setting for Dimmable Triac through Pot ###############*/
  
  if(regulator_value_temp!=regulator_value)
  {
    regulator_value=regulator_value_temp;
    regulator_value_changed =true;
  }
  else
  {
    regulator_value_changed =false;
  }


/*############### Uart Data ###########################*/
  
  if (Serial.available() > 0) 
  {   // is a character available
    
    serialReceived = Serial.readStringUntil('\n');
    Serial.println(serialReceived);
    
    if (serialReceived.substring(0, 7) == "Dimmer:")
    {
      Dimmer_value_temp = serialReceived;     
    }

    if (serialReceived.substring(0, 33) == "R_2 switched via web request to 1")
    {
      serialReceived2 = serialReceived;
    }
    
    if (serialReceived.substring(0, 33) == "R_2 switched via web request to 0")
    {
      serialReceived2 = serialReceived;
    }

    if (serialReceived.substring(0, 33) == "R_1 switched via web request to 1")
    {
      serialReceived1 = serialReceived;
    }
    
    if (serialReceived.substring(0, 33) == "R_1 switched via web request to 0")
    {
      serialReceived1 = serialReceived;
    }

 

  }

/*################## Flag setting for Dimmable Triac through uart ##################################*/

  if( Dimmer_value_temp!=Dimmer_value)
  {
    Dimmer_value=Dimmer_value_temp;
    dimmer_value_changed =true;
  }
  else 
  {
    dimmer_value_changed =false;
  }


/*##################### Non Dimmable Triac Two ##############################*/
  
  if (((serialReceived2.substring(0, 33) == "R_2 switched via web request to 1") && (!(digitalRead(SWITCH_INPIN2)))) || ((!(serialReceived2.substring(0, 33) == "R_2 switched via web request to 1")) && ((digitalRead(SWITCH_INPIN2))))) //exor logic
  {
    if(digitalRead(NON_DIMMABLE_TRIAC_TWO)==HIGH)
    {
      Serial.println("Load two is OFF");
    }
    digitalWrite(NON_DIMMABLE_TRIAC_TWO, LOW);   
  }
  else
  {
    if(digitalRead(NON_DIMMABLE_TRIAC_TWO)==LOW)
    {
      Serial.println("Load two is ON");
    }
    digitalWrite(NON_DIMMABLE_TRIAC_TWO, HIGH);
   
  }


/*##################### Non Dimmable Triac One ##############################*/
  
  if (((serialReceived1.substring(0, 33) == "R_1 switched via web request to 1") && (!(digitalRead(SWITCH_INPIN1)))) || ((!(serialReceived1.substring(0, 33) == "R_1 switched via web request to 1")) && ((digitalRead(SWITCH_INPIN1))))) //exor logic
  {
    if(digitalRead(NON_DIMMABLE_TRIAC_ONE)==HIGH)
    {
      Serial.println("Load one is OFF");
    }
    digitalWrite(NON_DIMMABLE_TRIAC_ONE, LOW);   
  }
  else
  {
    if(digitalRead(NON_DIMMABLE_TRIAC_ONE)==LOW)
    {
      Serial.println("Load one is ON");
    }
    digitalWrite(NON_DIMMABLE_TRIAC_ONE, HIGH);
   
  }


  
/*####################### Dimmable Triac ##################################*/
  
  if (Dimmer_value.substring(0, 7) == "Dimmer:" && dimmer_value_changed == true )
  {
    dimming = Dimmer_width - Dimmer_value.substring(7, 9).toInt();
    delay(5);
    Serial.println("Uart value to "+Dimmer_value);
  }

  if (regulator_value.substring(0, 7) == "Dimmer:" && regulator_value_changed == true )
  {
    dimming = Dimmer_width - regulator_value.substring(7, 9).toInt();
    delay(5);
    Serial.println("Regulator value to "+regulator_value);
  }
  
}


