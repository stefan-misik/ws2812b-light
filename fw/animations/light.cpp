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
        type_ = Type::COLD;
        intensity_ = 0xff;
        redraw_ = false;
        break;

    case Event::INIT:
        fillLedStrip(reinterpret_cast<AbstractLedStrip *>(parameter));
        redraw_ = false;
        break;

    case Event::ANIMATION_ROTATE:
        if (Type::COLD == type_)
        {
            type_ = Type::WARM;
        }
        else
        {
            type_ = Type::COLD;
        }
        break;

    case Event::STEP:
        if (redraw_)
        {
            fillLedStrip(reinterpret_cast<AbstractLedStrip *>(parameter));
            redraw_ = false;
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
        if (Type::COLD == type_)
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

    if (Type::COLD == type_)
    {
        r = intensity_;
        g = intensity_;
        b = intensity_;
    }
    else
    {
        r = intensity_;
        g = intensity_ >> 1;
        b = intensity_ >> 3;
    }

    for (auto & led : *led_strip)
    {
        led = {g, r, b};
    }
}
