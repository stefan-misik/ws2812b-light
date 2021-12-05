/**
 * @file
 */

#ifndef ANIMATIONS_OFF_H_
#define ANIMATIONS_OFF_H_

#include "animation.h"

class OffAnimation: public Animation
{
public:
    bool start(AbstractLedStrip * leds) override;
    bool update(AbstractLedStrip * leds) override;
    void stop(AbstractLedStrip * leds) override;
    bool handleButton(ButtonId button, uint8_t state) override;
};

#endif  // ANIMATIONS_OFF_H_
