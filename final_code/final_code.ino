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

const uint16_t MODERATE_COLOR_MIN = 350;
const uint16_t MODERATE_COLOR_MAX = 800;
const uint16_t LARGE_COLOR_MIN = 1200;

enum ColorEvent {
    COLOR_NONE = 0,
    COLOR_MODERATE = 1,
    COLOR_LARGE = 2
};

// ---------------- GLOBALS ----------------
// last time the sensor triggered
volatile uint16_t lastTime = 0;
// time between sensor pulses
volatile uint16_t period = 0;

// baseline period (used to detect changes later)
volatile int16_t startPeriod = 0;

// flag to know if we've started traversing something
bool traverse = false;
// counts how many traversals happened
uint16_t traverseCount = 0;

// ---------------- RUN STATE (NEW) ----------------
// basically "is the robot running or not"
volatile uint8_t runActive = 0;
volatile uint8_t resetRequested = 0;
volatile uint8_t colorEvent = COLOR_NONE;
volatile uint8_t colorState = COLOR_NONE;
volatile uint8_t lastButtonState = (1 << PB2);

void stop_motors();

uint16_t period_delta(uint16_t p, uint16_t baseline)
{
    return (p > baseline) ? (p - baseline) : (baseline - p);
}

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

        if (runActive && startPeriod > 0)
        {
            uint16_t delta = period_delta(period, (uint16_t)startPeriod);
            uint8_t detectedColor = COLOR_NONE;

            if (delta > LARGE_COLOR_MIN)
            {
                detectedColor = COLOR_LARGE;
            }
            else if (delta > MODERATE_COLOR_MIN && delta < MODERATE_COLOR_MAX)
            {
                detectedColor = COLOR_MODERATE;
            }

            if (detectedColor == COLOR_NONE)
            {
                colorState = COLOR_NONE;
            }
            else if (detectedColor != colorState)
            {
                colorState = detectedColor;
                if (colorEvent == COLOR_NONE)
                {
                    colorEvent = detectedColor;
                }
            }
        }
    }
}

// this interrupt fires when the start/reset button on PB2 changes
ISR(PCINT0_vect)
{
    uint8_t buttonState = PINB & (1 << PB2);

    // active-low button: a high-to-low transition requests a robot reset
    if (!buttonState && lastButtonState)
    {
        resetRequested = 1;
        runActive = 0;
        PORTB &= ~((1 << 0) | (1 << 3));
        PORTD &= ~((1 << 2) | (1 << 3));
    }

    lastButtonState = buttonState;
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
        if (resetRequested) {
            stop_motors();
            return;
        }
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
    if (resetRequested) return;

    // set motor direction pins for forward
    PORTB = (PORTB & 0b11110111) | (1 << 3);
    PORTD = (PORTD & 0b11110011) | (1 << 2);

    // run motors for calculated time
    delay_ms_safe(time_delay_func(distance));
    stop_motors(); // then stop
}

// turns left by some degrees
void drive_left(float degrees){
    if (resetRequested) return;

    // left turn config (one wheel forward, one backward)
    PORTB = (PORTB & 0b11110111) | (1 << 3);
    PORTD = (PORTD & 0b11110011) | (1 << 3);

    // run for calculated turn time
    delay_ms_safe(time_delay_func_ang(degrees));
    stop_motors();
}

void capture_start_period()
{
    uint8_t sreg = SREG;
    cli();
    startPeriod = period;
    colorEvent = COLOR_NONE;
    colorState = COLOR_NONE;
    SREG = sreg;
}

uint8_t take_color_event()
{
    uint8_t event;
    uint8_t sreg = SREG;
    cli();
    event = colorEvent;
    colorEvent = COLOR_NONE;
    SREG = sreg;
    return event;
}

void reset_robot()
{
    stop_motors();

    uint8_t sreg = SREG;
    cli();
    runActive = 0;
    traverse = false;
    traverseCount = 0;
    colorEvent = COLOR_NONE;
    colorState = COLOR_NONE;
    period = 0;
    lastTime = TCNT1;
    startPeriod = 0;
    resetRequested = 0;
    SREG = sreg;

    _delay_ms(100);          // let the color sensor produce a fresh baseline
    capture_start_period();
    runActive = 1;
}

void handle_button_reset()
{
    if (!resetRequested) return;

    stop_motors();
    _delay_ms(30); // debounce the active-low start/reset button

    while (!(PINB & (1 << PB2))) {
        stop_motors();
    }

    _delay_ms(30);
    reset_robot();
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

    lastButtonState = PINB & (1 << PB2);

    // enable pin change interrupts for the button on PB2 and color sensor on PC2
    PCICR |= (1 << PCIE0) | (1 << PCIE1);
    PCMSK0 |= (1 << PCINT2);
    PCMSK1 |= (1 << PCINT10);

    // setup timer1 (no prescaler)
    TCCR1A = 0;
    TCCR1B = (1 << CS10);

    sei(); // enable interrupts

    _delay_ms(500); // wait a bit for things to stabilize
    capture_start_period(); // capture baseline sensor timing
}

// ---------------- LOOP ----------------
void loop()
{
    handle_button_reset();

    // IF NOT RUNNING → DO NOTHING
    if (!runActive) return;

    uint8_t event = take_color_event();

    // -------- COLOR CHANGE --------
    // moderate color changes are detected and latched by the PC2 interrupt
    if (event == COLOR_MODERATE)
    {
        drive_forward(1);   // move a bit
        drive_left(75);     // turn (not quite 90 yet)
        traverse = true;    // mark that we started traversing
        drive_forward(1);   // move again
        if (resetRequested) return;
    }

    // large color changes are detected and latched by the PC2 interrupt
    if (event == COLOR_LARGE)
    {
        drive_left(180); // turn around
        drive_forward(1);

        if (traverse) traverseCount++; // count only if we were traversing
        if (resetRequested) return;
    }

    // after 2 traversals, do final move
    if (traverseCount == 2)
    {
        drive_left(90);
        drive_forward(12.0);

        stop_motors();
        runActive = 0;   // STOP AFTER FINISH
        return;
    }

    // default behavior: keep moving forward slowly
    drive_forward(1.0);
}
