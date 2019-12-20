#include "leds.h"

#include <avr/io.h>

void Leds::initialize(LedState * leds, size_t count)
{
    leds_ = leds;
    count_ = count;

    // Set output direction to OC0B - PD5
    DDRD |= ((1 << DDD5));

    // Disable power reduction for Timer 0
    PRR &= ~((1 << PRTIM0));

    // Disable PWM output A
    TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));
    TCCR0A |=  ((0 << COM0A1) | (0 << COM0A0));
    // Set the PWM output B to clear on match and set at BOTTOM
    TCCR0A &= ~((1 << COM0B1) | (1 << COM0B0));
    TCCR0A |=  ((1 << COM0B1) | (0 << COM0B0));
    // Set the Fast PWM mode
    TCCR0A &= ~((1 << WGM01) | (1 << WGM00));
    TCCR0B &= ~((1 << WGM02));
    TCCR0A |=  ((1 << WGM01) | (1 << WGM00));
    TCCR0B |=  ((0 << WGM02));
    // Select Fio/1 as clock source
    TCCR0B &= ~((1 << CS02) | (1 << CS01) | (1 << CS00));
    TCCR0B |=  ((0 << CS02) | (0 << CS01) | (1 << CS00));

    OCR0A = 9;

}
