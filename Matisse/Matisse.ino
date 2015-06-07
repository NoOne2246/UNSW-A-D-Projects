/* 
  Description:
  Matisse's Lighting Project
  Electrical Components for 2 modules:
    2x Arduino Uno
    2x Short Range RF24 Module
    2x DS1302RTC Real time Clock Modules
    2x Light Dependent Resistor
    4x 220 Ohm Resistor
    2x 10k Ohm Resistor
    Enough yellow and Blue LEDs

  Wiring:
    see fritzing diagram
  
  Specifications of the code:
    Using the light sensor, detect whether an object has been placed on top of the sensor,
    if there is, then light up the other module. the colour lit up will be yellow if between
    6am and 6pm, while it will be blue between 6pm and 6am. if there is no object, then turn
    off the lights.
  
  Author:
  Christopher Chun-Hung Ho
  z5019205
  
  History:
  v1.1  05/06/15  Write out radio communication
  v1.0  20/05/15  Write out RTC controlled light
*/

//-----------------------------------------------------------------------------------------------------
//Include
//-----------------------------------------------------------------------------------------------------

//Set up radio library
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

//set up rtc clock library
#include <Time.h>  
#include <Wire.h>  
#include <DS1302RTC.h>
//-----------------------------------------------------------------------------------------------------
//Define
//-----------------------------------------------------------------------------------------------------
#define DIFFERENCE 600 //threshold for when it goes from dark to light


//-----------------------------------------------------------------------------------------------------
//Const pins
//-----------------------------------------------------------------------------------------------------
//LED lights
const int yellowLED = 2;
const int blueLED = 3;


//-----------------------------------------------------------------------------------------------------
//Declaring
//-----------------------------------------------------------------------------------------------------
//Variables
boolean blocked = false; //whether the previous state of the LDR is blocked or not.
boolean light = LOW;     //whether the light needs to be on

int value;  //value read by ldr

char data; //information being sent

//Modules
//short range rf module
RF24 radio(9,10);        //9 to CE, 10 to CSN
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//RTC module
DS1302RTC RTC(5, 6, 7);  //5 to CE, 6 to IO, 7 to Clk

//Functions

//-----------------------------------------------------------------------------------------------------
// main
//-----------------------------------------------------------------------------------------------------


void setup() {
  //Set up Serial Comm
  Serial.begin(115200);
  
  //Set up radio
  radio.begin();
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.setChannel(111);    //make sure everyone is using a different number
  radio.startListening();
  
  //Set up RTC sync
  setSyncProvider(RTC.get);
  if(timeStatus()!=timeSet){
    Serial.println("Unable to sync with the RTC");
  }else{
    Serial.println("RTC has set the system time");
  }
  
  //Set up output pins
  pinMode(yellowLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
}

void loop() {
  radioReceive();                          //check whether data has been received and change light state accordingly
  
  bool changed = false;                    //set check for whether a change is made
  
  value = analogRead(A0);                  //read LDR value through analogRead
  
  if(value < DIFFERENCE && blocked==false){  //if it is covered and was previously not covered
    changed = true;
    data = 'H';
    blocked = true;
  }else if(value > DIFFERENCE && blocked == true){  //if it is not coverent and was previously covered
    changed = true;
    data = 'L';
    blocked = false;
  }
  
  if ( changed ) radioSend();            //if any change of state was made, send signal to other side
}



//-----------------------------------------------------------------------------------------------------
// subroutine
//-----------------------------------------------------------------------------------------------------
void radioSend(){
  //write to radio
  radio.stopListening(); 
  radio.write(&data, sizeof(char));
  radio.startListening();
  delay(5);
}

void radioReceive(){
  if(radio.available()){ 
    while(!radio.read(&data,1)){} // Read it into the receiving buffer
    if(data=='H'){          //if data received is a high signal, then give signal for the light to turn on
      light = HIGH;
    }else {
      light = LOW;
    }
  }
  if(light){                                  //if the light is to turn on, then check time and make decision
    if(hour()>6 && hour()<18){
      digitalWrite(yellowLED, HIGH);          //for day time, turn yellow on and make sure night time off
      digitalWrite(blueLED, LOW);
    }else{
      digitalWrite(blueLED, HIGH);            //for night time, turn blue on and yellow off
      digitalWrite(yellowLED, LOW);
    }
  }else{
    digitalWrite(blueLED, LOW);               //else turn off everything
    digitalWrite(yellowLED, LOW);
  }
}

