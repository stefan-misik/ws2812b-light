/**
 * @file
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_


#include "tools/button_filter.h"

#include <stdint.h>

class Buttons
{
public:
    enum ButtonId: uint8_t
    {
        UP = 0,
        RIGHT,
        LEFT,
        DOWN,

        BUTTON_COUNT
    };

    static ButtonFilter buttons[Buttons::BUTTON_COUNT];

    static void initialize();

    static void update();
};

#endif  // BUTTONS_H_
