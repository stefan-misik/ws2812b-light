#include "led_controller.h"
#include "buttons.h"
#include "time_service.h"
#include "animation.h"
#include "shared_storage.h"

#include "animations/rainbow.h"
#include "animations/color.h"
#include "animations/retro.h"
#include "animations/twinkle.h"

#include <util/delay.h>
#include <avr/pgmspace.h>

static const LedState twinkle_sparks_seq[] PROGMEM =
{
        LedState{0x47300D},
        LedState{0xFFFFFF},
        LedState{0xEDCC9C},
        LedState{0xDEA34A},
        LedState{0xA8711F},
        LedState{0x704B15},
        {0x00, 0x00, 0x00},
};

static const LedState twinkle_shimmer_seq[] PROGMEM =
{
        LedState{0x47300D},

        LedState{0x2F2009},
        LedState{0x1A1105},

        LedState{0x040301},

        LedState{0x1A1105},
        LedState{0x2F2009},

        {0x00, 0x00, 0x00},
};


PeriodicRoutine main_routine(1);

LedStrip<100> led_strip;

SharedStorage shared_storage;

ColorAnimation color;
RainbowAnimation rainbow;
RetroAnimation retro;
TwinkleAnimation twinkle_sparks{twinkle_sparks_seq};
TwinkleAnimation twinkle_shimmer{twinkle_shimmer_seq};

Animation * animations[] = {
    &color,
    &rainbow,
    &retro,
    &twinkle_sparks,
    &twinkle_shimmer,
};
constexpr int8_t ANIMATION_COUNT = sizeof(animations) / sizeof(animations[0]);

int8_t current_animation_number = 0;
Animation * current_animation = animations[current_animation_number];


inline uint8_t ledStripEvent(Animation::Event type)
{
    return current_animation->handleEvent(
            type,
            Animation::Param(led_strip.abstarctPtr()),
            &shared_storage);
}

void moveAnimation(int8_t offset)
{
    int8_t new_animation_number = current_animation_number + offset;

    if (new_animation_number >= ANIMATION_COUNT)
        new_animation_number = 0;
    else if (new_animation_number < 0)
        new_animation_number = ANIMATION_COUNT - 1;

    if (current_animation_number != new_animation_number)
    {
        ledStripEvent(Animation::Event::STOP);
        shared_storage.destroy();
        current_animation_number = new_animation_number;
        current_animation = animations[new_animation_number];
        ledStripEvent(Animation::Event::START);
    }
}

void handleButtons()
{
    Buttons::update();
    for (uint8_t button = 0; button < Buttons::BUTTON_COUNT; ++button)
    {
        uint8_t state = Buttons::buttons[button].state();
        const auto result = current_animation->handleEvent(
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
