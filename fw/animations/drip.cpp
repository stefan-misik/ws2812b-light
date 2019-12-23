/**
 * @file
 */

#include "animations/drip.h"

void DripAnimation::reset(AbstractLedStrip * led_strip)
{
    position_ = 0;
    sub_ = 0;
    fillLedStrip(led_strip);
}

uint8_t DripAnimation::step(AbstractLedStrip * led_strip)
{
    if (10 == sub_)
    {
        ++position_;
        sub_ = 0;

        if (position_ == led_strip->led_count)
        {
            position_ = 0;
        }
    }
    else
    {
        ++sub_;
    }
    fillLedStrip(led_strip);
    return 5;
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
