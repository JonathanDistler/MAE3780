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

int16_t startPeriod = 0;

bool traverse = false;
uint16_t traverseCount = 0;

// ---------------- RUN STATE (NEW) ----------------
uint8_t runActive = 0;

// ---------------- ISR ----------------
ISR(PCINT1_vect)
{
    if (PINC & (1 << PC2))
    {
        int16_t now = TCNT1;
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

void delay_ms_safe(float ms){
    while (ms >= 1.0f) {
        _delay_ms(1);
        ms -= 1.0f;
    }
}

// ---------------- MOTOR CONTROL ----------------
void stop_motors(){
    PORTB &= ~((1 << 0) | (1 << 3));
    PORTD &= ~((1 << 2) | (1 << 3));
}

void drive_forward(float distance){
    PORTB = (PORTB & 0b11110111) | (1 << 3);
    PORTD = (PORTD & 0b11110011) | (1 << 2);

    delay_ms_safe(time_delay_func(distance));
    stop_motors();
}

void drive_left(float degrees){
    PORTB = (PORTB & 0b11110111) | (1 << 3);
    PORTD = (PORTD & 0b11110011) | (1 << 3);

    delay_ms_safe(time_delay_func_ang(degrees));
    stop_motors();
}

// ---------------- INIT ----------------
void setup()
{
    DDRB |= (1 << 0) | (1 << 3);
    DDRD |= (1 << 2) | (1 << 3);

    DDRC &= ~(1 << PC2);
    DDRB &= ~(1 << PB2);

    PORTB |= (1 << PB2); // pull-up

    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT10);

    TCCR1A = 0;
    TCCR1B = (1 << CS10);

    sei();

    _delay_ms(500);
    startPeriod = period;
}

// ---------------- LOOP ----------------
void loop()
{
    // ---------------- BUTTON START (TOGGLE RUN) ----------------
    if (!(PINB & (1 << PB2)))
    {
        _delay_ms(30);
        if (!(PINB & (1 << PB2)))
        {
            runActive = 1;

            while (!(PINB & (1 << PB2))); // wait release
            _delay_ms(30);
        }
    }

    // IF NOT RUNNING → DO NOTHING
    if (!runActive) return;

    int16_t p;

    cli();
    p = period;
    sei();

    // -------- COLOR CHANGE --------
    if (p > 0 && abs(p - startPeriod) > 350 && abs(p - startPeriod) < 800)
    {
        drive_forward(1);
        drive_left(75); //not truly 90, too large at teh moment 
        traverse = true;
        drive_forward(1);
    }

    if (p > 0 && abs(p - startPeriod) > 1200)
    {
        drive_left(180);
        drive_forward(1);

        if (traverse) traverseCount++;
    }

    if (traverseCount == 2)
    {
        drive_left(90);
        drive_forward(12.0);

        stop_motors();
        runActive = 0;   // STOP AFTER FINISH
    }

    drive_forward(1.0);
}
