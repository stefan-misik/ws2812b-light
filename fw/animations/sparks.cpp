#include "animations/sparks.h"

#include <stdlib.h>

#include <avr/pgmspace.h>


bool SparksAnimation::start(AbstractLedStrip * leds)
{
    step_ = 0;
    return false;
}

bool SparksAnimation::update(AbstractLedStrip * leds)
{
    ++step_;
    if (STEPS != step_)
        return false;
    step_ = 0;

    for (auto & led: *leds)
        if ((rand() & 0x7ff) <= frequency_)
            led = {0xFF, 0xFF, 0xFF};
        else
            led = {0x47, 0x30, 0x0D};

    return true;
}

void SparksAnimation::stop(AbstractLedStrip * leds)
{
}

bool SparksAnimation::handleButton(ButtonId button, uint8_t state)
{
    if (ButtonState::PRESS & state)
    {
        switch (button)
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
    return true;
}

