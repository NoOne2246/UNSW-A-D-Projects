/* 
  Description:
  Matisse's Lighting Project
  
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
boolean blocked = false;
boolean light = LOW;

int value;  //value read by ldr


char data; //information being sent

//Modules
//short range rf module
RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//RTC module
DS1302RTC RTC(5, 6, 7);

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
  radioReceive();
  
  bool changed = false;
  
  //read value and check state
  value = analogRead(A0);
  
  if(value < DIFFERENCE && blocked==false){
    changed = true;
    data = 'H';
    blocked = true;
  }else if(value > DIFFERENCE && blocked == true){
    changed = true;
    data = 'L';
    blocked = false;
  }
  
  
  if ( changed ) radioSend();
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
    if(data=='H'){
      light = HIGH;
    }else {
      light = LOW;
    }
  }
  if(light){
    if(hour()>6 && hour()<18){
      digitalWrite(yellowLED, HIGH);
      digitalWrite(blueLED, LOW);
    }else{
      digitalWrite(blueLED, HIGH);
      digitalWrite(yellowLED, LOW);
    }
  }else{
    digitalWrite(blueLED, LOW);
    digitalWrite(yellowLED, LOW);
  }
}

