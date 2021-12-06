/**
 * @file
 */

#ifndef ANIMATIONS_RETRO_H_
#define ANIMATIONS_RETRO_H_

#include "animation.h"

class RetroAnimation: public Animation
{
public:
    RetroAnimation():
        delay_(0),
        variant_(0),
        state_(0)
    { }

    uint8_t handleEvent(Event type, Param param) override;

private:
    static const uint8_t VARIANT_CNT = 4;

    uint16_t delay_;
    uint8_t variant_: 4;
    uint8_t state_: 4;

    uint8_t render(AbstractLedStrip * leds);

    void reset()
    {
        delay_ = 0;
        state_ = 0;
    }
};

#endif  // ANIMATIONS_RETRO_H_
