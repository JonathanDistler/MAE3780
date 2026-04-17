//Goal of this is to turn only one motor, adding another motor would fit in easily with a new h-bridge with same amount of pins
//or a second h-bridge with twice as many pins

//same, same, opposite, opposite
int main(void){

  DDRB = 0b00000011;        // PB0, PB1 outputs
  DDRB &= ~(1 << 2);        // PB2 input
  PORTB |= (1 << 2);        // enable pull-up

  uint8_t prev = 1;         // previous state (not pressed)

  while(1){
    Serial.begin(9600);

    uint8_t curr = (PINB & (1 << 2)) ? 1 : 0;
    //Serial.print(curr);

    // detect press: HIGH → LOW
    if (prev == 1 && curr == 0) {
      Serial.print("should be running");

      drive_forward();
      _delay_ms(1000);

      drive_right();
      _delay_ms(1000);

      drive_backward();
      _delay_ms(1000);

      drive_left();
      _delay_ms(1000);
    }

    prev = curr;  // update state
  }
}

// option 1: function with no delay
void drive_forward(){
  // Left motor: forward
  //PORTB = 0b00000001;  // PB0 HIGH, PB1 LOW
  //PORTB=(PORTB &0b11111100) | 0b00000001; 
  PORTB = (PORTB & 0b11111100) | 0b00000010;
  
  // Right motor: forward
  //   PORTD=(PORTD&0b11110011) | 0b00001000;
}

// option 2: function with built-in delay
void drive_forward_delay(int t){ 
   // code to control appropriate pins here:
  // PORTD=(PORTD&0b11110011) | 0b00000110;
  //PORTB=0b00000011; 
  PORTB = (PORTB & 0b11111100) | 0b00000011;
  
  _delay_ms(t);         // wait for t milliseconds
}

void drive_right(){
  // Left motor: forward
  //PORTB = 0b00000001;
  PORTB=(PORTB & 0b11111100) | 0b00000010;
  
  // Right motor: reverse
  // PORTD=(PORTD&0b11110011) | 0b00000100;
}

void drive_left(){
  // Left motor: reverse
  //PORTB = 0b00000010;
  PORTB=(PORTB & 0b11111100) |0b00000001;
  
  // Right motor: forward
  // PORTD=(PORTD&0b11110011) | 0b00001000;
}

void drive_backward(){
  // Left motor: reverse
  //PORTB = 0b00000010;  // PB1 HIGH, PB0 LOW
  PORTB=(PORTB & 0b11111100) | 0b00000001;
  
  // Right motor: reverse
  // PD3 HIGH, PD2 LOW
  //PORTD=(PORTD&0b11110011) | 0b00000100;
}