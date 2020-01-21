#include "led_controller.h"
#include "buttons.h"
#include "time_service.h"
#include "animation.h"
#include "animations/rainbow.h"
#include "animations/drip.h"

#include <util/delay.h>

PeriodicRoutine main_routine(1);

LedStrip<30> led_strip;

RainbowAnimation rainbow;
DripAnimation drip;
Animation * animation;

uint8_t led;

/**
 * @brief Periodic routine called every 8 milliseconds
 */
void mainPeriodicRoutine()
{
    Buttons::update();
    //uint8_t delay = animation->step(led_strip.abstarctPtr());

    for (auto & led: led_strip)
    {
        led = {0x00, 0x00, 0x00};
    }


    if (Buttons::buttons[Buttons::UP].state() & ButtonFilter::PRESS)
    {
        ++led;
        if (led == led_strip.led_count)
        {
            led = 0;
        }
    }

    if (Buttons::buttons[Buttons::DOWN].state() & ButtonFilter::PRESS)
    {
        if (0 == led)
        {
            led = led_strip.led_count - 1;
        }
        else
        {
            --led;
        }
    }


    led_strip[led] = {0x0f, 0x0f, 0x0f};

    LedController::update(led_strip.abstarctPtr());
}

int main(void)
{
    TimeService::initialize();
    Buttons::initialize();
    LedController::initialize();

    led = 0;

    animation = &rainbow;
    //animation->reset(led_strip.abstarctPtr());
    while(1)
    {
        uint8_t current_time = TimeService::getTime();

        if (main_routine.shouldRunAt(current_time))
        {
            mainPeriodicRoutine();
        }
    }
}
