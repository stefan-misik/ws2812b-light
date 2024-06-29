/**
 * @file
 */

#ifndef STATUS_LED_H_
#define STATUS_LED_H_

#include "tools/color.h"


class StatusLed
{
public:
    void setColor(ColorId color) { color_ = color; }

    ColorId color() const { return color_; }

private:
    ColorId color_ = ColorId::BLACK;
};




#endif  // STATUS_LED_H_
