#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

volatile uint32_t period = 0;
volatile uint16_t timerValue = 0;
volatile uint8_t lastState = 0;

ISR(PCINT1_vect)   // PORTC interrupt group
{
    uint8_t currentState = PINC & (1 << PC2);  // A2 = PC2

    if (currentState && !lastState)
    {
        TCNT1 = 0;
    }
    else if (!currentState && lastState)
    {
        timerValue = TCNT1;
        period = timerValue;   // raw ticks
    }

    lastState = currentState;
}

void initColor()
{
    DDRC &= ~(1 << PC2);   // A2 input

    PCICR |= (1 << PCIE1);     // enable PORTC interrupt group
    PCMSK1 |= (1 << PCINT10);  // PC2 = PCINT10

    TCCR1A = 0x00;
    TCCR1B = (1 << CS10);      // no prescaler

    sei();
}

void setup()
{
    Serial.begin(9600);
    initColor();
}

void loop()
{
    uint32_t copy;

    cli();
    copy = period;
    sei();

    if (copy == 0)
    {
        Serial.println("No signal");
    }
    else if (copy < 300)
    {
        Serial.println("YELLOW");
    }
    else
    {
        Serial.println("BLUE");
    }

    delay(200);
}