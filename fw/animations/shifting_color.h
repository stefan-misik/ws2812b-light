/**
 * @file
 */

#ifndef ANIMATIONS_SHIFTING_COLOR_H_
#define ANIMATIONS_SHIFTING_COLOR_H_

#include "animation.h"

class ShiftingColorAnimation: public Animation
{
public:
    struct Segment
    {
        LedState color;
        LedSize length;
    };

    ShiftingColorAnimation(const Segment * segments):
        segments_(segments)
    { }

    uint8_t handleEvent(Event type, Param param, SharedStorage * storage) override;

private:
    const Segment * segments_;

    struct Shared
    {
        uint8_t step = 0;
        LedSize offset = 0;
    };

    void render(AbstractLedStrip * leds, LedSize offset);
};

#endif  // ANIMATIONS_SHIFTING_COLOR_H_
