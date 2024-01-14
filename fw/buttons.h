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
        NONE = 0,
        UP,
        RIGHT,
        LEFT,
        DOWN,
        O_BTN,
        X_BTN,

        BUTTON_COUNT
    };

    ButtonId button() const { return current_button_; }

    void initialize();
    uint8_t run();

private:
    ButtonId current_button_ = ButtonId::NONE;
    ButtonFilter filter_;
};

#endif  // BUTTONS_H_
