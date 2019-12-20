#include "leds.h"

Leds::LedState led_states[3];
Leds leds;

int main(void)
{
    leds.initialize(led_states, sizeof(led_states) / sizeof(led_states[0]));

    leds[0].red = 0xff;

    while(1)
    {
    }
}
