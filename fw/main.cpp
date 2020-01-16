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

/**
 * @brief Periodic routine called every 8 milliseconds
 */
void mainPeriodicRoutine()
{
    uint8_t delay = animation->step(led_strip.abstarctPtr());
    LedController::update(led_strip.abstarctPtr());
}

int main(void)
{
    TimeService::initialize();
    Buttons::initialize();
    LedController::initialize();

    animation = &rainbow;
    animation->reset(led_strip.abstarctPtr());
    while(1)
    {
        uint8_t current_time = TimeService::getTime();

        if (main_routine.shouldRunAt(current_time))
        {
            mainPeriodicRoutine();
        }
    }
}
