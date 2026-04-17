#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

//like all similar scripts, but a pin fell off in the board so I had to change the digital logic 

// ---------------- CONSTANTS ----------------
float wheel_diam = 2.57874016;
float rpm = 45;
float shaft_distance = 4.72440945;

// ---------------- PROTOTYPES ----------------
float time_delay_func(float distance);
float time_delay_func_ang(float angle_deg);

void delay_ms_safe(float ms);

void drive_forward(float distance);
void drive_backward(float distance);
void drive_right(float degrees);
void drive_left(float degrees);
void stop_motors();

// ---------------- MAIN ----------------
int main(void){

    // LEFT MOTOR NOW PB0 + PB3
    DDRB |= (1 << 0) | (1 << 3);

    // RIGHT MOTOR (UNCHANGED PD2, PD3)
    DDRD |= (1 << 2) | (1 << 3);

    // BUTTON PB2
    DDRB &= ~(1 << 2);
    PORTB |= (1 << 2);

    stop_motors();

    uint8_t prev = 1;

    while(1){

        uint8_t curr = (PINB & (1 << 2)) ? 1 : 0;

        if (prev == 1 && curr == 0) {

            _delay_ms(30);

            if (!(PINB & (1 << 2))) {

                drive_forward(15.0);
                drive_right(90);
                drive_forward(13.0);
                drive_left(100);
                drive_forward(7.0);
                drive_backward(18.0);
                drive_left(110);
                drive_forward(12.0);
            }
        }

        prev = curr;
    }
}

// ---------------- MOTOR CONTROL ----------------

void drive_forward(float distance){

    // FLIPPED: was backward → now true forward
    PORTB = (PORTB & 0b11110111) | (1 << 3);   // LEFT backward pin actually drives forward
    PORTD = (PORTD & 0b11110011) | (1 << 2);   // RIGHT backward pin actually drives forward

    delay_ms_safe(time_delay_func(distance));
    stop_motors();
}

void drive_backward(float distance){

    // FLIPPED: was forward → now true backward
    PORTB = (PORTB & 0b11111110) | (1 << 0);   // LEFT forward pin
    PORTD = (PORTD & 0b11110011) | (1 << 3);   // RIGHT forward pin

    delay_ms_safe(time_delay_func(distance));
    stop_motors();
}

void drive_right(float degrees){

    // LEFT forward, RIGHT backward
    PORTB = (PORTB & 0b11110110) | (1 << 0);
    PORTD = (PORTD & 0b11110011) | (1 << 2);

    delay_ms_safe(time_delay_func_ang(degrees));
    stop_motors();
}

void drive_left(float degrees){

    // LEFT backward, RIGHT forward
    PORTB = (PORTB & 0b11110111) | (1 << 3);
    PORTD = (PORTD & 0b11110011) | (1 << 3);

    delay_ms_safe(time_delay_func_ang(degrees));
    stop_motors();
}

void stop_motors(){

    // clear PB0 and PB3 (LEFT)
    PORTB &= ~((1 << 0) | (1 << 3));

    // clear PD2 and PD3 (RIGHT)
    PORTD &= ~((1 << 2) | (1 << 3));
}

// ---------------- TIMING ----------------

float time_delay_func(float distance){

    float wheel_circ = 3.14159265f * wheel_diam;
    float v = wheel_circ * (rpm / 60.0f);
    float t = distance / v;

    return t * 1000.0f;
}

float time_delay_func_ang(float angle_deg){

    float angle_rad = angle_deg * 3.14159265f / 180.0f;
    float wheel_circ = 3.14159265f * wheel_diam;

    float v = wheel_circ * (rpm / 60.0f);
    float omega = (2.0f * v) / shaft_distance;

    float t = angle_rad / omega;

    return t * 1000.0f;
}

// ---------------- DELAY ----------------

void delay_ms_safe(float ms){

    while (ms >= 1.0f) {
        _delay_ms(1);
        ms -= 1.0f;
    }

    if (ms > 0.0f) {
        _delay_ms(1);
    }
}