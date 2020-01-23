/**
 * @file
 */

#include "animations/cold_light.h"
#include "buttons.h"


Animation::Result ColdLightAnimation::handleEvent(
        Event event,
        intptr_t parameter)
{
    switch (event)
    {
    case Event::INIT:
        fillLedStrip(reinterpret_cast<AbstractLedStrip *>(parameter));
        redraw_ = false;
        break;

    case Event::STEP:
        if (redraw_)
        {
            fillLedStrip(reinterpret_cast<AbstractLedStrip *>(parameter));
            redraw_ = false;
        }
        break;

    case Event::KEY_PRESS:
        switch (parameter)
        {
        case Buttons::UP:
            if (intensity_ < 252)
            {
                intensity_ += 4;
                redraw_ = true;
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
        break;

    }

    return Result::NONE;
}

void ColdLightAnimation::fillLedStrip(AbstractLedStrip * led_strip) const
{
    for (auto & led : *led_strip)
    {
        led = {intensity_, intensity_, intensity_};
    }
}
