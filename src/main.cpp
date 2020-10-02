#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>

#include "bitmaps.h"

#define MIC_PIN       0
#define LED_PIN       2
#define BUTTON_PIN    4
#define THRESHOLD     600       // Mic input ADC value threshold (0-1023) to detect voice
#define NUM_READINGS  4         // Number of readings to take before averaging
#define ToT_DELAYON   500       // Time "Trick or Treat" message is on between flashes
#define ToT_DELAYOFF  200       // Time "Trick or Treat" message is off between flashes
#define ToT_FLASHES   4         // Number of time "Trick or Treat" flashes

#define DEBUG         false     // flag to turn on/off debugging
#define Serial if(DEBUG)Serial

// U8g2 constructor
U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);   

int val = 0;
int mic_val;
int readings[NUM_READINGS];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
int threshold = 700;

enum mouth_states { closed, open };
mouth_states mouth_state = closed, prev_mouth_state = closed;

void updateMouth(mouth_states state){
  u8g2.clearBuffer();
  switch (state){
    case closed:
      u8g2.drawXBMP(0, 0, closed_width, closed_height, closed_bits);
      u8g2.sendBuffer();
      break;
    case open:
      // Show transition (half-open) mouth first
      u8g2.drawXBMP(0, 0, half_width, half_height, half_bits);
      u8g2.sendBuffer();
      u8g2.drawXBMP(0, 0, open_width, open_height, open_bits);
      u8g2.sendBuffer();
      delay(random(1, 100));
      break;
  }
}

void showTrickorTreat(){
  for(int i = 0; i < ToT_FLASHES; i++)
  {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    delay(ToT_DELAYOFF);
    u8g2.drawXBMP(0, 0, TrickorTreat_width, TrickorTreat_height, TrickorTreat_bits);
    u8g2.sendBuffer();
    delay(ToT_DELAYON);
  }
}

void setup(void) {
  u8g2.begin();
  Serial.begin(115200); 

  pinMode(MIC_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < NUM_READINGS; thisReading++) {
    readings[thisReading] = 0;
  }

  mouth_state = closed;
  updateMouth(mouth_state);
  u8g2.setFont(u8g2_font_ncenB14_tr);
}

void loop(void) {
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(MIC_PIN);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;
  
  val = digitalRead(BUTTON_PIN);
  if ( val == LOW)
  { 
    //Serial.println("Displaying trick or treat");
    showTrickorTreat();
    mouth_state = closed;
    updateMouth(mouth_state);
  }

  if (readIndex >= NUM_READINGS) 
  {
    // calculate the average:
    average = total / NUM_READINGS;
    //Serial.println(average);
    mouth_state = (average < THRESHOLD) ? closed : open; 

    if (mouth_state != prev_mouth_state)
    { 
      digitalWrite(LED_PIN, mouth_state);
      updateMouth(mouth_state); 
    }
    prev_mouth_state = mouth_state;
    readIndex = 0;
  }
}

