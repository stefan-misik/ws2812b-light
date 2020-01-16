#include "buttons.h"

#include <avr/io.h>

void Buttons::initialize()
{
    DDRB  &= ~((1 << DDB0) | (1 << DDB1) | (1 << DDB3) | (1 << DDB4));
    PORTB |= ((1 << PORTB0) | (1 << PORTB1) | (1 << PORTB3) | (1 << PORTB4));
}

void Buttons::set(bool state)
{
    if (state)
    {
        PORTB |= (1 << PORTB4);
    }
    else
    {
        PORTB &= ~(1 << PORTB4);
    }
}
