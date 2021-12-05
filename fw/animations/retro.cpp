#include "animations/retro.h"

#include <stdlib.h>

static const LedState LED_RED =    {0xFF, 0x00, 0x00};
static const LedState LED_YELLOW = {0xA0, 0x7F, 0x00};
static const LedState LED_GREEN =  {0x00, 0xFF, 0x00};
static const LedState LED_BLUE =   {0x00, 0x00, 0xFF};


uint8_t RetroAnimation::handleEvent(Event type, Param param)
{
    switch (type)
    {
    case Event::START:
        delay_ = 0;
        return Result::IS_OK;

    case Event::UPDATE:
        if (0 != delay_)
        {
            --delay_;
            return Result::IGNORE_DEFAULT;
        }
        delay_ = ((rand() & 0x03) + 1) << 5;

        if (0 == variant_ || 1 == variant_)
            variant_ = 1 - variant_;

        render(param.ledStrip());
        return Result::IS_OK;

    case Event::STOP:
        return Result::IS_OK;

    case Event::BUTTON:
        if (param.paramLo() & ButtonState::PRESS)
        {
            switch (param.buttonId())
            {
            case ButtonId::UP:
                delay_ = 0;
                if (0 == variant_ || 1 == variant_)
                    variant_ = 2;
                break;
            case ButtonId::DOWN:
                delay_ = 0;
                if (2 == variant_)
                    variant_ = 0;
                break;
            }
        }
        return Result::IS_OK;
    }
    return Result::IS_OK;
}

void RetroAnimation::render(AbstractLedStrip * leds)
{
    uint8_t pos = 0;

    switch (variant_)
    {
    case 0:
        for (auto & led: *leds)
        {
            switch (pos & 0x01)
            {
            case 0: led = LED_RED; break;
            case 1: led = LED_YELLOW; break;
            }
            ++pos;
        }
        break;

    case 1:
        for (auto & led: *leds)
        {
            switch (pos & 0x01)
            {
            case 0: led = LED_GREEN; break;
            case 1: led = LED_BLUE; break;
            }
            ++pos;
        }
        break;

    case 2:
        for (auto & led: *leds)
        {
            switch (pos & 0x03)
            {
            case 0: led = LED_RED; break;
            case 1: led = LED_GREEN; break;
            case 2: led = LED_YELLOW; break;
            case 3: led = LED_BLUE; break;
            }
            ++pos;
        }
        break;
    }
}
