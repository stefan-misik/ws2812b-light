/**
 * @file
 */

#ifndef ANIMATIONS_COLOR_H_
#define ANIMATIONS_COLOR_H_

#include "animation.h"

class ColorAnimation: public Animation
{
public:
    ColorAnimation():
        color_(0),
        redraw_(false)
    {
    }

    uint8_t handleEvent(Event type, Param param) override;

private:
    uint8_t color_;
    bool redraw_;
};

#endif  // ANIMATIONS_COLOR_H_
