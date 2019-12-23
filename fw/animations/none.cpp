/**
 * @file
 */

#include "animations/none.h"


void NoneAnimation::reset(AbstractLedStrip * led_strip)
{
    for (auto & led: *led_strip)
    {
        led = {0x00, 0x00, 0x00};
    }
}

uint8_t NoneAnimation::step(AbstractLedStrip * led_strip)
{
    return 0;
}
