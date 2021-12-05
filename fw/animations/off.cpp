#include "animations/off.h"
#include "buttons.h"

bool OffAnimation::start(AbstractLedStrip * leds)
{
    for (auto & led: *leds)
    {
        led.red = 0;
        led.green = 0;
        led.blue = 0;
    }
    return true;
}

bool OffAnimation::update(AbstractLedStrip * leds)
{
    return false;
}

void OffAnimation::stop(AbstractLedStrip * leds)
{
}

bool OffAnimation::handleButton(ButtonId button, uint8_t state)
{
    return true;
}
