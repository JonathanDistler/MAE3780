#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

// ---------------- CONSTANTS ----------------
float wheel_diam = 2.57874016;        // inches
float rpm = 53.6912751678;            // motor RPM
float shaft_distance = 4.72440945;    // inches between wheels


// ---------------- PROTOTYPES ----------------
float time_delay_func(float distance);
float time_delay_func_ang(float angle_deg);

void drive_forward(float distance);
void drive_backward(float distance);
void drive_right(float degrees);
void drive_left(float degrees);
void stop_motors();

// ---------------- MAIN ----------------
int main(void){

  // LEFT MOTOR (PB0, PB1)
  DDRB |= (1 << 0) | (1 << 1);

  // RIGHT MOTOR (PD2, PD3)
  DDRD |= (1 << 2) | (1 << 3);

  // BUTTON PB2
  DDRB &= ~(1 << 2);
  PORTB |= (1 << 2);   // pull-up

  stop_motors();

  uint8_t prev = 1;

  while(1){

    uint8_t curr = (PINB & (1 << 2)) ? 1 : 0;

    if (prev == 1 && curr == 0) {

      drive_forward(0.3048); //drive forward 1 ft 

      drive_right(90); //turn right 90

      drive_forward(0.3048); //drive forward 1 ft 

      drive_left(90); //turn left 90 

      drive_forward(0.3048/2); //drive forward .5 ft 

      drive_backward(0.4572); //drive backwards 1.5 ft 

      drive_left(90); //turn left 90 

      drive_forward(0.3048); //drive forward 1 ft 

      stop_motors();
    }

    prev = curr;
  }
}

void drive_forward(float distance){

  // LEFT forward
  PORTB = (PORTB & 0b11111100) | 0b00000010;

  // RIGHT forward
  PORTD = (PORTD & 0b11110011) | 0b00001000;

  int t = time_delay_func(distance);

  _delay_ms(t);
}

void drive_backward(float distance){

  // LEFT reverse
  PORTB = (PORTB & 0b11111100) | 0b00000001;

  // RIGHT reverse
  PORTD = (PORTD & 0b11110011) | 0b00000100;

  int t = time_delay_func(distance);

  _delay_ms(t);
}

void drive_right(float degrees){

  // LEFT forward
  PORTB = (PORTB & 0b11111100) | 0b00000010;

  // RIGHT reverse
  PORTD = (PORTD & 0b11110011) | 0b00000100;

  int t = time_delay_func_ang(degrees);

  _delay_ms(t);
}

void drive_left(float degrees){

  // LEFT reverse
  PORTB = (PORTB & 0b11111100) | 0b00000001;

  // RIGHT forward
  PORTD = (PORTD & 0b11110011) | 0b00001000;

  int t = time_delay_func_ang(degrees);

  _delay_ms(t);
}

void stop_motors(){

  PORTB &= ~((1 << 0) | (1 << 1));
  PORTD &= ~((1 << 2) | (1 << 3));
}

float time_delay_func(float distance){

  float wheel_circ = 3.1415 * wheel_diam;

  float v = wheel_circ * (rpm / 60.0);   // inches/sec

  float t = distance / v;                // seconds

  return t * 1000.0;                     // ms
}

float time_delay_func_ang(float angle_deg){

  float angle_rad = angle_deg * 3.1415 / 180.0;

  float wheel_circ = PI * wheel_diam;

  float v = wheel_circ * (rpm / 60.0);

  float omega = (2.0 * v) / shaft_distance;

  float t = angle_rad / omega;

  return t * 1000.0;
}