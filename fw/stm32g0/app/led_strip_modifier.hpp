/**
 * @file
 */

#ifndef APP_LED_STRIP_MODIFIER_HPP_
#define APP_LED_STRIP_MODIFIER_HPP_

#include "led_strip.hpp"


class LedStripModifier
{
public:
    void next()
    {
        if (Type::BOTH == type_)
            type_ = Type::NONE;
        else
            type_ = static_cast<Type>(static_cast<int>(type_) + 1);
    }

    void modify(AbstractLedStrip * leds) const
    {
        if (Type::NONE != type_)
        {
            (*leds)[0] = LedState(0xFFFFFF);
            if (Type::BOTH == type_)
                (*leds)[leds->led_count - 1] = LedState(0xFFFFFF);
        }
    }

private:
    enum class Type
    {
        NONE,
        START,
        BOTH,
    };

    Type type_ = Type::NONE;
};

#endif  // APP_LED_STRIP_MODIFIER_HPP_

