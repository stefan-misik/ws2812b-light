/**
 * @file
 */

#include "animations/light.h"
#include "buttons.h"


Animation::Result LightAnimation::handleEvent(
        Event event,
        intptr_t parameter)
{
    switch (event)
    {
    case Event::CREATE:
        type_ = Type::OFF;
        intensity_ = 0xff;
        redraw_ = false;
        break;

    case Event::INIT:
        redraw_ = true;
        break;

    case Event::ANIMATION_ROTATE:
        if (Type::OFF == type_)
        {
            type_ = Type::WARM;
        }
        else
        {
            type_ = Type::OFF;
        }
        break;

    case Event::STEP:
        if (redraw_)
        {
            fillLedStrip(reinterpret_cast<AbstractLedStrip *>(parameter));
            redraw_ = false;
            return Result::NONE;
        }
        else
        {
            return Result::IGNORE_DEFAULT;
        }
        break;

    case Event::KEY_PRESS:
        return handleKeyPress(Buttons::ButtonId(parameter));

    }

    return Result::NONE;
}

Animation::Result LightAnimation::handleKeyPress(Buttons::ButtonId button)
{
    switch (button)
    {
    case Buttons::UP:
        if (intensity_ < 252)
        {
            intensity_ += 4;
            redraw_ = true;
        }
        break;

    case Buttons::RIGHT:
        if (Type::WARM == type_)
        {
            return Result::NONE;
        }
        else
        {
            ++ type_;
            redraw_ = true;
            return Result::IGNORE_DEFAULT;
        }
        break;

    case Buttons::LEFT:
        if (Type::OFF == type_)
        {
            return Result::NONE;
        }
        else
        {
            -- type_;
            redraw_ = true;
            return Result::IGNORE_DEFAULT;
        }
        break;

    case Buttons::DOWN:
        if (intensity_ > 3)
        {
            intensity_ -= 4;
            redraw_ = true;
        }
        break;
    }
    return Result::NONE;
}

void LightAnimation::fillLedStrip(AbstractLedStrip * led_strip) const
{
    uint8_t r, g, b;

    switch (type_)
    {
    case Type::OFF:
        r = 0;
        g = 0;
        b = 0;
        break;

    case Type::COLD:
        r = intensity_;
        g = intensity_;
        b = intensity_;
        break;

    case Type::WARM:
        r = intensity_;
        g = intensity_ >> 1;
        b = intensity_ >> 3;
        break;
    }

    for (auto & led : *led_strip)
    {
        led.red = r;
        led.green = g;
        led.blue = b;
    }
}
