/**
 * @file
 */

#include "animations/rainbow.h"

void RainbowAnimation::reset(AbstractLedStrip * led_strip)
{
    ca_ = 255;
    cb_ = 0;
    cc_ = 0;

    fillLedStrip(led_strip);
}

uint8_t RainbowAnimation::step(AbstractLedStrip * led_strip)
{
    stepRainbowColor();
    fillLedStrip(led_strip);
    return 5;
}

void RainbowAnimation::stepRainbowColor()
{
    if (ca_ > 0)
    {
        if (cc_ > 0)
        {
            ++ca_;
            --cc_;
        }
        else
        {
            --ca_;
            ++cb_;
        }
    }
    else if (cb_ > 0)
    {
        --cb_;
        ++cc_;
    }
    else
    {
        ++ca_;
        --cc_;
    }
}

void RainbowAnimation::fillLedStrip(AbstractLedStrip * led_strip) const
{
    uint8_t led_in_group = 0;
    for (auto & led: *led_strip)
    {
        switch (led_in_group)
        {
        case 0:
            led = {ca_, cb_, cc_};
            led_in_group = 1;
            break;
        case 1:
            led = {cc_, ca_, cb_};
            led_in_group = 2;
            break;
        case 2:
            led = {cb_, cc_, ca_};
            led_in_group = 0;
            break;
        }
    }
}
