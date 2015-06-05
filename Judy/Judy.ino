#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

int latchPin = 5;
int clockPin = 6;
int dataPin = 4;

boolean ledState[7];
char charStates[7];
int buttons[7] = {2, A0, A1, A2, A3, A4, A5};
//int light[7] = {2, 3, 4, 5, 6, 7, 8};
byte leds = 0;

RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

void setup() {
  // put your setup code here, to run once:
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  
  for(int i = 0; i<7; i++){
    pinMode(buttons[i], INPUT);
    //pinMode(light[i], OUTPUT);
    ledState[i] = LOW;
  }
  //Serial.begin(9600);
  radio.begin();
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.setChannel(65);
  radio.startListening();
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("test");
  radioReceive();
  bool changed = false;
  for(int i = 0; i<7; i++){
    if(digitalRead(buttons[i])==HIGH){
      //Serial.println("here");
      while(digitalRead(buttons[i])==HIGH){
      }
      changed = true;
      ledState[i] = !ledState[i];
      
    }
  }
  
  if ( changed ) radioSend();
  leds = 0;
  for(int i = 0; i<7;i++){
    if(ledState[i]){
      bitSet(leds, i);
    }
    //digitalWrite(light[i], ledState[i]);
  }
  updateShiftRegister();
}

void radioSend(){
  for(int i = 0; i<7;i++){
    if(ledState[i]){
      charStates[i]='1';
    }else{
      charStates[i]='0';
    }
  }
  
  radio.stopListening(); 
  radio.write(charStates, sizeof(charStates));
  radio.startListening();
}

void radioReceive(){
  if(radio.available()){          
    while(!radio.read(charStates,7)){} // Read it into the receiving buffer
    for(int i=0;i<7;i++){  // and print it out to the serial monitor
      if(charStates[i]=='1'){
        ledState[i]=HIGH;
      }else{
        ledState[i]=LOW;
      }
    }
  }
  
}

void updateShiftRegister()
{
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, leds);
   digitalWrite(latchPin, HIGH);
}
