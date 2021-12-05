/**
 * @file
 */

#ifndef ANIMATIONS_RAINBOW_H_
#define ANIMATIONS_RAINBOW_H_

#include "animation.h"

class RainbowAnimation: public Animation
{
public:
    RainbowAnimation():
        space_increment_(5),
        time_increment_(2),
        step_(0),
        hue_(0)
    { }

    uint8_t handleEvent(Event type, Param param) override;

private:
    uint8_t space_increment_: 4;
    uint8_t time_increment_: 4;

    uint8_t step_;
    uint16_t hue_;
};

#endif  // ANIMATIONS_RAINBOW_H_
