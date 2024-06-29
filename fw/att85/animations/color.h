/**
 * @file
 */

#ifndef ANIMATIONS_COLOR_H_
#define ANIMATIONS_COLOR_H_

#include "animation.h"

class ColorAnimation: public Animation
{
public:
    static const uint8_t FIRST_COLOR = 1;

    uint8_t handleEvent(Event type, Param param, SharedStorage * storage) override;

private:
    uint8_t color_ = FIRST_COLOR;

    struct Shared
    {
        bool redraw = false;
    };
};

#endif  // ANIMATIONS_COLOR_H_
