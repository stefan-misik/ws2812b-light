#include "led_controller.h"
#include "buttons.h"
#include "time_service.h"
#include "animation.h"
#include "shared_storage.h"
#include "animation_list.h"


#include <util/delay.h>

PeriodicRoutine main_routine(1);

LedStrip<100> led_strip;
SharedStorage shared_storage;
AnimationList animations;


inline uint8_t ledStripEvent(Animation::Event type)
{
    return animations.current()->handleEvent(type, Animation::Param(led_strip.abstarctPtr()), &shared_storage);
}

void moveAnimation(int8_t offset)
{
    int8_t new_animation_number = animations.currentId() + offset;

    if (new_animation_number >= animations.size())
        new_animation_number = 0;
    else if (new_animation_number < 0)
        new_animation_number = animations.size() - 1;

    if (animations.currentId() != new_animation_number)
    {
        ledStripEvent(Animation::Event::STOP);
        shared_storage.destroy();
        animations.setCurrentId(new_animation_number);
        ledStripEvent(Animation::Event::START);
    }
}

void handleButtons()
{
    Buttons::update();
    for (uint8_t button = 0; button < Buttons::BUTTON_COUNT; ++button)
    {
        uint8_t state = Buttons::buttons[button].state();
        const auto result = animations.current()->handleEvent(
                Animation::Event::BUTTON,
                Animation::Param(
                        static_cast<Buttons::ButtonId>(button),
                        state),
                &shared_storage);

        if (state & ButtonFilter::PRESS)
        {
            if (result == Animation::Result::IS_OK)
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
    }
}

/**
 * @brief Periodic routine called every 8 milliseconds
 */
void mainPeriodicRoutine()
{
    handleButtons();

    if (Animation::Result::IS_OK == ledStripEvent(Animation::Event::UPDATE))
        LedController::update(led_strip.abstarctPtr());
}

int main(void)
{
    TimeService::initialize();
    Buttons::initialize();
    LedController::initialize();

    ledStripEvent(Animation::Event::START);
    while(1)
    {
        if (main_routine.shouldRun())
        {
            mainPeriodicRoutine();
            if (main_routine.hasElapsed())
            {
                // Spent too much time
                //while (true)
                //{ }
            }
        }
    }
}
