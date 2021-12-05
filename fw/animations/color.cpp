#include "animations/color.h"

bool ColorAnimation::start(AbstractLedStrip * leds)
{
    redraw_ = true;
    return false;
}

bool ColorAnimation::update(AbstractLedStrip * leds)
{
    if (redraw_)
    {
        fillLedStrip(leds);
        redraw_ = false;
        return true;
    }
    else
        return false;
}

void ColorAnimation::stop(AbstractLedStrip * leds)
{
}

bool ColorAnimation::handleButton(ButtonId button, uint8_t state)
{
    if (state & ButtonState::PRESSED)
    {
        if (ButtonId::UP == button || ButtonId::DOWN == button)
        {
            stepRainbowColor();
            redraw_ = true;
        }
    }
    return true;
}

void ColorAnimation::stepRainbowColor()
{
    if (cr_ > 0)
    {
        if (cb_ > 0)
        {
            ++cr_;
            --cb_;
        }
        else
        {
            --cr_;
            ++cg_;
        }
    }
    else if (cg_ > 0)
    {
        --cg_;
        ++cb_;
    }
    else
    {
        ++cr_;
        --cb_;
    }
}

void ColorAnimation::fillLedStrip(AbstractLedStrip * led_strip) const
{
    uint8_t led_in_group = 0;
    for (auto & led: *led_strip)
    {
        led.red = cr_;
        led.green = cg_;
        led.blue = cb_;
    }
}
