#include "animations/rainbow.h"

#include "tools/color.h"

uint8_t RainbowAnimation::handleEvent(Event type, Param param)
{
    switch (type)
    {
    case Event::START:
        step_ = 0;
        return Result::IS_OK;

    case Event::UPDATE:
    {
        uint16_t hue = hue_;
        LedState color;
        for (auto & led: *param.ledStrip())
        {
            toSaturatedHue(hue, &color);
            hue = incrementHue(hue, space_increment_);
            led = color;
        }

        hue_ = incrementHue(hue_, time_increment_);

        return Result::IS_OK;
    }

    case Event::STOP:
        return Result::IS_OK;

    case Event::BUTTON:
        if (param.paramLo() & ButtonState::PRESS)
        {
            switch (param.buttonId())
            {
            case ButtonId::UP: ++space_increment_; break;
            case ButtonId::DOWN: ++time_increment_; break;
            }
        }
        return Result::IS_OK;
    }
    return Result::IS_OK;
}
