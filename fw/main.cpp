#include "led_controller.h"
#include "buttons.h"
#include "time_service.h"
#include "animation.h"

#include "animations/cold_light.h"
#include "animations/rainbow.h"
#include "animations/drip.h"

#include <util/delay.h>

PeriodicRoutine main_routine(1);

LedStrip<30> led_strip;

RainbowAnimation rainbow;
DripAnimation drip;
ColdLightAnimation cold_light;

Animation * animations[] = {&cold_light, &rainbow, &drip};
constexpr int8_t ANIMATION_COUNT = sizeof(animations) / sizeof(animations[0]);

int8_t current_animation_number = 0;
Animation * current_animation = animations[current_animation_number];


Animation::Result currentLedStripEvent(Animation::Event event)
{
    return current_animation->handleEvent(event,
            reinterpret_cast<intptr_t>(led_strip.abstarctPtr()));
}

void moveAnimation(int8_t offset)
{
    int8_t new_animation_number = current_animation_number + offset;

    if (new_animation_number >= ANIMATION_COUNT)
    {
        new_animation_number = 0;
    }
    else if (new_animation_number < 0)
    {
        new_animation_number = ANIMATION_COUNT - 1;
    }

    if (current_animation_number != new_animation_number)
    {
        currentLedStripEvent(Animation::Event::DEINIT);
        current_animation_number = new_animation_number;
        current_animation = animations[new_animation_number];
        currentLedStripEvent(Animation::Event::INIT);
    }
}

void handleButtons()
{
    Buttons::update();
    for (uint8_t button = 0; button < Buttons::BUTTON_COUNT; ++button)
    {
        uint8_t state = Buttons::buttons[button].state();

        if (state & ButtonFilter::PRESS)
        {
            Animation::Result result =
                    current_animation->handleEvent(Animation::Event::KEY_PRESS,
                            button);
            if (result != Animation::Result::IGNORE_DEFAULT)
            {
                switch (button)
                {
                case Buttons::RIGHT:
                    moveAnimation(1);
                    break;
                case Buttons::LEFT:
                    moveAnimation(-1);
                    break;
                }
            }
        }

        if (state & ButtonFilter::DOWN)
        {
            current_animation->handleEvent(Animation::Event::KEY_DOWN, button);
        }

        if (state & ButtonFilter::UP)
        {
            current_animation->handleEvent(Animation::Event::KEY_UP, button);
        }
    }
}

/**
 * @brief Periodic routine called every 8 milliseconds
 */
void mainPeriodicRoutine()
{
    handleButtons();

    currentLedStripEvent(Animation::Event::STEP);
    LedController::update(led_strip.abstarctPtr());
}

int main(void)
{
    TimeService::initialize();
    Buttons::initialize();
    LedController::initialize();

    for (auto & animation: animations)
    {
        animation->handleEvent(Animation::Event::CREATE, 0);
    }

    currentLedStripEvent(Animation::Event::INIT);

    while(1)
    {
        uint8_t current_time = TimeService::getTime();

        if (main_routine.shouldRunAt(current_time))
        {
            mainPeriodicRoutine();
        }
    }
}
