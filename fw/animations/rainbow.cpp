#include "animations/rainbow.h"

#include "tools/color.h"

uint8_t RainbowAnimation::handleEvent(Event type, Param param, SharedStorage * storage)
{
    auto s = [=]() -> auto & { return *storage->get<Shared>(); };

    switch (type)
    {
    case Event::START:
        storage->create<Shared>();
        return Result::IS_OK;

    case Event::UPDATE:
    {
        ++(s().step);
        if (2 != s().step)
            return Result::IGNORE_DEFAULT;
        s().step = 0;

        uint16_t hue = s().hue;
        LedState color;
        for (auto & led: param.ledStrip())
        {
            toSaturatedHue(hue, &color);
            hue = incrementHue(hue, space_increment_);
            led = color;
        }

        s().hue = incrementHue(s().hue, -time_increment_);

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
