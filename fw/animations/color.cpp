#include "animations/color.h"
#include "buttons.h"

Animation::Result ColorAnimation::handleEvent(Event event, intptr_t parameter)
{
    switch (event)
    {
    case Event::CREATE:
        cr_ = 255;
        cg_ = 0;
        cb_ = 0;
        redraw_ = false;
        break;

    case Event::INIT:
        redraw_ = true;
        break;

    case Event::STEP:
        if (redraw_)
        {
            fillLedStrip(reinterpret_cast<AbstractLedStrip *>(parameter));
            redraw_ = false;
            return Result::NONE;
        }
        else
        {
            return Result::IGNORE_DEFAULT;
        }
    break;

    case Event::KEY_PRESS:
        if (Buttons::ButtonId::UP == parameter ||
                Buttons::ButtonId::DOWN == parameter)
        {
            stepRainbowColor();
            redraw_ = true;
        }
        break;
    }
    return Result::NONE;
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
        led = {cg_, cr_, cb_};
    }
}
