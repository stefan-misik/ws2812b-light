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
        space_increment_(8),
        time_increment_(4)
    { }

    uint8_t handleEvent(Event type, Param param, SharedStorage * storage) override;

private:
    uint8_t space_increment_: 4;
    uint8_t time_increment_: 4;

    struct Shared
    {
        uint8_t step = 0;
        uint16_t hue = 0;
    };
};

#endif  // ANIMATIONS_RAINBOW_H_
