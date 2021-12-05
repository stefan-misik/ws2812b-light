#include "animations/sparks.h"

#include <stdlib.h>

#include <avr/pgmspace.h>


uint8_t SparksAnimation::handleEvent(Event type, Param param)
{
    switch (type)
    {
    case Event::START:
        step_ = 0;
        return Result::IS_OK;

    case Event::UPDATE:
        ++step_;
        if (STEPS != step_)
            return Result::IGNORE_DEFAULT;
        step_ = 0;

        for (auto & led: *param.ledStrip())
            if ((rand() & 0x7ff) <= frequency_)
                led = {0xFF, 0xFF, 0xFF};
            else
                led = {0x47, 0x30, 0x0D};

        return Result::IS_OK;

    case Event::STOP:
        return Result::IS_OK;

    case Event::BUTTON:
        if (param.paramLo() & ButtonState::PRESS)
        {
            switch (param.buttonId())
            {
            case ButtonId::UP:
                if (0x7ff != frequency_)
                    ++frequency_;
                break;
            case ButtonId::DOWN:
                if (-1 != frequency_)
                    --frequency_;
                break;
            }
        }
        return Result::IS_OK;
    }
    return Result::IS_OK;
}
