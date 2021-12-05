#include "animations/rainbow.h"

#include "tools/color.h"


bool RainbowAnimation::start(AbstractLedStrip * leds)
{
    step_ = 0;
    return false;
}

bool RainbowAnimation::update(AbstractLedStrip * leds)
{
    uint16_t hue = hue_;
    LedState color;
    for (auto & led: *leds)
    {
        toSaturatedHue(hue, &color);
        hue = incrementHue(hue, space_increment_);
        led = color;
    }

    hue_ = incrementHue(hue_, time_increment_);

    return true;
}

void RainbowAnimation::stop(AbstractLedStrip * leds)
{
}

bool RainbowAnimation::handleButton(ButtonId button, uint8_t state)
{
    if (ButtonState::PRESS & state)
    {
        switch (button)
        {
        case ButtonId::UP:
            ++space_increment_;
            break;
        case ButtonId::DOWN:
            ++time_increment_;
            break;
        }
    }
    return true;
}
