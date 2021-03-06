#include "buttons.h"

#include <avr/io.h>

ButtonFilter Buttons::buttons[Buttons::BUTTON_COUNT];

void Buttons::initialize()
{
    DDRB  &= ~((1 << DDB0) | (1 << DDB1) | (1 << DDB3) | (1 << DDB4));
    PORTB |= ((1 << PORTB0) | (1 << PORTB1) | (1 << PORTB3) | (1 << PORTB4));
}

void Buttons::update()
{
    uint8_t pins = ~PINB;
    buttons[0].updateButton(pins & (1 << PINB0));
    buttons[1].updateButton(pins & (1 << PINB1));
    buttons[2].updateButton(pins & (1 << PINB3));
    buttons[3].updateButton(pins & (1 << PINB4));
}
