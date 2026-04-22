#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

// Test script for sensing black color, only outputs message.
// ---------------- MAIN SEQUENCE ----------------
int main(void){
  // Maps PC0 and PC1 to inputs, which correspond to A0 and A1 on the Uno.
  DDRC=0b11111100; //all are outputs except last two bits
  int num_count=0; 

    while(1){
      if (PINC & 0b00000010 || PINC & 0b00000001){ //if either is hi (sees black)
        Serial.println("see black!");
        num_count++;
      }
      if (num_count>2){
        Serial.println("Too high"); 
        //this is where I would end getting out of turning loop 
      }
      _delay_ms(10); //to help clean things up 
    }
}


