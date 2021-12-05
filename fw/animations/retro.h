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
        variant_(0)
    { }

    uint8_t handleEvent(Event type, Param param) override;

private:
    uint8_t delay_;
    uint8_t variant_;

    void render(AbstractLedStrip * leds);
};

#endif  // ANIMATIONS_RETRO_H_
