#include "led_controller.h"
#include "buttons.h"
#include "time_service.h"
#include "animation.h"

#include "animations/rainbow.h"
#include "animations/color.h"
#include "animations/sparks.h"

#include <util/delay.h>

PeriodicRoutine main_routine(1);

LedStrip<100> led_strip;

ColorAnimation color;
RainbowAnimation rainbow;
SparksAnimation sparks;

Animation * animations[] = {
    &color,
    &rainbow,
    &sparks
};
constexpr int8_t ANIMATION_COUNT = sizeof(animations) / sizeof(animations[0]);

int8_t current_animation_number = 0;
Animation * current_animation = animations[current_animation_number];


bool moveAnimation(int8_t offset)
{
    bool redraw = false;
    int8_t new_animation_number = current_animation_number + offset;

    if (new_animation_number >= ANIMATION_COUNT)
        new_animation_number = 0;
    else if (new_animation_number < 0)
        new_animation_number = ANIMATION_COUNT - 1;

    if (current_animation_number != new_animation_number)
    {
        current_animation->stop(led_strip.abstarctPtr());
        current_animation_number = new_animation_number;
        current_animation = animations[new_animation_number];
        redraw = current_animation->start(led_strip.abstarctPtr());
    }
    return redraw;
}

bool handleButtons()
{
    bool redraw = false;

    Buttons::update();
    for (uint8_t button = 0; button < Buttons::BUTTON_COUNT; ++button)
    {
        uint8_t state = Buttons::buttons[button].state();
        const bool result = current_animation->handleButton(
                static_cast<Buttons::ButtonId>(button),
                state);

        if (state & ButtonFilter::PRESS)
        {
            if (result)
            {
                switch (button)
                {
                case Buttons::RIGHT:
                    if (moveAnimation(1))
                        redraw = true;
                    break;
                case Buttons::LEFT:
                    if (moveAnimation(-1))
                        redraw = true;
                    break;
                }
            }
        }
    }
    return redraw;
}

/**
 * @brief Periodic routine called every 8 milliseconds
 */
void mainPeriodicRoutine()
{
    if (handleButtons())
        LedController::update(led_strip.abstarctPtr());
    else
    {
        if (current_animation->update(led_strip.abstarctPtr()))
            LedController::update(led_strip.abstarctPtr());
    }
}

int main(void)
{
    TimeService::initialize();
    Buttons::initialize();
    LedController::initialize();

    if (current_animation->start(led_strip.abstarctPtr()))
        LedController::update(led_strip.abstarctPtr());

    while(1)
    {
        uint8_t current_time = TimeService::getTime();

        if (main_routine.shouldRunAt(current_time))
            mainPeriodicRoutine();
    }
}
