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

    bool start(AbstractLedStrip * leds) override;
    bool update(AbstractLedStrip * leds) override;
    void stop(AbstractLedStrip * leds) override;
    bool handleButton(ButtonId button, uint8_t state) override;

private:
    uint8_t color_;
    bool redraw_;
};

#endif  // ANIMATIONS_COLOR_H_
