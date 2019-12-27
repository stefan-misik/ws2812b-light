#include "led_controller.h"
#include "time_service.h"
#include "animation.h"
#include "animations/rainbow.h"
#include "animations/drip.h"

#include <util/delay.h>

LedStrip<30> led_strip;
LedController led_controller;

RainbowAnimation rainbow;
DripAnimation drip;
Animation * animation;

int main(void)
{
    led_controller.initialize(led_strip.abstarctPtr());

    animation = &rainbow;
    animation->reset(led_strip.abstarctPtr());
    while(1)
    {
        uint8_t delay = animation->step(led_strip.abstarctPtr());
        led_controller.update();
        _delay_ms(5);
    }
}
