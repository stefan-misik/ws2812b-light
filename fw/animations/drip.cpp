/**
 * @file
 */

#include "animations/drip.h"

Animation::Result DripAnimation::handleEvent(Event event, intptr_t parameter)
{
    switch (event)
    {
    case Event::INIT:
        position_ = 0;
        sub_ = 0;
        fillLedStrip(reinterpret_cast<AbstractLedStrip *>(parameter));
        break;

    case Event::STEP:
        if (10 == sub_)
        {
            ++position_;
            sub_ = 0;

            if (position_ ==
                    reinterpret_cast<AbstractLedStrip *>(parameter)->led_count)
            {
                position_ = 0;
            }
        }
        else
        {
            ++sub_;
        }
        fillLedStrip(reinterpret_cast<AbstractLedStrip *>(parameter));
    break;

    }
    return Result::NONE;
}

void DripAnimation::fillLedStrip(AbstractLedStrip * led_strip) const
{
    for (size_t position = 0; position < led_strip->led_count; ++position)
    {
        if (position == position_)
        {
            (*led_strip)[position] = {0xff, 0xff, 0xff};
        }
        else
        {
            (*led_strip)[position] = {0x00, 0x00, 0x00};
        }
    }
}
