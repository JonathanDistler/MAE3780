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

// ---------------- MOTOR CONTROL (YOUR FUNCTIONS) ----------------

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

void drive_right(float degrees){
    PORTB = (PORTB & 0b11110110) | (1 << 0);
    PORTD = (PORTD & 0b11110011) | (1 << 2);

    delay_ms_safe(time_delay_func_ang(degrees));
    stop_motors();
}

// ---------------- INIT ----------------

void setup()
{

    // Serial.begin(9600);
    DDRB |= (1 << 0) | (1 << 3);
    DDRD |= (1 << 2) | (1 << 3);

    DDRC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2));

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
    int16_t p;

    cli();
    p = period;
    sei();

    uint8_t pins = PINC;

    // -------- COLOR CHANGE --------

    // Blue to yellow or vice versa
    if (p > 0 && abs(p- startPeriod) > 350 && abs(p - startPeriod) < 800)
    {
        drive_left(90);           
        // drive_forward(8.0);  // use YOUR function
        // Serial.println("Blue");
        drive_forward(1.0); 
    }
    if (p > 0 && abs(p - startPeriod) > 1200)
    {
        drive_left(180);   // use YOUR function
        drive_forward(1.0);  // use YOUR function
        // Serial.println("Black");
        // drive_forward(1.0); 
    }
    drive_forward(2.0);
    Serial.println("Period: ");
    Serial.println(p);
    _delay_ms(200);
   

}
