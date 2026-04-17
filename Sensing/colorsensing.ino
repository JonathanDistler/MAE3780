#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

// --- Global Variables ---
volatile uint32_t period = 0;
volatile uint16_t timerValue = 0;
volatile uint8_t lastState = 0;

// --- ISR ---
ISR(PCINT2_vect)
{
    uint8_t currentState = PIND & (1 << PD2);  // Read PD2 safely

    // Rising edge
    if ((currentState != 0) && (lastState == 0))
    {
        TCNT1 = 0;
    }
    // Falling edge
    else if ((currentState == 0) && (lastState != 0))
    {
        timerValue = TCNT1;
        period = timerValue * 2;
    }

    lastState = currentState;
}

// --- init ---
void initColor()
{
    // PD2 input
    DDRD &= ~(1 << PD2);

    // PD4 output
    DDRD |= (1 << PD4);

    // Enable PCINT2 group
    PCICR |= (1 << PCIE2);

    // Disable all PCINT2 pins initially
    PCMSK2 = 0x00;   // <-- FIXED (semicolon added)

    // Timer1 normal mode
    TCCR1A = 0x00;
    TCCR1B = (1 << CS10);  // prescaler = 1

    sei();
}

// --- measure ---
uint16_t getColor()
{
    // Enable interrupt on PD2 (PCINT18)
    PCMSK2 |= (1 << PCINT18);

    _delay_ms(10);

    // Disable interrupt
    PCMSK2 &= ~(1 << PCINT18);

    uint32_t period_us = (period * 1000000UL) / 16000000UL;
    return (uint16_t)period_us;
}

// --- main ---
int main(void)
{
    initColor();

    while (1)
    {
        uint16_t ticks = getColor();

        if (ticks > 200)
        {
            PORTD |= (1 << PD4);   // ON
        }
        else
        {
            PORTD &= ~(1 << PD4);  // OFF
        }

        _delay_ms(200);
    }
}