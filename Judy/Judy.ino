/* 
  Description:
  Judy Seto's Lighting Project
  Electrical Components for 2 modules:
    2x Arduino Uno
    2x Short Range RF24 Module
    2x Shift Registers
    14x tactile switches
    14x yellow 5mm LED's
    14x 220 Ohm Resistors
    14x 10k Ohm Resistors

  Wiring:
    see fritzing diagram
  
  Specifications of the code:
    each module has 7 switches and 7 LED's the LEDs will be synced with each other.
    press the switch once to turn LED on, press it again to turn it off, the LED
    will correspond to each others.
  
  Author:
  Christopher Chun-Hung Ho
  Winnie
  
  History:
  v1.1  25/05/15  addition of Shift register code
  v1.0  15/05/15  Write out radio communication
*/

//-----------------------------------------------------------------------------------------------------
//Include
//-----------------------------------------------------------------------------------------------------
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

//-----------------------------------------------------------------------------------------------------
//Const pins
//-----------------------------------------------------------------------------------------------------
//Shift Register pins
const int latchPin = 5;
const int clockPin = 6;
const int dataPin = 4;

//-----------------------------------------------------------------------------------------------------
//Declaring
//-----------------------------------------------------------------------------------------------------
//variables
boolean ledState[7];
char charStates[7];
int buttons[7] = {2, A0, A1, A2, A3, A4, A5};
byte leds = 0;

//modules
RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//-----------------------------------------------------------------------------------------------------
// main
//-----------------------------------------------------------------------------------------------------
void setup() {
  //set up output pins
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  
  //turn all LEDs off
  for(int i = 0; i<7; i++){
    pinMode(buttons[i], INPUT);
    ledState[i] = LOW;
  }
  
  radio.begin();
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.setChannel(65);
  radio.startListening();
}

void loop() {
  radioReceive();                             //check for received data
  
  bool changed = false;                       //set up check for change
  
  for(int i = 0; i<7; i++){
    if(digitalRead(buttons[i])==HIGH){        //check if button has been pressed
      while(digitalRead(buttons[i])==HIGH){   //wait until button released
      }
      changed = true;                         //acknowledge change has occured
      radioReceive();                         //read other side state before changing ours
      ledState[i] = !ledState[i];             //change the led's state
      
    }
  }
  
  if ( changed ) radioSend();      //send to other module to sync changes
  leds = 0;
  for(int i = 0; i<7;i++){        //set up the byte to store the data on the bits to turn on
    if(ledState[i]){
      bitSet(leds, i);
    }
  }
  updateShiftRegister();          //output to pins
}

void radioSend(){
  for(int i = 0; i<7;i++){                                  //convert data to char for sending purposes
    if(ledState[i]){
      charStates[i]='1';
    }else{
      charStates[i]='0';
    }
  }
  
  radio.stopListening();                                   //write out data
  radio.write(charStates, sizeof(charStates));
  radio.startListening();
  delay(5);
}

void radioReceive(){
  if(radio.available()){                                       //check if radio is available
    while(!radio.read(charStates,7)){}                         // Read it into the receiving buffer
    for(int i=0;i<7;i++){                                      // and print it out to the serial monitor
      if(charStates[i]=='1'){                                  //convert back to bool format
        ledState[i]=HIGH;
      }else{
        ledState[i]=LOW;
      }
    }
    updateShiftRegister();                                     //update data
  }
}

void updateShiftRegister(){
   digitalWrite(latchPin, LOW);                                //output pin data to shift register
   shiftOut(dataPin, clockPin, LSBFIRST, leds);
   digitalWrite(latchPin, HIGH);
}
