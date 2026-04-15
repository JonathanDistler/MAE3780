#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

// ---------------- CONSTANTS ----------------
// (Using math.h PI safely — no redefinition)

float wheel_diam = 2.57874016;        // inches
float rpm = 53.6912751678;            // motor RPM
float shaft_distance = 4.72440945;    // inches between wheels


// ---------------- PROTOTYPES ----------------
float time_delay_func(float distance);
float time_delay_func_ang(float angle_deg);

void delay_ms_safe(float ms);

void drive_forward(float distance);
void drive_backward(float distance);
void drive_right(float degrees);
void drive_left(float degrees);
void stop_motors();


// ---------------- MAIN SEQUENCE ----------------
int main(void){

    // LEFT MOTOR (PB0, PB1)
    DDRB |= (1 << 0) | (1 << 1);

    // RIGHT MOTOR (PD2, PD3)
    DDRD |= (1 << 2) | (1 << 3);

    // BUTTON PB2
    DDRB &= ~(1 << 2);
    PORTB |= (1 << 2);   // pull-up enabled

    stop_motors();

    uint8_t prev = 1;


    while(1){

        uint8_t curr = (PINB & (1 << 2)) ? 1 : 0;

        // detect button press (HIGH -> LOW)
        if (prev == 1 && curr == 0) {

            _delay_ms(30); // debounce

            // re-check after debounce
            if (!(PINB & (1 << 2))) {

                // ---------------- MOTION SEQUENCE ----------------

                drive_forward(12.0);     // 1 foot

                drive_right(90.0);

                drive_forward(12.0);     // 1 foot

                drive_left(90.0);

                drive_forward(6.0);      // 6 inches

                drive_backward(18.0);    // 1.5 feet

                drive_left(90.0);

                drive_forward(12.0);     // return to start

                stop_motors();
            }
        }

        prev = curr;
    }
}


// ---------------- MOTOR CONTROL ----------------

void drive_forward(float distance){

    PORTB = (PORTB & 0b11111100) | 0b00000010;
    PORTD = (PORTD & 0b11110011) | 0b00001000;

    delay_ms_safe(time_delay_func(distance));
}

void drive_backward(float distance){

    PORTB = (PORTB & 0b11111100) | 0b00000001;
    PORTD = (PORTD & 0b11110011) | 0b00000100;

    delay_ms_safe(time_delay_func(distance));
}

void drive_right(float degrees){

    PORTB = (PORTB & 0b11111100) | 0b00000010;
    PORTD = (PORTD & 0b11110011) | 0b00000100;

    delay_ms_safe(time_delay_func_ang(degrees));
}

void drive_left(float degrees){

    PORTB = (PORTB & 0b11111100) | 0b00000001;
    PORTD = (PORTD & 0b11110011) | 0b00001000;

    delay_ms_safe(time_delay_func_ang(degrees));
}

void stop_motors(){

    PORTB &= ~((1 << 0) | (1 << 1));
    PORTD &= ~((1 << 2) | (1 << 3));
}


// ---------------- TIMING MODEL ----------------

float time_delay_func(float distance){

    float wheel_circ = 3.14159265f * wheel_diam;

    float v = wheel_circ * (rpm / 60.0f);   // inches/sec

    float t = distance / v;                // seconds

    return t * 1000.0f;                    // ms
}

float time_delay_func_ang(float angle_deg){

    float angle_rad = angle_deg * 3.14159265f / 180.0f;

    float wheel_circ = 3.14159265f * wheel_diam;

    float v = wheel_circ * (rpm / 60.0f);

    float omega = (2.0f * v) / shaft_distance;

    float t = angle_rad / omega;

    return t * 1000.0f;                    // ms
}


// ---------------- SAFE DELAY ----------------

void delay_ms_safe(float ms){

    while (ms >= 1.0f) {
        _delay_ms(1);
        ms -= 1.0f;
    }

    if (ms > 0.0f) {
        _delay_ms(1);
    }
}