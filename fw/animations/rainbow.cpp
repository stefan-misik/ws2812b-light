#include "animations/rainbow.h"

bool RainbowAnimation::start(AbstractLedStrip * leds)
{
    ca_ = 255;
    cb_ = 0;
    cc_ = 0;
    fillLedStrip(leds);
    return false;
}

bool RainbowAnimation::update(AbstractLedStrip * leds)
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

    fillLedStrip(leds);
    return true;
}

void RainbowAnimation::stop(AbstractLedStrip * leds)
{
}

bool RainbowAnimation::handleButton(ButtonId button, uint8_t state)
{
    return true;
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
