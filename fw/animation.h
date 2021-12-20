/**
 * @file
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "led_strip.h"
#include "buttons.h"
#include "shared_storage.h"

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
         * @brief Handle the button press
         */
        BUTTON
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

    virtual ~Animation() = default;
};

struct Animation::Param
{
    Param(uintptr_t new_value):
        value(new_value)
    { }

    Param(uint8_t hi, uint8_t lo):
        value(static_cast<uintptr_t>(hi) << 8 | static_cast<uintptr_t>(lo))
    { }

    Param(ButtonId button_id, uint8_t state):
        Param(static_cast<uint8_t>(button_id), state)
    { }

    explicit Param(AbstractLedStrip * led_strip):
            value(reinterpret_cast<uintptr_t>(led_strip))
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

    ButtonId buttonId() const { return static_cast<ButtonId>(paramHi()); }
    uint8_t buttonState() const { return paramLo(); }
    void setButtonId(ButtonId new_id) { setParamHi(new_id); }
    void setButtonState(uint8_t new_state) { setParamLo(new_state); }

    AbstractLedStrip & ledStrip() const
    {
        return *reinterpret_cast<AbstractLedStrip *>(value);
    }
    void setLedStrip(AbstractLedStrip * new_led_strip)
    {
        value = reinterpret_cast<uintptr_t>(new_led_strip);
    }

    uintptr_t value;
};


#endif  // ANIMATION_H_
