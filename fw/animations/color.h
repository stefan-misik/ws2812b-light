/**
 * @file
 */

#ifndef ANIMATIONS_COLOR_H_
#define ANIMATIONS_COLOR_H_

#include "animation.h"

class ColorAnimation: public Animation
{
public:
    uint8_t handleEvent(Event type, Param param, SharedStorage * storage) override;

private:
    uint8_t color_ = 0;

    struct Shared
    {
        bool redraw = false;
    };
};

#endif  // ANIMATIONS_COLOR_H_
