#include "animations/sparks.h"

#include <stdlib.h>


uint8_t SparksAnimation::handleEvent(Event type, Param param, SharedStorage * storage)
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
        if (7 != s().step)
            return Result::IGNORE_DEFAULT;
        s().step = 0;

        for (auto & led: param.ledStrip())
            led = {0x47, 0x30, 0x0D};

        const uint16_t mask = 0x7FFF >> frequency_;
        const int num = (rand() & mask);
        if (num < param.ledStrip().led_count)
            param.ledStrip().leds[num] = {0xFF, 0xFF, 0xFF};

        return Result::IS_OK;
    }

    case Event::STOP:
        return Result::IS_OK;

    case Event::BUTTON:
        if (param.paramLo() & ButtonState::PRESS)
        {
            switch (param.buttonId())
            {
            case ButtonId::UP:
                if (7 != frequency_)
                    ++frequency_;
                break;
            case ButtonId::DOWN:
                if (0 != frequency_)
                    --frequency_;
                break;
            }
        }
        return Result::IS_OK;
    }
    return Result::IS_OK;
}
