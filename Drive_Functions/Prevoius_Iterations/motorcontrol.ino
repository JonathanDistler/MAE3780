/*

INSTRUCTIONS

One of the motors is already wired for you. It is controlled
by Arduino pins 8 and 9.

1. Wire the other side of the H-bridge to the other motor
   and to Arduino pins of your choice.
2. Write four functions to make your "robot" drive forward
   (both motors have positive RPM), drive backward (both 
   motors have negative RPM), turn left (right motor positive,
   left motor negative), and turn right (opposite).
3. Call the functions in the order you would need for milestone 1.
   You can adjust the delay timing when testing your physical robot.
4. Think about careful use of masking - your robot will have
   more stuff connected to the other pins.
5. As usual, no Arduino commands like pinMode - do everything
   with registers. You can use or adapt this code for
   milestone 1 with your robot.
6. Submit a screenshot of your circuit and a file for your code
   on Gradescope. 
   
   Both hi stops it, both lo stops it

*/

int main(void){ // setup code that only runs once
  // set pins 8 and 9 as outputs for "left" motor
  DDRB = 0b00000011;
  DDRD=0b00001100; //
  while(1){ // code that loops forever
    // call functions for different directions
    
    drive_forward();
    _delay_ms(10000);
    
    drive_right();
    _delay_ms(10000);
    
    drive_backward();
    _delay_ms(10000);
    
    drive_left();
    _delay_ms(10000);
    
    // option 1: function with no argument followed by delay
    // drive_forward();
    // _delay_ms(1000);
    
    // option 2: function with argument for delay
    // drive_forward_delay(1000);
  }
}

// option 1: function with no delay
void drive_forward(){
  // Left motor: forward
  //PORTB = 0b00000001;  // PB0 HIGH, PB1 LOW
  //PORTB=(PORTB &0b11111100) | 0b00000001; 
  PORTB = (PORTB & 0b11111100) | 0b00000010;
  
  // Right motor: forward
  //PORTD = 0b00000100;  // PD2 HIGH, PD3 LOW
  PORTD=(PORTD&0b11110011) | 0b00001000;
}

// option 2: function with built-in delay
void drive_forward_delay(int t){ 
   // code to control appropriate pins here:
  //PORTD=0b00001100;
  PORTD=(PORTD&0b11110011) | 0b00000110;
  //PORTB=0b00000011; 
  PORTB = (PORTB & 0b11111100) | 0b00000011;
  
  _delay_ms(t);         // wait for t milliseconds
}

void drive_right(){
  // Left motor: forward
  //PORTB = 0b00000001;
  PORTB=(PORTB & 0b11111100) | 0b00000010;
  
  // Right motor: reverse
  //PORTD = 0b00001000;
  PORTD=(PORTD&0b11110011) | 0b00000100;
}

void drive_left(){
  // Left motor: reverse
  //PORTB = 0b00000010;
  PORTB=(PORTB & 0b11111100) |0b00000001;
  
  // Right motor: forward
  //PORTD = 0b00000100;
  PORTD=(PORTD&0b11110011) | 0b00001000;
}

void drive_backward(){
  // Left motor: reverse
  //PORTB = 0b00000010;  // PB1 HIGH, PB0 LOW
  PORTB=(PORTB & 0b11111100) | 0b00000001;
  
  // Right motor: reverse
  //PORTD = 0b00001000;  // PD3 HIGH, PD2 LOW
  PORTD=(PORTD&0b11110011) | 0b00000100;
}