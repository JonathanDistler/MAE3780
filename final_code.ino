#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <math.h>

// ---------------- CONSTANTS ----------------
// wheel diameter (looks like inches)
float wheel_diam = 2.57874016;
// motor speed in RPM
float rpm = 45;
// distance between wheels (used for turning math)
float shaft_distance = 4.72440945;

// ---------------- GLOBALS ----------------
// last time the sensor triggered
volatile uint16_t lastTime = 0;
// time between sensor pulses
volatile uint16_t period = 0;

// baseline period (used to detect changes later)
int16_t startPeriod = 0;

// flag to know if we've started traversing something
bool traverse = false;
// counts how many traversals happened
uint16_t traverseCount = 0;

// ---------------- RUN STATE (NEW) ----------------
// basically "is the robot running or not"
uint8_t runActive = 0;

// ---------------- ISR ----------------
// this interrupt fires when the pin on PC2 changes
ISR(PCINT1_vect)
{
    // only act on HIGH signal
    if (PINC & (1 << PC2))
    {
        int16_t now = TCNT1;      // grab current timer value
        period = now - lastTime;  // compute time between pulses
        lastTime = now;           // update last time
    }
}

// ---------------- TIMING ----------------
// converts a distance into how long the motors should run
float time_delay_func(float distance){
    float wheel_circ = 3.14159265f * wheel_diam; // circumference
    float v = wheel_circ * (rpm / 60.0f);        // linear speed
    float t = distance / v;                      // time = d / v
    return t * 1000.0f;                          // convert to ms
}

// same idea but for turning a certain angle
float time_delay_func_ang(float angle_deg){
    float angle_rad = angle_deg * 3.14159265f / 180.0f;
    float wheel_circ = 3.14159265f * wheel_diam;

    float v = wheel_circ * (rpm / 60.0f);
    float omega = (2.0f * v) / shaft_distance;   // angular velocity

    float t = angle_rad / omega;                 // time needed to turn
    return t * 1000.0f;                          // ms
}

// safer delay so we don't overflow _delay_ms limits
void delay_ms_safe(float ms){
    while (ms >= 1.0f) {
        _delay_ms(1);
        ms -= 1.0f;
    }
}

// ---------------- MOTOR CONTROL ----------------
// shuts everything off
void stop_motors(){
    PORTB &= ~((1 << 0) | (1 << 3));
    PORTD &= ~((1 << 2) | (1 << 3));
}

// drives forward a given distance
void drive_forward(float distance){
    // set motor direction pins for forward
    PORTB = (PORTB & 0b11110111) | (1 << 3);
    PORTD = (PORTD & 0b11110011) | (1 << 2);

    // run motors for calculated time
    delay_ms_safe(time_delay_func(distance));
    stop_motors(); // then stop
}

// turns left by some degrees
void drive_left(float degrees){
    // left turn config (one wheel forward, one backward)
    PORTB = (PORTB & 0b11110111) | (1 << 3);
    PORTD = (PORTD & 0b11110011) | (1 << 3);

    // run for calculated turn time
    delay_ms_safe(time_delay_func_ang(degrees));
    stop_motors();
}

// ---------------- INIT ----------------
void setup()
{
    // set motor pins as outputs
    DDRB |= (1 << 0) | (1 << 3);
    DDRD |= (1 << 2) | (1 << 3);

    // sensor pin input
    DDRC &= ~(1 << PC2);
    // button input
    DDRB &= ~(1 << PB2);

    // enable pull-up on button
    PORTB |= (1 << PB2);

    // enable pin change interrupt for PC2
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT10);

    // setup timer1 (no prescaler)
    TCCR1A = 0;
    TCCR1B = (1 << CS10);

    sei(); // enable interrupts

    _delay_ms(500); // wait a bit for things to stabilize
    startPeriod = period; // capture baseline sensor timing
}

// ---------------- LOOP ----------------
void loop()
{
    // ---------------- BUTTON START (TOGGLE RUN) ----------------
    // if button is pressed (active low)
    if (!(PINB & (1 << PB2)))
    {
        _delay_ms(30); // debounce
        if (!(PINB & (1 << PB2)))
        {
            runActive = 1; // start running

            while (!(PINB & (1 << PB2))); // wait until release
            _delay_ms(30); // debounce again
        }
    }

    // IF NOT RUNNING → DO NOTHING
    if (!runActive) return;

    int16_t p;

    // safely copy the period (avoid interrupt conflict)
    cli();
    p = period;
    sei();

    // -------- COLOR CHANGE --------
    // detects a moderate change in period (likely different surface/color)
    if (p > 0 && abs(p - startPeriod) > 350 && abs(p - startPeriod) < 800)
    {
        drive_forward(1);   // move a bit
        drive_left(75);     // turn (not quite 90 yet)
        traverse = true;    // mark that we started traversing
        drive_forward(1);   // move again
    }

    // detects a large change (probably a bigger marker)
    if (p > 0 && abs(p - startPeriod) > 1200)
    {
        drive_left(180); // turn around
        drive_forward(1);

        if (traverse) traverseCount++; // count only if we were traversing
    }

    // after 2 traversals, do final move
    if (traverseCount == 2)
    {
        drive_left(90);
        drive_forward(12.0);

        stop_motors();
        runActive = 0;   // STOP AFTER FINISH
    }

    // default behavior: keep moving forward slowly
    drive_forward(1.0);
}