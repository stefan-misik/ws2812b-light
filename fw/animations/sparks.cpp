#include "animations/sparks.h"

#include <stdlib.h>


uint8_t SparksAnimation::handleEvent(Event type, Param param)
{
    switch (type)
    {
    case Event::START:
        step_ = 0;
        return Result::IS_OK;

    case Event::UPDATE:
    {
        ++step_;
        if (7 != step_)
            return Result::IGNORE_DEFAULT;
        step_ = 0;

        for (auto & led: *param.ledStrip())
            led = {0x47, 0x30, 0x0D};

        const uint16_t mask = 0x7FFF >> frequency_;
        const int num = (rand() & mask);
        if (num < param.ledStrip()->led_count)
            param.ledStrip()->leds[num] = {0xFF, 0xFF, 0xFF};

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
