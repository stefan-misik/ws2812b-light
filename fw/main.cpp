#include "led_controller.h"
#include "analog_in.h"
#include "buttons.h"
#include "time_service.h"
#include "nvm_storage.h"
#include "animation.h"
#include "shared_storage.h"
#include "animation_list.h"


#include <util/delay.h>

PeriodicRoutine main_routine(1);

LedStrip<100> led_strip;
AnalogIn analog_in;
Buttons buttons;
SharedStorage shared_storage;
NvmStorage nvm_storage;
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

inline void handleButtons()
{
    const uint8_t state = buttons.run(analog_in.keypad());
    if (0 != state)
    {
        const auto result = animations.current()->handleEvent(
                Animation::Event::BUTTON,
                Animation::Param(buttons.button(), state),
                &shared_storage);

        if (state & ButtonFilter::PRESS)
        {
            if (result == Animation::Result::IS_OK)
            {
                switch (buttons.button())
                {
                case Buttons::RIGHT:
                    moveAnimation(1);
                    break;
                case Buttons::LEFT:
                    moveAnimation(-1);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

inline void handleNvmStorage()
{
    switch (nvm_storage.run())
    {
    case NvmStorage::Operation::NONE:
        break;

    case NvmStorage::Operation::WRITE_GLOBAL:
    {
        auto * global_config = nvm_storage.writeGlobalConfiguration();
        global_config->animatio_id = animations.currentId();
        break;
    }

    case NvmStorage::Operation::WRITE_ANIMATION:
    {
        // auto * animation_config = nvm_storage.writeAnimationConfigurationData();
        break;
    }

    case NvmStorage::Operation::READ_GLOBAL:
    {
        const auto * global_config = nvm_storage.readGlobalConfiguration();
        animations.setCurrentId(global_config->animatio_id);
        break;
    }

    case NvmStorage::Operation::READ_ANIMATION:
    {
        // const auto * animation_config = nvm_storage.readAnimationConfigurationData();
        break;
    }
    }
}

/**
 * @brief Periodic routine called every 8 milliseconds
 */
void mainPeriodicRoutine()
{
    analog_in.convert(AnalogIn::Channel::KEYPAD);
    handleButtons();

    if (Animation::Result::IS_OK == ledStripEvent(Animation::Event::UPDATE))
        LedController::update(led_strip.abstarctPtr(), ColorId::BLACK);
    else
        LedController::updateStatus(ColorId::BLACK);
}

int main(void)
{
    TimeService::initialize();
    LedController::initialize();

    analog_in.initialize();
    buttons.initialize();

    ledStripEvent(Animation::Event::START);
    while(1)
    {
        //handleNvmStorage();
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
