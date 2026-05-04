#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <math.h>

// ---------------- CONSTANTS ----------------
float wheel_diam = 2.57874016;
float rpm = 45;
float shaft_distance = 4.72440945;

// ---------------- GLOBALS ----------------
volatile uint16_t lastTime = 0;
volatile uint16_t period = 0;

uint16_t startPeriod = 0;

int black_count = 0;
int color_count = 0;

uint8_t started = 0;

// ---------------- ISR ----------------
ISR(PCINT1_vect)
{
    if (PINC & 0b00000100)
    {
        uint16_t now = TCNT1;
        period = now - lastTime;
        lastTime = now;
    }
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

// ---------------- MOTOR CONTROL ----------------
void stop_motors(){
    PORTB &= ~(0b00001001);
    PORTD &= ~(0b00001100);
}

void drive_forward(float distance){
    PORTB = (PORTB & 0b11110111) | 0b00001000;
    PORTD = (PORTD & 0b11110011) | 0b00000100;
    delay_ms_safe(time_delay_func(distance));
    stop_motors();
}

void drive_backwards(float distance){
    PORTB = (PORTB & 0b11111110) | 0b00000001;
    PORTD = (PORTD & 0b11110011) | 0b00001000;
    delay_ms_safe(time_delay_func(distance));
    stop_motors();
}

void drive_right(float degrees){
    PORTB = (PORTB & 0b11110110) | 0b00000001;
    PORTD = (PORTD & 0b11110011) | 0b00000100;
    delay_ms_safe(time_delay_func_ang(degrees));
    stop_motors();
}

void drive_left(float degrees){
    PORTB = (PORTB & 0b11110111) | 0b00001000;
    PORTD = (PORTD & 0b11110011) | 0b00001000;
    delay_ms_safe(time_delay_func_ang(degrees));
    stop_motors();
}

// ---------------- INIT ----------------
void setup()
{
    PORTB &= ~(0b00001001);
    PORTD &= ~(0b00001100);

    DDRB |= 0b00001001;
    DDRD |= 0b00001100;

    // BUTTON PB2
    DDRB &= ~(0b00000100);
    PORTB |= 0b00000100;

    DDRC &= ~(0b00000111);

    PCICR |= 0b00000010;
    PCMSK1 |= 0b00000100;

    TCCR1A = 0;
    TCCR1B = 0;

    sei();
}

// ---------------- LOOP ----------------
void loop()
{
    // -------- WAIT FOR BUTTON --------
    if (!started)
    {
        // wait for release
        while (!(PINB & 0b00000100)) { _delay_ms(5); }

        // wait for press
        while (PINB & 0b00000100) { _delay_ms(5); }

        _delay_ms(50);

        if (!(PINB & 0b00000100))
        {
            // -------- START SYSTEM --------
            started = 1;

            cli();
            TCNT1 = 0;
            lastTime = 0;
            period = 0;
            sei();

            TCCR1B = 0b00000001; // start timer

            // -------- CAPTURE BASELINE HERE --------
            uint32_t sum = 0;

            for (uint8_t i = 0; i < 10; i++)
            {
                _delay_ms(20);

                cli();
                sum += period;
                sei();
            }

            startPeriod = sum / 10; // averaged baseline
        }

        return;
    }

    uint16_t p;

    cli();
    p = period;
    sei();

    if (black_count == 0 && color_count == 0){
        drive_forward(1.2);
    }

    // -------- COLOR CHANGE --------
    if (p > 0 && abs(p - startPeriod) > 400 && abs(p - startPeriod) < 1000 && color_count < 1)
    {
        drive_left(90);
        drive_forward(1.85/2);
        color_count++;
    }

    // -------- FIRST BLACK --------
    if (p > 0 && abs(p - startPeriod) > 2000 && black_count < 1)
    {
        drive_left(180);
        drive_forward(1.85);
        black_count++;
    }

    // -------- SECOND BLACK --------
    if (p > 0 && abs(p - startPeriod) > 2000 && black_count == 1)
    {
        drive_right(135);
        drive_forward(2.657);
        stop_motors();
        while(1);
    }

    _delay_ms(100);
}