#include "led_controller.h"
#include "analog_in.h"
#include "buttons.h"
#include "ir_receiver.h"
#include "time_service.h"
#include "status_led.h"
#include "music.h"
#include "nvm_storage.h"
#include "animation.h"
#include "shared_storage.h"
#include "animation_list.h"


#include <util/delay.h>

PeriodicRoutine main_routine(1);

LedStrip<100> led_strip;
AnalogIn analog_in;
Buttons buttons;
IrReceiver ir_receiver;
StatusLed status_led;
SharedStorage shared_storage;
NvmStorage nvm_storage;
AnimationList animations;
Music music;


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

inline void handleButtons(Animation::Events * events)
{
    const auto state = buttons.run(analog_in.keypad());
    Buttons::ButtonId button = ir_receiver.run();
    if (Buttons::NONE == button)
    {
        if (state & ButtonFilter::PRESS)
            button = buttons.button();
    }

    switch (button)
    {
    case Buttons::UP:
        events->setFlag(Animation::Events::SETTINGS_UP);
        break;
    case Buttons::DOWN:
        events->setFlag(Animation::Events::SETTINGS_DOWN);
        break;
    case Buttons::RIGHT:
        moveAnimation(1);
        break;
    case Buttons::LEFT:
        moveAnimation(-1);
        break;
    case Buttons::O_BTN:
        music.change(-1);
        break;
    case Buttons::X_BTN:
        music.change(1);
        break;
    default:
        break;
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
    Animation::Events events;
    analog_in.convert(AnalogIn::Channel::KEYPAD);
    handleButtons(&events);

    {
        const auto music_result = music.play();
        switch (music_result)
        {
        case Music::Result::STOPPED: events.setFlag(Animation::Events::MUSIC_STOPPED); break;
        case Music::Result::CHANGE: events.setFlag(Animation::Events::NOTE_CHANGED); break;
        default: break;
        }
    }

    if (0 != events)
        animations.current()->handleEvent(Animation::Event::EVENTS, Animation::Param{events}, &shared_storage);

    if (Animation::Result::IS_OK == ledStripEvent(Animation::Event::UPDATE))
        LedController::update(led_strip.abstarctPtr(), status_led.color());
    else
        LedController::updateStatus(status_led.color());
}

int main(void)
{
    TimeService::initialize();
    LedController::initialize();
    Music::initialize();

    analog_in.initialize();
    buttons.initialize();
    ir_receiver.initialize();

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
