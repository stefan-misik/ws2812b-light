/**
 * @file
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "led_strip.h"
#include "buttons.h"
#include "shared_storage.h"
#include "nvm_storage.h"

#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Class defining a single type of animation
 */
class Animation
{
public:
    using ButtonId = Buttons::ButtonId;
    using ButtonState = ButtonFilter::State;

    class Events;
    struct Param;

    enum Result: uint8_t
    {
        IS_OK,
        IGNORE_DEFAULT,
    };

    enum class Event: uint8_t
    {
        /**
         * @brief Start the animation - initialize the strip
         */
        START,

        /**
         * @brief Render animation frame
         *
         * Default action is to update the LED strip.
         */
        UPDATE,

        /**
         * @brief Stop the animation
         */
        STOP,

        /**
         * @brief Handle the external events
         */
        EVENTS,

        /**
         * @brief Save the current configuration of the animation
         */
        SAVE_CONFIG,

        /**
         * @brief Load the configuration of the animation
         */
        LOAD_CONFIG
    };

    /**
     * @brief Handle event of given type
     *
     * @param type Type of the event to be handled
     * @param param Parameter associated with the event
     * @param[in,out] storage Shared storage object
     *
     * @return Result of the handling
     */
    virtual uint8_t handleEvent(Event type, Param param, SharedStorage * storage) = 0;

    // Theses objects are never destroyed, and we can save significant amount of
    // memory in v-tables, since they are copied in RAM
    //virtual ~Animation() = default;
};


class Animation::Events
{
public:
    enum Type: uint8_t
    {
        SETTINGS_UP,
        SETTINGS_DOWN,
        MUSIC_STOPPED,
        NOTE_CHANGED,
    };

    explicit Events(uint8_t flags = 0):
        flags_(flags)
    { }

    void setFlag(Type type) { flags_ |= (1 << static_cast<uint8_t>(type)); }
    bool isFlagSet(Type type) const { return flags_ & (1 << static_cast<uint8_t>(type)); }

    void setFlagIf(bool cond, Type type)
    {
        if (cond)
            setFlag(type);
    }

    operator uint8_t() const { return flags_; }

private:
    uint8_t flags_ = 0;
};


struct Animation::Param
{
    Param(uintptr_t new_value):
        value(new_value)
    { }

    Param(uint8_t hi, uint8_t lo):
        value(static_cast<uintptr_t>(hi) << 8 | static_cast<uintptr_t>(lo))
    { }

    Param(const Events & flags):
        value(flags)
    { }

    Param(ButtonId button_id, uint8_t state):
        Param(static_cast<uint8_t>(button_id), state)
    { }

    explicit Param(AbstractLedStrip * led_strip):
            value(reinterpret_cast<uintptr_t>(led_strip))
    { }

    explicit Param(AnimationConfigurationData * config):
            value(reinterpret_cast<uintptr_t>(config))
    { }

    uint8_t paramHi() const { return value >> 8; }
    uint8_t paramLo() const { return value & 0xFF; }

    void setParamHi(uint8_t new_hi)
    {
        value = (static_cast<uintptr_t>(new_hi) << 8) | (value & 0x00FF);
    }
    void setParamLo(uint8_t new_lo)
    {
        value = static_cast<uintptr_t>(new_lo) | (value & 0xFF00);
    }

    Events events() const
    {
        return Events{static_cast<uint8_t>(value)};
    }

    void setEvents(const Events & events)
    {
        value = events;
    }

    AbstractLedStrip & ledStrip() const
    {
        return *reinterpret_cast<AbstractLedStrip *>(value);
    }
    void setLedStrip(AbstractLedStrip * new_led_strip)
    {
        value = reinterpret_cast<uintptr_t>(new_led_strip);
    }

    const AnimationConfigurationData & loadConfigurationData() const
    {
        return *reinterpret_cast<const AnimationConfigurationData *>(value);
    }
    AnimationConfigurationData & saveConfigurationData() const
    {
        return *reinterpret_cast<AnimationConfigurationData *>(value);
    }
    void setConfigurationData(AnimationConfigurationData * config)
    {
        value = reinterpret_cast<uintptr_t>(config);
    }

    uintptr_t value;
};


#endif  // ANIMATION_H_
